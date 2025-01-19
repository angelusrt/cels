#include "./packages/menus.h"
#include "./packages/menus.c"
#include "./packages/utils.c"

#include "../source/strings.h"
#include "../source/strings.c"

#include "../source/utils.c"
#include "../source/vectors.c"
#include "../source/mems.c"
#include "../source/errors.c"
#include "../source/nodes.c"
#include "../source/ios.c"
#include "../source/jsons.c"
#include "../source/files.c"
#include "../source/csvs.c"

int main(int arg, char **argc) {
	if (arg < 2) {
		menus_print_help();
		return 0;
	} 

	const allocator mem = arenas_init(4096);

	size_t init_hash = strings_prehash("init");
	size_t build_hash = strings_prehash("build");
	size_t help_hash = strings_prehash("help");
	size_t debug_hash = strings_prehash("debug");

	string command = strings_make(argc[1], &mem);
	size_t command_hash = strings_hasherize(&command);

	if (command_hash == init_hash) {
		menus_handle_init();
	} else if (command_hash == build_hash) {
		menus_handle_build(true);
	} else if (command_hash == debug_hash) {
		menus_handle_build(false);
	} else if (command_hash == help_hash) {
		menus_print_help();
	} else {
		printf("invalid option - use 'cels help' for help.\n");
	}

	return 0;
}
