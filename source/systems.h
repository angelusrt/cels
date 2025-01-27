#ifndef cels_systems_h
#define cels_systems_h

#include "files.h"
#include "utils.h"
#include <stdlib.h>

/*
 * The module 'systems' deals with 
 * platform information, system 
 * environment and dealing with os.
 */

typedef enum system_error {
	system_successfull,
	system_generic_error,
	system_env_file_mal_formed_error
} system_error;

/*
 * Loads the environment with variable 
 * found on file with path 'path'.
 *
 * #allocated #to-review
 */
error systems_load(const string path, const allocator *mem);

#endif
