#include "menus.h"

void menus_print_help(void) {
	printf(
		"cels is a tool to make c more convenient.\n\n"
		"commands:\n"
		"\tinit - starts a cel project\n"
		"\tbuild - builds the project for production\n"
		"\tdebug - builds the project in debug mode\n");
}

string menus_handle_ask_configuration(void) {
	const string template = strings_premake("{\n"
		"\t\"name\": \"%s\",\n"
		"\t\"author\": \"%s\",\n"
		"\t\"compiler\": \"%s\",\n"
		"\t\"build\": \"%s\",\n"
		"\t\"dev\": \"%s\"\n"
	"}");

	const string_vec compilers = vectors_premake(
		string, 
		strings_premake("gcc"), 
		strings_premake("clang")
	);

	const string_vec gccs = vectors_premake(
		string, 
		strings_premake("gcc -Wall -Wextra -Wpedantic %s.c -o %s.o -lm"), 
		strings_premake("gcc -Wall -Wextra -Wpedantic -g %s.c -o %s.o -lm -Dcels_debug=true")
	);
	
	const string_vec clangs = vectors_premake(
		string, 
		strings_premake("nada"), 
		strings_premake("nada")
	);

	//

	string build = {0};
	string dev = {0};
	string name = ios_ask("name: ", null);
	string author = ios_ask("author: ", null);
	size_t selected = ios_select("compiler: ", compilers);

	if (selected == 0) {
		build = strings_make_format(gccs.data[0].data, null, name.data, name.data);
		dev = strings_make_format(gccs.data[1].data, null, name.data, name.data);
	} else {
		build = clangs.data[0];
		dev = clangs.data[1];
	}

	printf("\n");

	string configuration = strings_make_format(
		template.data, 
		null, 
		name.data, 
		author.data, 
		compilers.data[selected].data,
		build.data,
		dev.data);

	return configuration;
}

void menus_handle_init(void) {
	string configuration = menus_handle_ask_configuration();
		
	file *json_check = fopen("./.cels-package.json", "r");
	if (json_check != null) {
		printf("a package is already initialized.\n");
		fclose(json_check);
		return;
	}

	file *json = fopen("./.cels-package.json", "w+");
	error write_error = files_write(json, configuration);
	if (write_error) {
		printf("an error happened while writing to file.\n");
		goto close;
	}

	printf("your package was successfully initialized.\n");

	close:
	fclose(json);
}

void menus_handle_build(bool is_build_mode) {
	file *json_file = fopen("./.cels-package.json", "r");
	if (json_file == null) {
		printf("a package must be initialized.\n");
		return;
	}

	estring json = files_read(json_file, null);
	fclose(json_file);

	if (json.error != file_successfull) {
		printf("an error occurred.\n");
		return;
	}

	estring_map json_map = jsons_unmake(&json.value, null);
	if (json_map.error != json_successfull) {
		printf("an error parsing occurred.\n");
		printf("%d\n", json_map.error);
		return;
	}

	const string build_key = strings_premake("build");
	const string dev_key = strings_premake("dev");

	string *value = string_maps_get(
			json_map.value, 
			is_build_mode ? build_key : dev_key);

	if (value == null) {
		printf("cels-package is mal-formed.\n");
		return;
	}

	int system_status = system(value->data);
	if (system_status != 0) {
		printf("an error while compiling ocurred.\n");
		return;
	}

	printf("Project successfully compiled.\n");
}
