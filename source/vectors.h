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
#define vector_min 4

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
#define vectors_premake(type, ...) { \
	.capacity=sizeof((type[]){__VA_ARGS__})/sizeof(type), \
	.size=sizeof((type[]){__VA_ARGS__})/sizeof(type), \
	.data=(type []){__VA_ARGS__} \
}

/*
 * Generates all type-specific functions for vectors.
 * #to-review
 */
#define vectors_generate_implementation( \
	type, name, check0, clone0, print0, compare0, compare1, cleanup0 \
) \
	name name##s_init(size_t len, const allocator *mem) { \
		name self = { \
			.data=(type *)mems_alloc(mem, sizeof(type) * len), \
			.capacity=len \
		}; \
		\
		errors_abort("self.data", self.data == null); \
		\
		return self; \
	} \
	\
	name name##s_clone(name *self, const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((const vector *)self)); \
		} \
		\
		name other = name##s_init(vector_min, mem); \
		\
		for (size_t i = 0; i < self->size; i++) { \
			type item = clone0(&self->data[i], mem); \
			name##s_push(&other, item, mem); \
		} \
		\
		return other; \
	} \
	\
	error name##s_upscale(name *self, const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((const vector *)self)); \
		} \
		\
		size_t new_capacity = self->capacity << 1; \
		void *new_data = mems_realloc( \
			mem, \
			self->data, \
			self->capacity * sizeof(type), \
			new_capacity * sizeof(type)); \
		\
		if (cels_debug) { \
			errors_inform("new_data", !new_data); \
		} \
		\
		if (!new_data) { \
			self->size--; \
			return fail; \
		} \
		\
		self->capacity = new_capacity; \
		self->data = new_data; \
		\
		return ok; \
	} \
	\
	error name##s_downscale(name *self, const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((const vector *)self)); \
		} \
		\
		if (self->size < self->capacity >> 1) { \
			size_t new_capacity = self->capacity >> 1; \
			void *new_data = mems_realloc( \
				mem, \
				self->data, \
				self->capacity * sizeof(type), \
				new_capacity * sizeof(type)); \
			\
			if (cels_debug) { \
				errors_inform("new_data", !new_data); \
			} \
			\
			if (!new_data) { \
				self->size++; \
				return fail; \
			} \
			\
			self->capacity = new_capacity; \
			self->data = new_data; \
			\
		} \
		\
		return ok; \
	} \
	\
	/* #to-review */ \
	type name##s_get(const name *self, size_t position) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((const vector *)self)); \
		} \
		\
		if (self->size == 0) { \
			return (type){0}; \
		} else if (position > self->size - 1) { \
			return self->data[self->size - 1]; \
		} \
		\
		return self->data[position]; \
	} \
	\
	error name##s_pop(name *self, const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((const vector *)self)); \
		} \
		\
		cleanup0(&self->data[self->size - 1], mem); \
		self->size--; \
		error downscale_error = ok; \
		\
		if (self->size < self->capacity >> 1) { \
			downscale_error = name##s_downscale(self, mem); \
		} \
		\
		return downscale_error; \
	} \
	\
	/* #to-review */\
	type name##s_toss(name *self, const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((const vector *)self)); \
		} \
		\
		if (self->size == 0) { \
			return (type){0}; \
		} \
		\
		type item = self->data[self->size - 1]; \
		self->size--; \
		\
		if (self->size < self->capacity >> 1) { \
			name##s_downscale(self, mem); \
		} \
		\
		return item; \
		\
	} \
	\
	error name##s_push(name *self, type item, const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((const vector *)self)); \
			errors_abort("item", check0(&item)); \
		} \
		\
		self->size++; \
		self->data[self->size - 1] = item; \
		error upscale_error = ok; \
		\
		if (self->size >= self->capacity) { \
			upscale_error = name##s_upscale(self, mem); \
		} \
		\
		return upscale_error; \
	} \
	\
	error name##s_cpush(name *self, type item, const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((const vector *)self)); \
			errors_abort("item", check0(&item)); \
		} \
		\
		error push_error = name##s_push(self, item, mem); \
		if (push_error) { \
			cleanup0(&item, mem); \
			return fail; \
		} \
		\
		return ok; \
	} \
	\
	error name##s_fpush(name *self, type item, const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((const vector *)self)); \
			errors_abort("item", check0(&item)); \
		} \
		\
		error push_error = name##s_push(self, item, mem); \
		if (push_error) { \
			cleanup0(&item, mem); \
			name##s_free(self, mem); \
			return fail; \
		} \
		\
		return ok; \
	} \
	\
	void name##s_free(name *self, const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((const vector *)self)); \
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
			errors_abort("self", vectors_check((const vector *)self)); \
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
	error name##s_filter(name *self, filterfunc filter, const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((const vector *)self)); \
		} \
		\
		name other = name##s_init(vector_min, mem); \
		\
		for (size_t i = 0; i < self->size; i++) { \
			if (filter(&self->data[i])) { \
				error push_error = name##s_push(&other, self->data[i], mem); \
				if (push_error) { \
					return fail; \
				} \
			} else { \
				cleanup0(&self->data[i], mem); \
			} \
		} \
		\
		error dealloc_error = mems_dealloc(mem, self->data, self->capacity); \
		if (dealloc_error) { \
			return fail; \
		} \
		\
		*self = other; \
		\
		return ok; \
	} \
	\
	error name##s_filter_unique(name *self, const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((void *)self)); \
		} \
		\
		name other = name##s_init(vector_min, mem); \
 		\
		for (size_t i = 0; i < self->size; i++) { \
			bool match = false; \
			for (size_t j = 0; j < other.size; j++) { \
				if (compare0(&self->data[i], &other.data[j])) { \
					cleanup0(&self->data[i], mem); \
					match = true; \
					break; \
				} \
			} \
 			\
			if (!match) { \
				error push_error = name##s_push(&other, self->data[i], mem); \
				if (push_error) { \
					return fail; \
				} \
			} \
		} \
 		\
		error dealloc_error = mems_dealloc(mem, self->data, self->capacity); \
		if (dealloc_error) { \
			return fail; \
		} \
		\
		*self = other; \
 		\
		return ok; \
	} \
	\
	void name##s_debug(const name *self) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((void *)self)); \
		} \
		\
		printf( \
			"<" #name ">{size: %zu, capacity: %zu, data: %p}\n", \
			self->size, self->capacity, (void *)self->data); \
	} \
	\
	void name##s_print(const name *self) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((void *)self)); \
		} \
		\
		for (size_t i = 0; i < self->size; i++) { \
			print0(&self->data[i]); \
		} \
	} \
	\
	bool name##s_equals(const name *self, const name *other) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((void *)self)); \
			errors_abort("other", vectors_check((void *)other)); \
		} \
		\
		if (self->size != other->size) { \
			return false; \
		} \
		\
		for (size_t i = 0; i < self->size; i++) { \
			if (!compare0(&self->data[i], &other->data[i])) { \
				return false; \
			} \
		} \
		\
		return true; \
	} \
	\
	bool name##s_seems(const name *self, const name *other) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((void *)self)); \
			errors_abort("other", vectors_check((void *)other)); \
		} \
		\
		if (self->size != other->size) { \
			return false; \
		} \
		\
		for (size_t i = 0; i < self->size; i++) { \
			if (!compare1(&self->data[i], &other->data[i])) { \
				return false; \
			} \
		} \
		\
		return true; \
	} \
	\
	ssize_t name##s_find(const name *self, type item) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((void *)self)); \
			errors_abort("item", check0(&item)); \
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
			errors_abort("self", vectors_check((void *)self)); \
			errors_abort("item", check0(&item)); \
		} \
		\
		for (size_t i = 0; i < self->size; i++) { \
			if (compare1(&self->data[i], &item)) { \
				return i; \
			} \
		} \
		\
		return -1; \
	} \
	\
	void name##s_shift(name *self, size_t position, notused const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((void *)self)); \
		} \
		\
		if (position + 1 >= self->size) { \
			return; \
		} \
		\
		cleanup0(&self->data[position], mem); \
		\
		for (size_t i = position; i < self->size - 1; i++) { \
			self->data[i] = self->data[i + 1]; \
		} \
		\
		self->size--; \
		\
		return; \
	} \
	\
	/* #to-review */ \
	error name##s_unite(name *self, name* other, notused const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((void *)self)); \
			errors_abort("other", vectors_check((void *)other)); \
		} \
		\
		for (size_t i = 0; i < other->size; i++) { \
			error push_error = name##s_push(self, other->data[i], mem); \
			if (push_error) { \
				return fail; \
			} \
		} \
 		\
		error dealloc_error = mems_dealloc(mem, other->data, other->capacity); \
		if (dealloc_error) { \
			return fail; \
		} \
		\
		other->size = 0; \
 		\
		return ok; \
	} \

