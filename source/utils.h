#ifndef cels_utils_h
#define cels_utils_h

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stddef.h>
#include "errors.h"


/*
 * The module 'utils' is a collection 
 * of useful functions.
 */


/* utils */

typedef clock_t (*measurefunc) (size_t);

/*
 * A convenience over utils_measure
 * that provides the function name.
 *
 * #to-review
 */
#define utils_measure(callback) utils_measure_helper(#callback, callback)

/*
 * A function that measure the 
 * performance of the callback in 
 * buckets of time and prints to 
 * the terminal.
 *
 * Don't use it, prefer 
 * utils_measure intead.
 *
 * #to-review
 */
void utils_measure_helper(const char *function_name, measurefunc callback);

/*
 * A convenience over 'next' which 
 * encapsulates the iterator within 
 * scope.
 *
 * #to-review
 */
#define range(var, start, end, step, body) { \
	iterator var = {0}; \
	while (next(start, end, step, &var)) body \
}


/* next */

typedef struct iterator {
	ssize_t data;
	bool is_init;
} iterator;

/*
 * A generic number iterator. 
 * 'start' is inclusive and 'end' 
 * is exclusive.
 * 
 * Iterator must be initialized to 0.
 *
 * #to-review
 */
bool next(ssize_t start, ssize_t end, size_t step, iterator *iterator);

#endif
