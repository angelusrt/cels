#include "menus.h"

void menus_print_help(void) {
	printf(
		"cels is a tool to make c more convenient.\n\n"
		"commands:\n"
		"\tinit - starts a cel project\n"
		"\tbuild - builds the project for production\n"
		"\tdebug - builds the project in debug mode\n");
}

void menus_handle_init(void) {
	file *json_check = fopen("./.cels-package.json", "r");
	if (json_check != null) {
		printf("a package is already initialized.\n");
		fclose(json_check);
		return;
	}

	string flags = {0};
	estring filepath = utils_get_main_file(null);
	if (filepath.error == file_successfull) {
		estring eflags = utils_get_flags(filepath.value, null);
		if (eflags.error == 0) {
			flags = eflags.value;
		}
	}

	configuration configuration = utils_ask_configuration();
	configuration.main = filepath.value;
	configuration.flags = flags;

	string configuration_json = utils_create_configuration(
		&configuration, null);

	if (filepath.error != file_successfull) {
		file *file = fopen(filepath.value.data, "a");

		if (file) {
			fclose(file);
		}
	}

	file *json = fopen("./.cels-package.json", "w+");
	error write_error = files_write(json, configuration_json);
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

	strings_trim(&json.value);
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
