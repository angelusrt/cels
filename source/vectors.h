#ifndef vectors_h
#define vectors_h

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/cdefs.h>

#include "errors.h"

//

#ifndef cels_debug
#define cels_debug false
#endif

//

#define vectors(t) struct { \
    size_t size; \
    size_t capacity; \
    typeof(t) data; \
}

typedef vectors(void *) vector;

#define vectors_min 8

#include "mems.h"
#include "utils.h"

/*
 * The module 'vectors' is about the manipulation
 * of a dynamic array structure (aka vector). 
 */

/*
 * Creates an allocated vector with len being the 
 * size of the type followed by the vector capacity.
 *
 * Should be check'd afterwards.
 *
 * #allocates #may-fail #depends:stdio.h #posix-reliant #tested #to-edit
 * vectors_init(len, mem) -> vector
 */

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
 * vectors_push(self, item, mem) -> error
 */

/*
 * Frees vector.data shallowly.
 *
 * Shouldn't be used with automatic variables.
 *
 * #may-fail #depends:stdio.h #posix-reliant #tested #to-edit
 * vectors_free(self, mem) -> void
 */

/*
 * Sorts vector depending on compare func 
 * (of type compfunc, defined in utils.h).
 *
 * It uses insert-sort algorithm.
 *
 * #tested
 * vectors_sort(self, compare) -> void
 */

/*
 * Prints a debug-friendly message 
 * about vector's information.
 *
 * #debug
 * vectors_debug(self) -> void
 */

/*
 * Prints elements in vector, provided a 
 * print function (printvecfunc - definition 
 * found in utils.h).
 * vectors_print(self, print) -> void
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
 * Generates all type-specific functions for vectors.
 * #to-review
 */
#define vectors_generate_implementation( \
	type, name, check0, print0, compare0, compare1, cleanup0 \
) \
	name name##s_init(size_t len, const allocator *mem) { \
		return (name) { \
			.data=(type *)mems_alloc(mem, sizeof(type) * len), \
			.capacity=len \
		}; \
	} \
	\
	bool name##s_push(name *self, type item, const allocator *mem) { \
		if (cels_debug) { \
			errors_panic(utils_fcat(".self"), vectors_check((const vector *)self)); \
			errors_panic(utils_fcat(".item"), check0(&item)); \
		} \
		\
		self->size++; \
		self->data[self->size - 1] = item; \
		if (self->size >= self->capacity) { \
			size_t new_capacity = self->capacity << 1; \
			void *new_data = mems_realloc( \
				mem, \
				self->data, \
				self->capacity * sizeof(type), \
				new_capacity * sizeof(type)); \
			\
			if (new_data == NULL) { \
				self->size--; \
				return true; \
			} \
			\
			self->capacity = new_capacity; \
			self->data = new_data; \
			\
		} \
		return false; \
	} \
	\
	void name##s_free(name *self, const allocator *mem) { \
		if (cels_debug) { \
			errors_panic(utils_fcat(".self"), vectors_check((const vector *)self)); \
		} \
		\
		if (self->data != null) { \
			for (size_t i = 0; i < self->size; i++) { \
				cleanup0(&self->data[i], mem); \
			} \
			\
			mems_dealloc(mem, self->data, self->capacity * sizeof(*self->data)); \
			self->data = null; \
		} \
	} \
	\
	void name##s_sort(name *self, compfunc compare) {\
		if (cels_debug) { \
			errors_panic(utils_fcat(".self"), vectors_check((const vector *)self)); \
		} \
		\
		for (size_t i = self->size; i > 0; i--) { \
			for (size_t j = 1; j < i; j++) { \
				bool is_bigger = compare(&self->data[j - 1], &self->data[j]); \
				if (is_bigger) { \
					maths_swap(self->data[j], self->data[j - 1]); \
				} \
			} \
		} \
	} \
	\
	void name##s_debug(const name *self) { \
		if (cels_debug) { \
			errors_panic(utils_fcat(".self"), vectors_check((const vector *)self)); \
		} \
		\
		printf( \
			"<" #name ">{size: %zu, capacity: %zu, data: %p}\n", \
			self->size, self->capacity, (void *)self->data); \
	} \
	\
	void name##s_print(const name *self) { \
		if (cels_debug) { \
			errors_panic(utils_fcat(".self"), vectors_check((const vector *)self)); \
		} \
		\
		for (size_t i = 0; i < self->size; i++) { \
			print0(&self->data[i]); \
		} \
	} \
	\
	bool name##s_equals(const name *v0, const name *v1) { \
		if (cels_debug) { \
			errors_panic(utils_fcat(".v0"), vectors_check((const vector *)v0)); \
			errors_panic(utils_fcat(".v1"), vectors_check((const vector *)v1)); \
		} \
		\
		if (v0->size != v1->size) { return false; } \
		for (size_t i = 0; i < v0->size; i++) { \
			if (!compare0(&v0->data[i], &v1->data[i])) { \
				return false; \
			} \
		} \
		\
		return true; \
	} \
	\
	bool name##s_seems(const name *v0, const name *v1) { \
		if (cels_debug) { \
			errors_panic(utils_fcat(".v0"), vectors_check((const vector *)v0)); \
			errors_panic(utils_fcat(".v1"), vectors_check((const vector *)v1)); \
		} \
		\
		if (v0->size != v1->size) { return false; } \
		for (size_t i = 0; i < v0->size; i++) { \
			if (!compare1(&v0->data[i], &v1->data[i])) { \
				return false; \
			} \
		} \
		\
		return true; \
	} \
	\
	ssize_t name##s_find(const name *self, type item) { \
		if (cels_debug) { \
			errors_panic(utils_fcat(".self"), vectors_check((const vector *)self)); \
			errors_panic(utils_fcat(".item"), check0(&item)); \
		} \
		\
		for (size_t i = 0; i < self->size; i++) { \
			if (compare0(&self->data[i], &item)) { \
				return i; \
			} \
		} \
		\
		return -1; \
	} \
	\
	ssize_t name##s_search(const name *self, type item) { \
		if (cels_debug) { \
			errors_panic(utils_fcat(".self"), vectors_check((const vector *)self)); \
			errors_panic(utils_fcat(".item"), check0(&item)); \
		} \
		\
		for (size_t i = 0; i < self->size; i++) { \
			if (compare1(&self->data[i], &item)) { \
				return i; \
			} \
		} \
		\
		return -1; \
	}

/*
 * Generates all type-specific function-definitions for vectors.
 * #to-review
 */
#define vectors_generate_definition(type, name) \
	typedef vectors(type *) name; \
	\
	__attribute_warn_unused_result__ \
	name name##s_init(size_t len, const allocator *mem); \
	\
	bool name##s_push(name *self, type item, const allocator *mem); \
	\
	void name##s_free(name *self, const allocator *mem); \
	\
	void name##s_sort(name *self, compfunc compare); \
	\
	void name##s_debug(const name *self); \
	\
	void name##s_print(const name *self); \
	\
	__attribute_warn_unused_result__ \
	bool name##s_equals(const name *v0, const name *v1); \
	\
	__attribute_warn_unused_result__ \
	bool name##s_seems(const name *v0, const name *v1); \
	\
	__attribute_warn_unused_result__ \
	ssize_t name##s_find(const name *self, type item); \
	\
	__attribute_warn_unused_result__ \
	ssize_t name##s_search(const name *self, type item);

/**
 * Checks shallowly if vector was properly initialized.
 *
 * #tested
 */
__attribute_warn_unused_result__
bool vectors_check(const vector *self);

/* definitions */

vectors_generate_definition(size_t, size_vec)

#endif
