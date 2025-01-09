#include "menus.h"

void menus_print_help(void) {
	printf(
		"cels is a tool to make c more convenient.\n\n"
		"commands:\n"
		"\tinit - starts a cel project\n"
		"\tbuild - builds the project\n");
}

void menus_handle_init(void) {
	string json_configuration = strings_premake("{\n"
		"\t\"name\": \"cels\",\n"
		"\t\"author\": \"angelus\",\n"
		"\t\"compiler\": \"gcc\",\n"
		"\t\"build\": \"gcc -Wall -Wextra -Wpedantic cels.c -o cels.o\",\n"
		"\t\"dev\": \"gcc -Wall -Wextra -Wpedantic -g cels.c -o cels.o -Dcels_debug=true\"\n"
	"}");
		
	file *json_check = fopen("./.cels-package.json", "r");
	if (json_check != null) {
		printf("a package is already initialized.\n");
		fclose(json_check);
		return;
	}

	file *json = fopen("./.cels-package.json", "w+");
	error write_error = files_write(json, json_configuration);
	if (write_error) {
		printf("an error happened while writing to file.\n");
		goto close;
	}

	printf("your package was successfully initialized.\n");

	close:
	fclose(json);
}
