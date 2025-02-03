#include "modules/menus.h"
#include "modules/menus.c"
#include "modules/utils.c"

#include "modules/utils.h"
#include "packages/cels/strings.h"
#include "packages/cels/strings.c"

#include "packages/cels/utils.c"
#include "packages/cels/vectors.c"
#include "packages/cels/mems.c"
#include "packages/cels/errors.c"
#include "packages/cels/nodes.c"
#include "packages/cels/ios.c"
#include "packages/cels/jsons.c"
#include "packages/cels/files.c"
#include "packages/cels/csvs.c"
#include "packages/cels/maths.c"

int main(int arg, char **argc) {
	utils_get_includes(strings_do("cels.c"), null);
	exit(1);

	if (arg < 2) {
		menus_print_help();
		return 0;
	} 

	const allocator mem = arenas_init(2048);

	size_t init_hash = strings_prehash("init");
	size_t build_hash = strings_prehash("build");
	size_t help_hash = strings_prehash("help");
	size_t debug_hash = strings_prehash("debug");
	size_t install_hash = strings_prehash("install");
	size_t generate_hash = strings_prehash("generate");

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
	} else if (command_hash == install_hash) {
		menus_handle_install();
	} else if (command_hash == generate_hash) {
		menus_handle_generate();
	} else {
		printf("invalid option - use 'cels help' for help.\n");
	}

	mems_free(&mem, null);

	return 0;
}
