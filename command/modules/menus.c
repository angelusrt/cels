#include "menus.h"

static const string_vec options = vectors_premake(
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

error menus_handle_init(void) {
	const allocator mem = arenas_init(2048);

	file *json_check = fopen(".cels-package.json", "r");
	if (json_check) {
		printf("a package is already initialized.\n");
		fclose(json_check);
		goto cleanup0;
	}

	char *home = getenv("HOME");
	const string dictionary_path = strings_format(
		"%s/.config/cels/cels-dictionary.txt", &mem, home);

	file *install_exists = fopen(dictionary_path.data, "r");
	if (!install_exists) {
		printf("cels not installed.\n\n");

		size_t option = ios_select("install?", options);
		if (option > 0) { 
			goto cleanup0; 
		}

		error err = menus_handle_install();
		if (err != ok) {
			printf("init failed");
			goto cleanup0;
		}
	} else {
		fclose(install_exists);
	}

	string flags = {0};
	estring filepath = utils_get_main_file(&mem);
	if (filepath.error == ok) {
		estring eflags = utils_get_flags(filepath.value, &mem);
		if (eflags.error == ok) {
			flags = eflags.value;
		}
	}

	configuration configuration = utils_ask_configuration(&mem);
	configuration.main = filepath.value;
	configuration.flags = flags;

	string configuration_json = utils_create_configuration(
		&configuration, &mem);

	file *json = fopen(".cels-package.json", "w+");
	if (!json) {
		printf("file '.cels-package.json' wasn't created.\n");
		goto cleanup0;
	}

	error write_error = files_write(json, configuration_json);
	if (write_error) {
		printf("an error happened while writing to file.\n");
		goto cleanup1;
	}

	error make_error = dirs_make("packages", 0700);
	if (make_error == file_directory_not_created_error) {
		printf("directory 'packages' couldn't be created.\n");
		goto cleanup1;
	}

	make_error = dirs_make("modules", 0700);
	if (make_error == file_directory_not_created_error) {
		printf("directory 'modules' couldn't be created.\n");
		goto cleanup1;
	}

	printf("your package was successfully initialized.\n");


	fclose(json);
	mems_free(&mem, null);
	return ok;

	cleanup1:
	fclose(json);

	cleanup0:
	mems_free(&mem, null);
	return fail;
}

error menus_handle_build(bool is_build_mode) {
	const allocator mem = arenas_init(2048);

	file *json_file = fopen(".cels-package.json", "r");
	if (!json_file) {
		printf("a package must be initialized.\n\n");

		size_t option = ios_select("initialize?", options);
		if (option > 0) { 
			goto cleanup0; 
		}

		error err = menus_handle_init();
		if (err != ok) {
			printf("build failed");
			goto cleanup0;
		}
 	}

	estring json = files_read(json_file, &mem);
	fclose(json_file);

	if (json.error != file_successfull) {
		printf("an error occurred.\n\n");
		goto cleanup0;
	}

	//strings_trim(&json.value);
	estring_map json_map = jsons_unmake(&json.value, &mem);

	if (json_map.error != json_successfull) {
		printf("an error parsing occurred.\n");
		printf("%d\n", json_map.error);
		goto cleanup0;
	}

	const string build_key = strings_premake("build");
	const string prod_key = strings_premake("prod");

	string *value = string_maps_get(
		&json_map.value, is_build_mode ? build_key : prod_key);

	if (!value) {
		printf("cels-package is mal-formed.\n");
		goto cleanup0;
	}

	int system_status = system(value->data);
	if (system_status != 0) {
		printf("an error while compiling ocurred.\n");
		goto cleanup0;
	}

	printf("Project successfully compiled.\n");

	mems_free(&mem, null);
	return ok;

	cleanup0:
	mems_free(&mem, null);
	return fail;
}

error menus_handle_install(void) {
	const allocator mem = arenas_init(1024);

	file *bynary_file = fopen("cels.o", "r");
		if (!bynary_file) {
		printf("No compiled bynary was found.\n\n");

		size_t option = ios_select("compile?", options);
		if (option > 0) { 
			goto cleanup0; 
		}

		error err = menus_handle_build(true);
		if (err != ok) {
			printf("install failed");
			goto cleanup0;
		}
	}

	if (bynary_file) {
		fclose(bynary_file);
	}

	const char *home = getenv("HOME");
	const string homepath = strings_format("%s/.config/cels", &mem, home);

	error make_error = dirs_make(homepath.data, 0700);
	if (make_error == file_directory_not_created_error) {
		printf("Project failed to be created\n");
		goto cleanup0;
	}

	const string dict = strings_format("%s/.config/cels/cels-dictionary.txt", &mem, home);
	error rename_error = rename("cels-dictionary.txt", dict.data);
	if (rename_error) {
		printf("Dictionary failed to install\n");
		goto cleanup0;
	}

	const string comp = strings_format("%s/.local/bin/cels", &mem, home);
	rename_error = rename("cels.o", comp.data);
	if (rename_error) {
		printf("Executable failed to install\n");
		goto cleanup0;
	}

	printf("Project installed successfully\n");

	mems_free(&mem, null);
	return ok;

	cleanup0:
	mems_free(&mem, null);
	return fail;
}

error menus_handle_generate(void) {
	const allocator mem = arenas_init(2048);

	file *json_file = fopen(".cels-package.json", "r");
	if (!json_file) {
		printf("a package must be initialized.\n\n");

		size_t option = ios_select("initialize?", options);
		if (option > 0) { 
			goto cleanup0; 
		}

		error err = menus_handle_init();
		if (err != ok) {
			printf("generation failed");
			goto cleanup0;
		}
 	}

	estring json = files_read(json_file, &mem);
	fclose(json_file);

	if (json.error != file_successfull) {
		printf("an error occurred.\n\n");
		goto cleanup0;
	}

	//strings_trim(&json.value);
	estring_map json_map = jsons_unmake(&json.value, &mem);

	if (json_map.error != json_successfull) {
		printf("an error parsing occurred.\n");
		printf("%d\n", json_map.error);
		goto cleanup0;
	}

	mems_free(&mem, null);
	return ok;

	cleanup0:
	mems_free(&mem, null);
	return fail;
}
