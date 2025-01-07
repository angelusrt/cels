#include "../source/strings.h"
#include "../source/strings.c"

#include "../source/utils.c"
#include "../source/vectors.c"
#include "../source/mems.c"
#include "../source/errors.c"
#include "../source/nodes.c"

void menus_print_help(void) {
	printf(
		"cels is a tool to make c more convenient.\n\n"
		"commands:\n"
		"\tinit - starts a cel project\n"
		"\tbuild - builds the project\n");
}

int main(int arg, char **argc) {
	if (arg != 2) {
		menus_print_help();
		return 0;
	} 

	const allocator mem = arenas_init(4096);

	size_t init_hash = strings_prehash("init");
	size_t build_hash = strings_prehash("build");
	size_t help_hash = strings_prehash("help");

	string command = strings_make(argc[1], &mem);
	size_t command_hash = strings_hasherize(&command);

	if (command_hash == init_hash) {
		printf("init - work in progress\n");
	} else if (command_hash == build_hash) {
		printf("build - work in progress\n");
	} else if (command_hash == help_hash) {
		menus_print_help();
	} else {
		printf("invalid option - use 'cels help' for help.");
	}

	return 0;
}
