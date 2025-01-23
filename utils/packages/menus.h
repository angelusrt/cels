#ifndef cels_menu_h
#define cels_menu_h

#include "../../source/strings.h"
#include "../../source/files.h"
#include "../../source/ios.h"
#include "../../source/jsons.h"
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

#endif
