#ifndef vectors_h
#define vectors_h

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/cdefs.h>

#include "errors.h"

#define vectors(t) struct { \
    size_t size; \
    size_t capacity; \
    typeof(t) data; \
}

typedef vectors(void *) vector;

#define vectors_min 8

#include "mems.h"
//#include "utils.h"

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
 * #allocates #may-fail #depends:stdio.h #posix-reliant #tested #to-edit
 */
#define vectors_init(type, len, mem) \
	{.data=(type *)mems_alloc(mem, sizeof(type) * len), .capacity=len}

/*
 * Pushes item (to be owned) to vector.
 * Requiring further capacity, it automaticaly upscales.
 *
 * Shouldn't be used with automatic variables.
 *
 * If it fails to realloc, true is assigned to error.
 * If error is null silent errors happens.
 *
 * #allocates #may-fail #depends:stdio.h #posix-reliant #tested #to-edit
 */
#define vectors_push(self, item, mem, error) { \
    self.size++; \
	self.data[self.size - 1] = item; \
    if (self.size >= self.capacity) { \
		vectors_upscale((vector *)&self, sizeof(typeof(item)), mem, error); \
	} \
}

/*
 * Frees vector.data shallowly.
 *
 * Shouldn't be used with automatic variables.
 *
 * #may-fail #depends:stdio.h #posix-reliant #tested #to-edit
 */
#define vectors_free(self, mem) \
	if (self.data != null) { \
		mems_dealloc(mem, self.data, self.capacity * sizeof(*self.data)); \
		self.data = null; \
	}

/*
 * Sorts vector depending on compare func 
 * (of type compfunc, defined in utils.h).
 *
 * It uses insert-sort algorithm.
 *
 * #tested
 */
#define vectors_sort(self, compare) \
	for (size_t i = self.size; i > 0; i--) { \
		for (size_t j = 1; j < i; j++) { \
			bool is_bigger = compare(&self.data[j - 1], &self.data[j]); \
			if (is_bigger) { \
				maths_swap(self.data[j], self.data[j - 1]); \
			} \
		} \
	}

/*
 * Prints a debug-friendly message 
 * about vector's information.
 *
 * #debug
 */
#define vectors_debug(self) \
	printf( \
		#self "<vector>{size: %zu, capacity: %zu, data: %p}\n", \
		self.size, self.capacity, (void *)self.data); \

/*
 * Prints elements in vector, provided a 
 * print function (printvecfunc - definition 
 * found in utils.h).
 */
#define vectors_print(self, print) \
	for (size_t i = 0; i < self->size; i++) { \
		print(self->data, &i); \
	}

/*
 * #to-review
 */
#define vectors_generate_implementation(type, name) \
	bool name##s_equals(const name *v0, const name *v1) { \
		if (v0->size != v1->size) { return false; } \
		for (size_t i = 0; i < v0->size; i++) { \
			if (v0->data[i] != v1->data[i]) { \
				return false; \
			} \
		} \
		\
		return true; \
	} \
	\
	ssize_t name##s_find(const name *self, type item) { \
		for (size_t i = 0; i < self->size; i++) { \
			if (self->data[i] == item) { \
				return i; \
			} \
		} \
		\
		return -1; \
	}

#define vectors_generate_definition(type, name) \
	typedef vectors(type *) name; \
	\
	bool name##s_equals(const name *v0, const name *v1); \
	\
	ssize_t name##s_find(const name *self, type item)

vectors_generate_definition(size_t, size_vec);

/*
 * Checks shallowly if vecto was properly initialized.
 *
 * #tested
 */
__attribute_warn_unused_result__
bool vectors_check(const vector *self);

/*
 * Upscales vector having type size informed.
 * If it fails to realloc sets error to 
 * true if not null. 
 * If null, it silently errors.
 *
 * #allocates #may-fail #depends:stdio.h 
 * #posix-reliant #tested #to-edit
 */
void vectors_upscale(vector *self, size_t object_size, const allocator *mem, bool *error);

#endif
