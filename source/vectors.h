#ifndef vectors_h
#define vectors_h

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/cdefs.h>

#include "errors.h"

typedef struct vector {
	size_t size;
	size_t capacity;
	void *data;
} vector;

#define vectors(t) struct { \
    size_t size; \
    size_t capacity; \
    typeof(t) *data; \
}

typedef vectors(size_t) size_vec;

#define vectors_min 16

#include "utils.h"

/*
 * The module 'vectors' is about the manipulation
 * of a dynamic array structure (aka vector). 
 *
 */

/*
 * Creates an automatic vector (aka a normal list with size) 
 * with items supplied.
 *
 * #automatic #tested
 */
#define vectors_premake(type, len, ...) \
	{.capacity=len, .size=len, .data=(type [len]){__VA_ARGS__}}

/*
 * Creates an allocated vector with len being the 
 * size of the type followed by the vector capacity.
 *
 * Should be check'd afterwards.
 *
 * #allocates #may-fail #depends:stdio.h #posix-reliant #tested
 */
#define vectors_init(len, cap) \
	{.data=calloc(cap, len), .size=0, .capacity=cap}

/*
 * Pushes item (to be owned) to v.
 * Requiring further capacity, it automaticaly upscales.
 *
 * Shouldn't be used with automatic variables.
 *
 * If it fails to realloc, true is assigned to error.
 * If error is null silent errors happens.
 *
 * #allocates #may-fail #depends:stdio.h #posix-reliant #tested
 */
#define vectors_push(v, item, error) { \
    v.size++; \
	v.data[v.size - 1] = item; \
    if (v.size >= v.capacity) { \
		vectors_upscale((vector *)&v, sizeof(typeof(item)), error); \
	} \
}

/*
 * Frees v.data shallowly.
 *
 * Shouldn't be used with automatic variables.
 *
 * #may-fail #depends:stdio.h #posix-reliant #tested
 */
#define vectors_free(v) \
	if (v.data != NULL) { \
		free(v.data); \
		v.data = NULL; \
	}

/*
 * Sorts v depending on compare func 
 * (of type compfunc, defined in utils.h).
 *
 * It uses insert-sort algorithm.
 *
 * #tested
 */
#define vectors_sort(v, compare) \
	for (size_t i = v.size; i > 0; i--) { \
		for (size_t j = 1; j < i; j++) { \
			bool is_bigger = compare(&v.data[j - 1], &v.data[j]); \
			if (is_bigger) { \
				maths_swap(v.data[j], v.data[j - 1]); \
			} \
		} \
	}

/*
 * Prints a debug-friendly message 
 * about v's information.
 *
 * #debug
 */
#define vectors_debug(v) \
	printf( \
		#v "<vector>{size: %zu, capacity: %zu, data: %p}\n", \
		v.size, v.capacity, (void *)v.data); \

/*
 * Prints elements in vector v0 provided a 
 * print function (printvecfunc - definition 
 * found in utils.h).
 */
#define vectors_print(v0, print) \
	for (size_t i = 0; i < v0->size; i++) { \
		print(v0->data, &i); \
	}

/*
 * Checks shallowly if v was properly initialized.
 *
 * #tested
 */
__attribute_warn_unused_result__
bool vectors_check(const vector *v);

/*
 * Upscales v having type size informed.
 * If it fails to realloc sets error to 
 * true if not null. 
 * If null, it silently errors.
 *
 * #allocates #may-fail #depends:stdio.h 
 * #posix-reliant #tested
 */
void vectors_upscale(vector *v, size_t type_size, bool *error);

/*
 * Compares if two vectors are equal 
 * returning true if they are. A comparison 
 * function (compvecfunc, provided in utils.h) 
 * must be provided.
 *
 * #tested
 */
__attribute_warn_unused_result__
bool vectors_equals(const vector *v0, const vector *v1, compvecfunc compare);

/*
 * Compares literally if two vectors are equal 
 * returning true if they are. An object_size 
 * must be provided, ideally like 'sizeof(int)', 
 * for indexing and iterating over vectors.
 *
 * #danger
 */
__attribute_warn_unused_result__
bool vectors_sized_equals(const vector *v0, const vector *v1, size_t object_size);

/*
 * Finds first match of item in vector, giving, 
 * type's size is provided, than it returns the 
 * position on the list or -1 for not found.
 *
 * #danger #tested
 */
__attribute_warn_unused_result__
ssize_t vectors_sized_find(const vector *v, void *item, size_t object_size);

#endif