/*
 * Generates all type-specific function-definitions for vectors.
 * #to-review
 */
#define vectors_generate_definition(type, name) \
	typedef vectors(type *) name; \
	\
	__attribute__ ((__warn_unused_result__)) \
	name name##s_init(size_t len, const allocator *mem); \
	\
	__attribute__ ((__warn_unused_result__)) \
	name name##s_clone(name *self, const allocator *mem); \
	\
	error name##s_upscale(name *self, const allocator *mem); \
	\
	error name##s_downscale(name *self, const allocator *mem); \
	\
	__attribute__ ((__warn_unused_result__)) \
	type name##s_get(const name *self, size_t position); \
	\
	error name##s_pop(name *self, const allocator *mem); \
	\
	type name##s_toss(name *self, const allocator *mem); \
	\
	error name##s_push(name *self, type item, const allocator *mem); \
	\
	error name##s_cpush(name *self, type item, const allocator *mem); \
	\
	error name##s_fpush(name *self, type item, const allocator *mem); \
	\
	void name##s_free(name *self, const allocator *mem); \
	\
	void name##s_sort(name *self, compfunc compare); \
	\
	error name##s_filter(name *self, filterfunc filter, const allocator *mem); \
	\
	error name##s_filter_unique(name *self, const allocator *mem); \
	\
	void name##s_debug(const name *self); \
	\
	void name##s_print(const name *self); \
	\
	__attribute__ ((__warn_unused_result__)) \
	bool name##s_equals(const name *self, const name *other); \
	\
	__attribute__ ((__warn_unused_result__)) \
	bool name##s_seems(const name *self, const name *other); \
	\
	__attribute__ ((__warn_unused_result__)) \
	ssize_t name##s_find(const name *self, type item); \
	\
	__attribute__ ((__warn_unused_result__)) \
	ssize_t name##s_search(const name *self, type item); \
	\
	void name##s_shift(name *self, size_t position, notused const allocator *mem); \
	\
	error name##s_unite(name *self, name* other, const allocator *mem);

/**
 * Checks shallowly if vector was properly initialized.
 *
 * #tested
 */
__attribute__ ((__warn_unused_result__))
bool vectors_check(const vector *self);

/*
 * Print's a debug-friendly message of vector's struct. 
 *
 * #to-review
 */
void vectors_debug(const vector *self);

/* definitions */

vectors_generate_definition(size_t, size_vec)

#endif
