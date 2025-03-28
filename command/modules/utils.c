#include "utils.h"


/* utils */

estring utils_get_main_file(const allocator *mem) {
	static const string cfile = strings_premake(".c");
	dir_iterator it = {0};
	while (dirs_next("./", &it, mem)) {
		if (!strings_has_suffix(&it.data, cfile)) {
			continue;
		}

		file *file = fopen(it.data.data, "r");
		if (!file) { continue; }

		const byte_vec main_function = vectors_premake(byte, 'm', 'a', 'i', 'n');
		ssize_t position = files_find(file, main_function, 0);
		if (position < 0) {
			fclose(file);
			continue;
		}

		string the_file_path = strings_clone(&it.data, mem);
		dir_iterators_free(&it, mem);

		return (estring){.value=the_file_path};
	}

	return (estring){.error=fail};
}


/* private */

estring_vec utils_get_packages(const string path, const allocator *mem) {
	#if cels_debug
		errors_abort("path", strings_check_extra(&path));
	#endif

	estring path_normalized = paths_make(&path, mem);
	if (path_normalized.error != file_successfull) {
		return (estring_vec){.error=-1};
	}

	file *file = fopen(path_normalized.value.data, "r");
	strings_free(&path_normalized.value, mem);

	if (!file) {
		return (estring_vec){.error=-1};
	}
	
	string_vec non_terminals = {0};
	vectors_init(&non_terminals, sizeof(string), vector_min, mem);

	string_vec terminals = {0};
	vectors_init(&terminals, sizeof(string), vector_min, mem);

	byte_vec line = {0};
	while (!files_next(file, &line, mem)) {
		string *l = (string *)&line;

		if (strings_find_with(l, "#include", 0) < 0) {
			continue;
		}

		string line_view = strings_cut(l);
		if (line_view.size == 0 || line_view.data[0] != '#') {
			continue;
		}

		const string extension = strings_premake(".c\"");
		if (strings_has_suffix(&line_view, extension)) {
			continue;
		}

		ssize_t position = strings_find_with(&line_view, "\"", 0);
		if (position > 0) {
			ssize_t pos = strings_find_with(&line_view, ".h\"", position);
			if (pos < 0) {
				continue;
			}

			string new_path = strings_clone(&line_view, mem);
			strings_slice(&new_path, position + 1, line_view.size - 2);
			//TODO: strings_slice_from()

			string path_workaround = strings_init(vector_min, mem);
			strings_push_with(&path_workaround, "./", mem);
			strings_push(&path_workaround, path, mem);
			strings_push_with(&path_workaround, "/../", mem);
			strings_push(&path_workaround, new_path, mem);
			strings_free(&new_path, mem);
			vectors_push(&non_terminals, &path_workaround, mem);

			continue;
		}

		position = strings_find_with(&line_view, "<", 0);

		if (position > 0) {
			ssize_t pos = strings_find_with(&line_view, ".h>", position);
			if (pos < 0) {
				continue;
			}

			string new_path = strings_clone(&line_view, mem);
			strings_slice(&new_path, position + 1, line_view.size - 2);
			vectors_push(&terminals, &new_path, mem);
			continue;
		}
	}

	fclose(file);
	for (size_t i = 0; i < non_terminals.size; i++) {
		estring_vec file_terminals = utils_get_packages(
			non_terminals.data[i], mem);

		if (file_terminals.error != 0) {
			continue;
		}

		vectors_unite(&terminals, &file_terminals.value, mem);
	}
	
	vectors_free(&non_terminals, (freefunc)strings_free, mem);

	return (estring_vec){.value=terminals};
}

estring utils_get_flags(string main_file_name, const allocator *mem) {
	#if cels_debug
		errors_abort("main_file_name", strings_check_extra(&main_file_name));
	#endif

	estring_vec packages = utils_get_packages(main_file_name, mem);
	if (packages.error != 0) { return (estring){.error=1}; }

	error filter_error = vectors_filter_unique(
		&packages.value, 
		(compfunc)strings_equals, 
		(freefunc)strings_free, 
		mem);

	if (filter_error) {
		vectors_free(&packages.value, (freefunc)strings_free, mem);
		return (estring){.error=fail};
	}

	char *home = getenv("HOME");
	const string homepath = strings_format(
		"%s/.config/cels/cels-dictionary.txt", mem, home);

	file *dictionary_file = fopen(homepath.data, "r");
	if (!dictionary_file) {
		return (estring){.error=-1};
	}

	byte_vec line = {0};
	string flag = strings_init(vector_min, mem);
	for (size_t i = 0; i < packages.value.size; i++) {
		while (!files_next(dictionary_file, &line, mem)) {
			string *l = (string *)&line;

			if (strings_find(l, packages.value.data[i], 0) < 0) {
				continue;
			}

			ssize_t pos = strings_find_with(l, ", ", 0);
			if (pos < 0) {
				continue;
			}
			
			string_view flag_view = strings_view(l, pos + 2, line.size - 1);
			strings_push_with(&flag, " ", mem);
			strings_push(&flag, flag_view, mem);
		}

		int seek_error = fseek(dictionary_file, 0, SEEK_SET);
		if (seek_error) {
			break;
		}
	}

	if (line.data) {
		byte_vecs_free(&line, mem);
	}

	return (estring){.value=flag};
}

