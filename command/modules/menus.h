#ifndef menu_h
#define menu_h

#include "../packages/cels/strings.h"
#include "../packages/cels/files.h"
#include "../packages/cels/ios.h"
#include "../packages/cels/jsons.h"
#include "utils.h"

/*
 * Prints the typical help 
 * message that any terminal-programm 
 * prints comprising of the most 
 * important commands.
 *
 * #to-review
 */
void menus_print_help(void);

/*
 * Initializes project.
 *
 * #to-review
 */
void menus_handle_init(void);

/*
 * Builds project
 *
 * #to-review
 */
void menus_handle_build(bool is_build_mode);

/*
 * Installs project
 *
 * #to-review
 */
void menus_handle_install(void);

/*
 * Generates code.
 *
 * #to-review
 */
void menus_handle_generate();

#endif
