#ifndef jsons_h 
#define jsons_h

#include <stdbool.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include "strings.h"

typedef enum json_error {
	json_successfull,
	json_invalid_error,
	json_invalid_state_error,
} json_error;


/*
 * Creates a intermediary representation of a 
 * json which is the basis of the json parser.
 *
 * Places a json_error into '.error' if 
 * an error happened.
 *
 * #to-review
 */
__attribute_warn_unused_result__
estring jsons_describe(const string *json, const allocator *mem);

/*
 * Takes a description given by 'jsons_describe' 
 * function and resumes it, so that a shallow 
 * parsing may take place.
 *
 * Places a json_error into '.error' if 
 * an error happened.
 *
 * #to-review
 */
__attribute_warn_unused_result__
estring jsons_resume(const string *description, const allocator *mem);

/*
 * Takes a json and returns a map with 
 * the json shallowly copied over it.
 *
 * Places a json_error into '.error' if 
 * an error happened.
 *
 * #to-review
 */
__attribute_warn_unused_result__
estring_map jsons_unmake(const string *json, const allocator *mem);

/*
 * Makes a json from a map.
 *
 * Places a json_error into '.error' if 
 * an error happened.
 *
 * #to-review
 */
__attribute_warn_unused_result__
estring jsons_make(const string_map *self, const allocator *mem);

#endif
