#ifndef cels_jsons_h 
#define cels_jsons_h

#include <stdbool.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <stdlib.h>

#include "strings.h"


/*
 * The 'jsons' module parses and 
 * manipulate json strings turning 
 * them into string_map data_structure.
 */


/* jsons */

typedef enum json_error {
	json_successfull,
	json_missing_colon_error,
	json_missing_comma_error,
	json_misplaced_colon_error,
	json_misplaced_comma_error,
	json_key_size_error,
	json_value_size_error,
	json_invalid_naked_value_error,
	json_invalid_error,
	json_invalid_state_error,
	json_impossible_state_error,
} json_error;

/*
 * Takes a json and returns a map with 
 * the json shallowly copied over it.
 *
 * Places a json_error into '.error' if 
 * an error happened.
 *
 * It only checks if json is valid shallowly. 
 *
 * #to-review
 */
cels_warn_unused
estring_map jsons_unmake(const string *json, const allocator *mem);

/*
 * Makes a json from a map.
 *
 * Places a json_error into '.error' 
 * if an error happened.
 *
 * #to-review
 */
cels_warn_unused
estring jsons_make(const string_map *self, const allocator *mem);

#endif
