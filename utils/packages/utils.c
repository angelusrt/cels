#include "utils.h"

/* solução temporária */
string_vec string_vecs_filter_unique2(string_vec *self, const allocator *mem) { 
	if (cels_debug) { 
		errors_abort("self", vectors_check((void *)self)); 
	} 
	
	string_vec other = string_vecs_init(vector_min, mem); 
	
	for (size_t i = 0; i < self->size; i++) { 
		if (cels_debug) { 
			errors_abort("self.data[i]", strings_check_extra(&self->data[i])); 
		} 
		
		bool match = false; 
		for (size_t j = 0; j < other.size; j++) { 
			if (strings_equals(&self->data[i], &other.data[j])) { 
				match = true; 
				break; 
			} 
		} 
		
		if (!match) { 
			string clone = strings_clone(&self->data[i], mem);
			string_vecs_push(&other, clone, mem); 
		} 
	} 

	return other;
}
	

/* private */
bool utils_filter_suffix(string *self) {
	#if cels_debug
		errors_abort("self", strings_check_extra(self));
	#endif

	static const string c_extension = strings_premake(".c");
	return strings_has_suffix(self, c_extension);
}

estring utils_get_main_file(const allocator *mem) {
	const string path = strings_premake("./");

	estring_vec files = files_list(path, mem);
	if (files.error != file_successfull) {
		return (estring){.error=-1};
	}

	error filter_error = string_vecs_filter(
		&files.value, (filterfunc)utils_filter_suffix, mem);

	if (filter_error) {
		return (estring){.error=-1};
	}

	for (size_t i = 0; i < files.value.size; i++) {
		file *file = fopen(files.value.data[i].data, "r");
		if (!file) { continue; }

		const string main_function = strings_premake("main");
		ssize_t position = files_find(file, main_function, 0);
		if (position < 0) {
			fclose(file);
			continue;
		}

		string the_file_path = strings_clone(&files.value.data[i], mem);
		string_vecs_free(&files.value, mem);
		return (estring){.value=the_file_path};
	}

	string_vecs_free(&files.value, mem);
	return (estring){.error=1};
}

/* private */
estring_vec utils_get_packages(const string path, const allocator *mem) {
	#if cels_debug
		errors_abort("path", strings_check_extra(&path));
	#endif

	estring path_normalized = files_path(&path, mem);
	if (path_normalized.error != file_successfull) {
		return (estring_vec){.error=-1};
	}

	file *file = fopen(path_normalized.value.data, "r");
	strings_free(&path_normalized.value, mem);

	if (!file) {
		return (estring_vec){.error=-1};
	}
	
	string_vec non_terminals = string_vecs_init(vector_min, mem);
	string_vec terminals = string_vecs_init(vector_min, mem);

	string line = {0};
	while (!files_next(file, &line, mem)) {
		if (strings_find_with(&line, "#include", 0) < 0) {
			continue;
		}

		string line_view = strings_cut(&line);
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
			string_vecs_push(&non_terminals, path_workaround, mem);

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
			string_vecs_push(&terminals, new_path, mem);
			continue;
		}
	}

	fclose(file);
	for (size_t i = 0; i < non_terminals.size; i++) {
		estring_vec file_terminals = utils_get_packages(non_terminals.data[i], mem);
		if (file_terminals.error != 0) {
			continue;
		}

		string_vecs_unite(&terminals, &file_terminals.value, mem);
	}
	
	string_vecs_free(&non_terminals, mem);
	//string_vecs_filter_unique(&terminals, mem);

	return (estring_vec){.value=terminals};
}

estring utils_get_flags(string main_file_name, const allocator *mem) {
	#if cels_debug
		errors_abort("main_file_name", strings_check_extra(&main_file_name));
	#endif

	estring_vec packages = utils_get_packages(main_file_name, mem);
	if (packages.error != 0) {
		return (estring){.error=1};
	}

	//error filter_error = string_vecs_filter_unique(&packages.value, mem);
	packages.value = string_vecs_filter_unique2(&packages.value, mem);

	char *home = getenv("HOME");
	const string homepath = strings_format(
		"%s/.config/cels/cels-dictionary.txt", mem, home);

	file *dictionary_file = fopen(homepath.data, "r");
	if (!dictionary_file) {
		return (estring){.error=-1};
	}

	string line = {0};
	string flag = strings_init(vector_min, mem);
	for (size_t i = 0; i < packages.value.size; i++) {
		while (!files_next(dictionary_file, &line, mem)) {
			if (strings_find(&line, packages.value.data[i], 0) < 0) {
				continue;
			}

			ssize_t pos = strings_find_with(&line, ", ", 0);
			if (pos < 0) {
				continue;
			}
			
			string flag_view = strings_view(&line, pos + 2, line.size - 1);
			strings_push_with(&flag, " ", mem);
			strings_push(&flag, flag_view, mem);
		}

		int seek_error = fseek(dictionary_file, 0, SEEK_SET);
		if (seek_error) {
			break;
		}
	}

	return (estring){.value=flag};
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

string utils_create_configuration(own configuration *configuration, const allocator *mem) {
	const string template = strings_premake("{\n"
		"\t\"name\": \"%s\",\n"
		"\t\"author\": \"%s\",\n"
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
		strings_premake("gcc -Wall -Wextra -Wpedantic -g -rdynamic %s.c -o %s.o%s -Dcels_debug=true"),
		strings_premake("gcc -Wall -Wextra -Wpedantic %s.c -o %s.o%s -Dcels_debug=false")
	);
	
	const string_vec clangs = vectors_premake(
		string, 
		strings_premake("nada"), 
		strings_premake("nada")
	);

	string name = {0};
	if (configuration->main.size > 0) {
		const string extension = strings_premake(".c");
		name = strings_replace_with(&configuration->main, extension, "", 0, mem);
	} else {
		name = strings_clone(&configuration->name, mem);
	}

	if (configuration->flags.size == 0) {
		configuration->flags = strings_make("", mem);
	}
	
	string build = {0};
	string dev = {0};
	if (configuration->compiler == 0) {
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
		compilers.data[configuration->compiler].data,
		build.data,
		dev.data);

	return configuration_json;
}

