#include "menus.h"

const string_vec options = vectors_premake(
	string, 
	strings_premake("yes"),
	strings_premake("no"));

void menus_print_help(void) {
	printf(
		"cels is a tool to make c more convenient.\n\n"
		"commands:\n"
		"\tinit - starts a cel project\n"
		"\tbuild - builds the project for production\n"
		"\tdebug - builds the project in debug mode\n");
}

void menus_handle_init(void) {
	const allocator mem = arenas_init(2048);

	file *json_check = fopen(".cels-package.json", "r");
	if (json_check) {
		printf("a package is already initialized.\n");
		fclose(json_check);
		return;
	}

	char *home = getenv("HOME");
	const string dictionary_path = strings_format(
		"%s/.config/cels/cels-dictionary.txt", &mem, home);

	file *install_exists = fopen(dictionary_path.data, "r");
	if (!install_exists) {
		printf("cels not installed.\n\n");

		size_t option = ios_select("install?", options);
		if (option > 0) { goto cleanup; }

		menus_handle_install();
	}

	if (install_exists) {
		fclose(install_exists);
	}

	string flags = {0};
	estring filepath = utils_get_main_file(&mem);
	if (filepath.error == file_successfull) {
		estring eflags = utils_get_flags(filepath.value, &mem);
		if (eflags.error == 0) {
			flags = eflags.value;
		}
	}

	configuration configuration = utils_ask_configuration(&mem);
	configuration.main = filepath.value;
	configuration.flags = flags;

	string configuration_json = utils_create_configuration(
		&configuration, &mem);

	if (filepath.error != file_successfull) {
		file *file = fopen(filepath.value.data, "a");

		if (file) {
			fclose(file);
		}
	}

	file *json = fopen(".cels-package.json", "w+");
	error write_error = files_write(json, configuration_json);
	if (write_error) {
		printf("an error happened while writing to file.\n");
		goto close;
	}

	error error = files_make_directory("packages", 0700);
	if (error == file_directory_not_created_error) {
		printf("directory couldn't be created.\n");
		goto close;
	}

	printf("your package was successfully initialized.\n");

	close:
	fclose(json);

	cleanup:
	mem.free(mem.storage);
}

void menus_handle_build(bool is_build_mode) {
	const allocator mem = arenas_init(2048);

	file *json_file = fopen(".cels-package.json", "r");
	if (!json_file) {
		printf("a package must be initialized.\n\n");

		size_t option = ios_select("initialize?", options);
		if (option > 0) { goto cleanup; }

		menus_handle_init();
 	}

	estring json = files_read(json_file, &mem);
	fclose(json_file);

	if (json.error != file_successfull) {
		printf("an error occurred.\n\n");
		goto cleanup;
	}

	//strings_trim(&json.value);
	estring_map json_map = jsons_unmake(&json.value, &mem);

	if (json_map.error != json_successfull) {
		printf("an error parsing occurred.\n");
		printf("%d\n", json_map.error);
		goto cleanup;
	}

	const string build_key = strings_premake("build");
	const string prod_key = strings_premake("prod");

	string *value = string_maps_get(
			&json_map.value, 
			is_build_mode ? build_key : prod_key);

	if (!value) {
		printf("cels-package is mal-formed.\n");
		goto cleanup;
	}

	int system_status = system(value->data);
	if (system_status != 0) {
		printf("an error while compiling ocurred.\n");
		goto cleanup;
	}

	printf("Project successfully compiled.\n");

	cleanup:
	mem.free(mem.storage);
}

void menus_handle_install(void) {
	const allocator mem = arenas_init(1024);

	file *bynary_file = fopen("cels.o", "r");
		if (!bynary_file) {
		printf("No compiled bynary was found.\n\n");

		size_t option = ios_select("compile?", options);
		if (option > 0) { goto cleanup; }

		menus_handle_build(true);
	}

	if (bynary_file) {
		fclose(bynary_file);
	}

	const char *home = getenv("HOME");
	const string homepath = strings_format("%s/.config/cels", &mem, home);

	error make_error = files_make_directory(homepath.data, 0700);
	if (make_error == file_directory_not_created_error) {
		printf("Project failed to be created\n");
		goto cleanup;
	}

	const string dict = strings_format("%s/.config/cels/cels-dictionary.txt", &mem, home);
	error rename_error = rename("cels-dictionary.txt", dict.data);
	if (rename_error) {
		printf("Dictionary failed to install\n");
		goto cleanup;
	}

	const string comp = strings_format("%s/.local/bin/cels", &mem, home);
	rename_error = rename("cels.o", comp.data);
	if (rename_error) {
		printf("Executable failed to install\n");
		goto cleanup;
	}

	printf("Project installed successfully\n");

	cleanup:
	mem.free(mem.storage);
}