/* private */
estring_vec utils_get_includes(const string path, const allocator *mem) {
	#if cels_debug
		errors_abort("path", strings_check_extra(&path));
	#endif

	file *file = fopen(path.data, "r");
	if (!file) {
		printf("file not found\n");
		return (estring_vec){.error=-1};
	}
	
	string_vec file_paths = {0};
	vectors_init(&file_paths, sizeof(string), vector_min, mem);

	byte_vec line = {0};
	while (!files_next(file, &line, mem)) {
		string *l = (string *)&line;

		strings_trim(l);
		
		if (line.size < 1) {
			continue;
		}

		if (line.data[0] != '#' && line.data[0] != '/' && line.data[0] != '*') {
			break;
		}

		if (strings_find_with(l, "#include", 0) < 0) {
			continue;
		}

		ssize_t position = strings_find_with(l, "\"", 0);
		if (position > 0) {
			ssize_t pos = strings_find_with(l, "\"", position + 1);
			if (pos < 0) { continue; }

			string new_path = strings_clone(l, mem);
			strings_slice(&new_path, position + 1, pos);
			vectors_push(&file_paths, &new_path, mem);

			continue;
		}
	}

	fclose(file);
	return (estring_vec){.value=file_paths};
}

void configurations_free(configuration *configuration, const allocator *mem) {
	strings_free(&configuration->name, mem);
	strings_free(&configuration->flags, mem);
	strings_free(&configuration->main, mem);
	strings_free(&configuration->author, mem);
}

configuration utils_ask_configuration(const allocator *mem) {
	const string_vec compilers = vectors_premake(
		string, 
		strings_premake("gcc"), 
		strings_premake("clang")
	);

	string name = ios_ask("name: ", mem);
	string author = ios_ask("author: ", mem);
	size_t selected = ios_select("compiler: ", compilers);

	return (configuration) {
		.name=name,
		.author=author,
		.compiler=selected,
	};
}

string utils_create_configuration(
	own configuration *configuration, const allocator *mem) {

	const string template = strings_premake("{\n"
		"\t\"name\": \"%s\",\n"
		"\t\"author\": \"%s\",\n"
		"\t\"entry\": \"%s\",\n"
		"\t\"compiler\": \"%s\",\n"
		"\t\"build\": \"%s\",\n"
		"\t\"prod\": \"%s\"\n"
	"}");

	const string_vec compilers = vectors_premake(
		string, 
		strings_premake("gcc"), 
		strings_premake("clang")
	);

	const string_vec gccs = vectors_premake(
		string, 
		strings_premake(
			"gcc -Wall -Wextra -Wpedantic -g -rdynamic "
			"%s.c -o %s.o%s -Dcels_debug=true"),
		strings_premake(
			"gcc -Wall -Wextra -Wpedantic "
			"%s.c -o %s.o%s -Dcels_debug=false")
	);
	
	const string_vec clangs = vectors_premake(
		string, 
		strings_premake("nada"), 
		strings_premake("nada")
	);

	const string extension = strings_premake(".c");
	string main = {0};
	string name = {0};

	if (configuration->main.size > 0) {
		name = strings_replace_with(
			&configuration->main, extension, "", 0, mem);

		main = strings_clone(&configuration->main, mem);
	} else {
		name = strings_clone(&configuration->name, mem);
		main = strings_clone(&configuration->name, mem);
		strings_push_with(&main, ".c", mem);
	}

	if (configuration->flags.size == 0) {
		configuration->flags = strings_init(8, mem);
	}
	
	string build = {0};
	string dev = {0};
	if (configuration->compiler == compiler_gcc) {
		build = strings_format(
			gccs.data[0].data, 
			mem, 
			name.data, 
			name.data, 
			configuration->flags.data);

		dev = strings_format(
			gccs.data[1].data, 
			mem, 
			name.data, 
			name.data, 
			configuration->flags.data);
	} else {
		build = strings_make(clangs.data[0].data, mem);
		dev = strings_make(clangs.data[1].data, mem);
	}

	string configuration_json = strings_format(
		template.data, 
		mem, 
		configuration->name.data, 
		configuration->author.data, 
		main.data, 
		compilers.data[configuration->compiler].data,
		build.data,
		dev.data);

	return configuration_json;
}
