#ifndef vectors_h
#define vectors_h

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "errors.h"
#include "mems.h"


/*
 * The module 'vectors' is about the manipulation
 * of a dynamic array structure (aka vector). 
 */


/* vectors */

#define vector_min 4

#define vectors(type0) \
	struct { \
		size_t size; \
		size_t capacity; \
		type0 *data; \
		size_t type_size; \
	}

typedef vectors(void) vector;
typedef vectors(size_t) size_vec;

/*
 * Creates an automatic vector (aka a normal list with size) 
 * with items supplied.
 *
 * #automatic #tested
 */
#define vectors_premake(type, ...) { \
	.capacity=sizeof((type[]){__VA_ARGS__})/sizeof(type), \
	.size=sizeof((type[]){__VA_ARGS__})/sizeof(type), \
	.data=(type []){__VA_ARGS__}, \
	.type_size=sizeof(type) \
}

/*
 * Ranges through vector calling callback.
 *
 * #to-review
 */
#define vectors_prerange(self, callback, ...) { \
	for (size_t i = 0; i < self.size; i++) { \
		callback(&self.data[i] cels_vargs(__VA_ARGS__));\
	} \
}

/*
 * Checks shallowly if vector was properly initialized.
 *
 * #tested
 */
cels_warn_unused
bool vectors_check(const vector *self);

/*
 * Print's a debug-friendly message of vector's struct. 
 *
 * #to-review
 */
void vectors_debug(const void *self, printfunc printer);

/*
 * Initializes a vector with capacity.
 *
 * An uninitialized vector-like 'self' must be provided, 
 * as well as it's underlying type as 'type-size'.
 *
 * #to-review
 */
error vectors_init(
	void *self, size_t type_size, size_t capacity, const allocator *mem);

/*
 * Upscales a vector.
 *
 * A vector-like 'self' must be provided, 
 * as well as it's underlying type as 'type-size'.
 *
 * #to-review
 */
error vectors_upscale(void *self, const allocator *mem);

/*
 * Downscale a vector.
 *
 * A vector-like 'self' must be provided, 
 * as well as it's underlying type as 'type-size'.
 *
 * #to-review
 */
error vectors_downscale(void *self, const allocator *mem);

/*
 * Pops an item from a vector.
 *
 * A vector-like 'self' must be provided, 
 * as well as it's underlying type as 'type-size'.
 *
 * An optional cleaner may be provided to free any 
 * underlying allocations. 
 *
 * #to-review
 */
error vectors_pop(void *self, freefunc cleaner, const allocator *mem);

/*
 * Pushes an item from a vector.
 *
 * A vector-like 'self' must be provided, 
 * as well as it's underlying type as 'type-size'.
 *
 * An 'item' of such underlying type must also 
 * be provided. 
 *
 * #to-review
 */
error vectors_push(void *self, void *item, const allocator *mem);

/*
 * Frees a vector.
 *
 * A vector-like 'self' must be provided, 
 * as well as it's underlying type as 'type-size'.
 *
 * An optional cleaner may be provided to clean 
 * any underlying allocations.
 *
 * #to-review
 */
void vectors_free(void *self, freefunc cleaner, const allocator *mem);

/*
 * Downscales a vector to fit in optimum space.
 *
 * A vector-like 'self' must be provided, 
 * as well as it's underlying type as 'type-size'.
 *
 * #to-review
 */
error vectors_fit(void *self, const allocator *mem);

/*
 * Gets item of vector.
 *
 * #to-review
 */
void *vectors_get(const void *self, ssize_t position);

/*
 * Sorts vector.
 *
 * #to-review
 */
void vectors_sort(void *self, void *temp, compfunc compare);

/*
 * Matches both vectors, having provided 'comparer'; 
 *
 * #to-review
 */
bool vectors_match(const void *self, const void *other, compfunc comparer);

/*
 * Filters unwanted items from vector.
 * 
 * A 'filter' function must be provided, 
 * while a 'cleaner' function may be provided 
 * to do extra cleanup.
 *
 * #to-review
 */
error vectors_filter(
	void *self, filterfunc filter, freefunc cleaner, const allocator *mem);

/*
 * Filter vector uniquely.
 *
 * A 'comparer' function must be provided, 
 * while a 'cleaner' function may be provided 
 * to do extra cleanup.
 *
 * #to-review
 */
error vectors_filter_unique(
	void *self, compfunc comparer, freefunc cleaner, const allocator *mem);

/*
 * Shifts vector by amount 'amount' starting at 
 * position 'position'.
 *
 * A 'cleaner' function may be provided to 
 * do extra cleanup.
 *
 * #to-review
 */
void vectors_shift(
	void *self, 
	size_t position, 
	size_t amount, 
	freefunc cleaner, 
	notused const allocator *mem);

/*
 * Finds item in vector provided 'comparer' function.
 *
 * #to-review
 */
ssize_t vectors_find(const void *self, void *item, compfunc comparer);

/*
 * Unites two vectors.
 *
 * #to-review
 */
error vectors_unite(void *self, own void* other, notused const allocator *mem);

/*
 * Calls 'callback' for each item.
 *
 * #to-review
 */
void vectors_do(void *self, dofunc callback);

/*
 * Ranges through items, calling 'callback' 
 * which receives an additional argument.
 *
 * #to-review
 */
void vectors_range(void *self, shoutfunc callback, void *args);

#endif
