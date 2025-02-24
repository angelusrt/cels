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

#define vector_min 4

#define vectors(type0) \
	struct { \
		size_t size; \
		size_t capacity; \
		type0 data; \
	}

typedef vectors(void *) vector;

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
 * Ranges through vector calling callback.
 *
 * #to-review
 */
#define vectors_range(self, callback, ...) { \
	for (size_t i = 0; i < self.size; i++) { \
		callback(&self.data[i] cels_vargs(__VA_ARGS__));\
	} \
} \

/*
 * Generates all type-specific functions for vectors.
 *
 * #to-review
 */
#define vectors_generate( \
	name, type, check0, clone0, print0, debug0, compare0, compare1, cleanup0) \
	name name##s_init(size_t capacity, const allocator *mem) { \
		name self = { \
			.data=(type *)mems_alloc(mem, sizeof(type) * capacity), \
			.capacity=capacity \
		}; \
		\
		errors_abort("self.data", !self.data); \
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
	type name##s_get(const name *self, ssize_t position) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((const vector *)self)); \
		} \
		\
		if (self->size == 0) { \
			return self->data[0]; \
		} else if (position > (ssize_t)self->size - 1) { \
			return self->data[self->size - 1]; \
		} else if (position < 0 && ((ssize_t)self->size - position) >= 0) { \
			return self->data[(ssize_t)self->size - position]; \
		} \
		\
		return self->data[position]; \
	} \
	\
	error name##s_upscale(name *self, const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((const vector *)self)); \
		} \
		\
		size_t new_capacity = self->capacity << 1; \
		\
		if (cels_debug) { \
			errors_abort( \
				"new_capacity (overflow)", \
				new_capacity < self->capacity); \
		} \
		\
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
	error name##s_pop(name *self, const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((const vector *)self)); \
		} \
		\
		bool is_oversized = false; \
		if (self->size < self->capacity >> 1) { \
			is_oversized = true; \
		} \
		\
		cleanup0(&self->data[self->size - 1], mem); \
		self->size--; \
		error downscale_error = ok; \
		\
		if (!is_oversized && self->size < self->capacity >> 1) { \
			downscale_error = name##s_downscale(self, mem); \
		} \
		\
		return downscale_error; \
	} \
	\
	type name##s_toss(name *self, const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((const vector *)self)); \
		} \
		\
		if (self->size == 0) { \
			return self->data[0]; \
		} \
		\
		type item = self->data[self->size - 1]; \
		self->size--; \
		\
		if (self->size < self->capacity >> 1) { \
			name##s_downscale(self, mem); \
			/*TODO: should treat error*/\
		} \
		\
		return item; \
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
	error name##s_press(name *self, type item, const allocator *mem) { \
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
	error name##s_force(name *self, type item, const allocator *mem) { \
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
		if (!self->data) { \
			for (size_t i = 0; i < self->size; i++) { \
				cleanup0(&self->data[i], mem); \
			} \
			\
			mems_dealloc(mem, self->data, self->capacity * sizeof(type)); \
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
			if (cels_debug) { \
				errors_abort("self.data[i]", check0(&self->data[i])); \
			} \
			\
			if (filter(&self->data[i])) { \
				error push_error = name##s_force(&other, self->data[i], mem); \
				if (push_error) { return fail; } \
			} else { \
				cleanup0(&self->data[i], mem); \
			} \
		} \
		\
		error dealloc_error = mems_dealloc(mem, self->data, self->capacity); \
		if (dealloc_error) { return fail; } \
		\
		*self = other; \
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
			if (cels_debug) { \
				errors_abort("self.data[i]", check0(&self->data[i])); \
			} \
			\
			bool match = false; \
			for (size_t j = 0; j < other.size; j++) { \
				if (compare0(&self->data[i], &other.data[j])) { \
					match = true; \
					break; \
				} \
			} \
 			\
			if (!match) { \
				error push_error = name##s_push(&other, self->data[i], mem); \
				if (push_error) { return fail; } \
			} else { \
				cleanup0(&self->data[i], mem); \
			} \
		} \
 		\
		error dealloc_error = mems_dealloc(mem, self->data, self->capacity); \
		if (dealloc_error) { return fail; } \
		\
		*self = other; \
		\
		if (cels_debug) { \
			errors_abort("self", vectors_check((const vector *)self)); \
			for (size_t i = 0; i < self->size; i++) { \
				if (cels_debug) { \
					errors_abort("self.data[i]", check0(&self->data[i])); \
				} \
			} \
		} \
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
			"<" #name ">{.size: %zu, .capacity: %zu, .data: {",  \
			self->size, \
			self->capacity); \
		\
		for (size_t i = 0; (long)i < (long)self->size; i++) { \
			printf("\""); \
			debug0(&self->data[i]); \
			printf("\""); \
			\
			if (i != self->size - 1) { \
				printf(", "); \
			} \
		} \
		\
		printf("}}"); \
	} \
	\
	void name##s_debugln(const name *self) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((void *)self)); \
		} \
		\
		name##s_debug(self); \
		printf("\n"); \
	} \
	\
	void name##s_print(const name *self) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((void *)self)); \
		} \
		\
		for (size_t i = 0; i < self->size; i++) { \
			print0(&self->data[i]); \
			if (i != self->size - 1) { \
				printf(", "); \
			} \
		} \
	} \
	\
	void name##s_println(const name *self) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((void *)self)); \
		} \
		\
		for (size_t i = 0; i < self->size; i++) { \
			print0(&self->data[i]); \
			printf("\n"); \
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
	void name##s_shift(name *self, size_t position, size_t amount, notused const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((void *)self)); \
		} \
		\
		if (position + amount >= self->size || amount == 0) { \
			return; \
		} \
		\
		for (size_t i = position; i < amount; i++) { \
			cleanup0(&self->data[i], mem); \
		} \
		\
		for (size_t i = position; i < self->size - amount; i++) { \
			self->data[i] = self->data[i + amount]; \
		} \
		\
		self->size -= amount; \
		\
		return; \
	} \
	\
	/* #to-review */ \
	error name##s_unite(name *self, own name* other, notused const allocator *mem) { \
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
	\
	void name##s_range(name *self, shoutfunc callback, void *args) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((void *)self)); \
		} \
		\
		for (size_t i = 0; i < self->size; i++) { \
			if (cels_debug) { \
				errors_abort("self.data[i]", check0(&self->data[i])); \
			} \
			\
			callback(&self->data[i], args);\
		} \
	} \
	\
	error name##s_fit(name *self, const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("self", !self); \
		} \
		\
		while (true) { \
			if (self->size < self->capacity >> 1 && self->capacity > vector_min) { \
				error downscale_error = name##s_downscale(self, mem); \
				if (downscale_error) { \
					return downscale_error; \
				} \
			} else { \
				break; \
			} \
		} \
		\
		return ok; \
	}

/*
 * Generates all type-specific function-definitions 
 * for vectors.
 *
 * #to-review
 */
#define vectors_define(name, type) \
	typedef vectors(type *) name; \
	\
	/*
	 * Creates an allocated vector 
	 * with capacity 'capacity'.
	 *
	 * #allocates #may-panic #tested
	 */ \
	cels_warn_unused \
	name name##s_init(size_t capacity, const allocator *mem); \
	\
	/*
	 * Creates a clone of vector 'self' 
	 *
	 * #allocates #may-panic #tested
	 */ \
	cels_warn_unused \
	name name##s_clone(name *self, const allocator *mem); \
	\
	/* 
	 * Gets 'type' at position 'position' 
	 * with bound-checking defaulting to 
	 * last item.
	 *
	 * It also allows negative position 
	 * to look at the back of the list.
	 */ \
	cels_warn_unused \
	type name##s_get(const name *self, ssize_t position); \
	\
	/*
	 * Upscales vector 'self' to double its capacity.
	 *
	 * #allocates #may-error #tested
	 */ \
	error name##s_upscale(name *self, const allocator *mem); \
	\
	/*
	 * Downscales vector 'self' to half its capacity.
	 *
	 * #allocates #may-error #tested
	 */ \
	error name##s_downscale(name *self, const allocator *mem); \
	\
	/*
	 * Pops item from list, deallocating it 
	 * and downscaling vector if necessary.
	 *
	 * #allocates #may-error
	 */ \
	error name##s_pop(name *self, const allocator *mem); \
	\
	/* Pops up item from list, returning it 
	 * and downscaling vector if necessary. 
	 *
	 * #allocates #to-review
	 */\
	type name##s_toss(name *self, const allocator *mem); \
	\
	/*
	 * Pushes an item to vector 'self', 
	 * upscaling it if necessary.
	 *
	 * #allocates #may-error
	 */ \
	error name##s_push(name *self, type item, const allocator *mem); \
	\
	/*
	 * Pushes an item to 'self' and if an 
	 * error happens it automatically frees item.
	 *
	 * #allocates #may-error
	 */ \
	error name##s_press(name *self, type item, const allocator *mem); \
	\
	/*
	 * Pushes an item to 'self' and if an 
	 * error happens it automatically frees 
	 * item and 'self'.
	 *
	 * #allocates #may-error
	 */ \
	error name##s_force(name *self, type item, const allocator *mem); \
	\
	/*
	 * Resizes vector to fit. 
	 *
	 * #allocates
	 */ \
	error name##s_fit(name *self, const allocator *mem); \
	\
	/*
	 * Frees vector 'self'.
	 *
	 * #allocates
	 */ \
	void name##s_free(name *self, const allocator *mem); \
	\
	/*
	 * Sorts vector according to compare function.
	 * Insert-sort is the algorithm implemented.
	 */ \
	void name##s_sort(name *self, compfunc compare); \
	\
	/*
	 * Filters vector with filter.
	 *
	 * #allocates #may-error
	 */ \
	error name##s_filter(name *self, filterfunc filter, const allocator *mem); \
	\
	/*
	 * Filters vector with unique-items only.
	 *
	 * #allocates #may-error
	 */ \
	error name##s_filter_unique(name *self, const allocator *mem); \
	\
	/*
	 * Prints a debug-friendly message to 
	 * terminal with its internals.
	 */ \
	void name##s_debug(const name *self); \
	\
	/*
	 * Prints a debug-friendly message to 
	 * terminal with its internals and feeds 
	 * it new-line.
	 */ \
	void name##s_debugln(const name *self); \
	\
	/*
	 * Prints items to terminal.
	 */ \
	void name##s_print(const name *self); \
	\
	/*
	 * Prints items in each line to terminal.
	 */ \
	void name##s_println(const name *self); \
	\
	/*
	 * Returns true if and only if 
	 * 'self' and 'other' are equal.
	 *
	 * #case-sensitive
	 */ \
	cels_warn_unused \
	bool name##s_equals(const name *self, const name *other); \
	\
	/*
	 * Returns true if and only if 
	 * 'self' and 'other' are alike.
	 *
	 * #case-insensitive
	 */ \
	cels_warn_unused \
	bool name##s_seems(const name *self, const name *other); \
	\
	/*
	 * Finds item within self and returns position.
	 *
	 * If item isn't found -1 is returned instead.
	 *
	 * #case-sensitive
	 */ \
	cels_warn_unused \
	ssize_t name##s_find(const name *self, type item); \
	\
	/*
	 * Finds item within self and returns position.
	 *
	 * If item isn't found -1 is returned instead.
	 *
	 * #case-insensitive
	 */ \
	cels_warn_unused \
	ssize_t name##s_search(const name *self, type item); \
	\
	/*
	 * Shifts whole vector to the side, 
	 * popping an item.
	 *
	 * #allocates
	 */ \
	void name##s_shift(name *self, size_t position, size_t amount, notused const allocator *mem); \
	\
	/*
	 * Unites 'other' to end of 'self' 
	 * and frees it.
	 *
	 * #allocates
	 */ \
	error name##s_unite(name *self, own name* other, const allocator *mem); \
	\
	/*
	 * Applies callback to all items.
	 */ \
	void name##s_range(name *self, shoutfunc callback, void *args);

/*
 * Helps the generation of an operation over 
 * a vector data-structure. 
 */
#define vectors_generate_arithmetic_operation(name, operation, operator) \
	error name##s_##operation(const name *self, const name *other, name *result) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((void *)self)); \
			errors_abort("other", vectors_check((void *)other)); \
			errors_abort("result", vectors_check((void *)result)); \
		} \
		\
		if (self->size != other->size) { return fail; } \
		\
		if (self->size > result->capacity) { return fail; } \
		\
		for (size_t i = 0; i < self->size; i++) { \
			typeof(result->data[i]) new_result = self->data[i] operator other->data[i]; \
			\
			if (cels_debug) { \
				errors_abort( \
					"new_result (overflow)", \
					new_result < result->data[i]); \
			} \
			\
			result->data[i] = new_result; \
		} \
		 \
		result->size = self->size; \
		\
		return ok; \
	}

/*
 * Generates arithmetic code 
 * for number-like types.
 */
#define vectors_generate_operation(name, type) \
	vectors_generate_arithmetic_operation(name, add, +) \
	\
	vectors_generate_arithmetic_operation(name, multiply, *) \
	\
	vectors_generate_arithmetic_operation(name, divide, /) \
	\
	vectors_generate_arithmetic_operation(name, subtract, -) \
	\
	error name##s_power(const name *self, const name *other, name *result) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((void *)self)); \
			errors_abort("other", vectors_check((void *)other)); \
			errors_abort("result", vectors_check((void *)result)); \
		} \
		\
		if (self->size != other->size) { return fail; } \
		\
		if (self->size > result->capacity) { return fail; } \
		\
		for (size_t i = 0; i < self->size; i++) { \
			typeof(result->data[i]) new_result = pow(self->data[i], other->data[i]); \
			\
			if (cels_debug) { \
				errors_abort( \
					"new_result (overflow)", \
					new_result < result->data[i]); \
			} \
			\
			result->data[i] = new_result; \
		} \
		 \
		result->size = self->size; \
		\
		return ok; \
	} \
	\
	type name##s_summation(name *self) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((void *)self)); \
		} \
		\
		type result = 0; \
		for (size_t i = 0; i < self->size; i++) { \
			type new_result = result + self->data[i]; \
			if (cels_debug) { \
				errors_abort( \
					"new_result (overflow)", \
					new_result < result); \
			} \
			\
			result = new_result; \
		} \
		\
		return result; \
	} \
	\
	type name##s_product(name *self) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((void *)self)); \
		} \
		\
		type result = 0; \
		for (size_t i = 0; i < self->size; i++) { \
			type new_result = result * self->data[i]; \
			\
			if (cels_debug) { \
				errors_abort( \
					"new_result (overflow)", \
					new_result < result); \
			} \
			\
			result = new_result; \
		} \
		\
		return result; \
	} \
	\
	type name##s_mean(name *self) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((void *)self)); \
		} \
		\
		if (self->size == 0) { return 0; } \
		return name##s_product(self)/self->size; \
	} \
	\
	type name##s_dot(const name *self, const name *other, name *by_product) { \
		if (cels_debug) { \
			errors_abort("self", vectors_check((void *)self)); \
			errors_abort("other", vectors_check((void *)other)); \
			errors_abort("by_product", vectors_check((void *)by_product)); \
		} \
		\
		error multiply_error = name##s_multiply(self, other, by_product); \
		if (multiply_error) { return 0; } \
		\
		return name##s_summation(by_product); \
	}

/*
 * Generates arithmetic definitions 
 * for number-like types.
 */
#define vectors_define_operation(name, type) \
	/*
	 * Adds element of 'self' with its 
	 * pair of 'other' and puts it 
	 * in 'result'.
	 *
	 * #may-error
	 */ \
	error name##s_##add(const name *self, const name *other, name *result); \
	\
	/*
	 * Multiplies element of 'self' with its 
	 * pair of 'other' and puts it 
	 * in 'result'.
	 *
	 * #may-error
	 */ \
	error name##s_##multiply(const name *self, const name *other, name *result); \
	\
	/*
	 * Divides element of 'self' with its 
	 * pair of 'other' and puts it 
	 * in 'result'.
	 *
	 * #may-error
	 */ \
	error name##s_##divide(const name *self, const name *other, name *result); \
	\
	/*
	 * Substract element of 'self' with its 
	 * pair of 'other' and puts it 
	 * in 'result'.
	 *
	 * #may-error
	 */ \
	error name##s_##subtract(const name *self, const name *other, name *result); \
	/*
	 * Powers element of 'self' with its 
	 * pair of 'other' and puts it 
	 * in 'result'.
	 *
	 * #may-error
	 */ \
	error name##s_power(const name *self, const name *other, name *result); \
	\
	/*
	 * Makes the summation of all elements 
	 * of 'self' and returns it.
	 */ \
	type name##s_summation(name *self); \
	\
	/*
	 * Makes the product of all elements 
	 * of 'self' and returns it.
	 */ \
	type name##s_product(name *self);  \
	\
	/*
	 * Takes the average of 'self' 
	 * and returns it.
	 */ \
	type name##s_mean(name *self); \
	\
	/*
	 * Makes the dot product of 'self' and 'other'; 
	 * releasing a by_product which is a vector with 
	 * the results of the multiplication.
	 */ \
	type name##s_dot(const name *self, const name *other, name *by_product);

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
void vectors_debug(const vector *self);

/*
 * Initializes a vector with capacity.
 *
 * An uninitialized vector-like 'self' must be provided, 
 * as well as it's underlying type as 'type-size'.
 *
 * #to-review
 */
error vectors_init(void *self, size_t capacity, size_t type_size, const allocator *mem);

/*
 * Upscales a vector.
 *
 * A vector-like 'self' must be provided, 
 * as well as it's underlying type as 'type-size'.
 *
 * #to-review
 */
error vectors_upscale(void *self, size_t type_size, const allocator *mem);

/*
 * Downscale a vector.
 *
 * A vector-like 'self' must be provided, 
 * as well as it's underlying type as 'type-size'.
 *
 * #to-review
 */
error vectors_downscale(void *self, size_t type_size, const allocator *mem);

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
error vectors_pop(void *self, size_t type_size, freefunc cleaner, const allocator *mem);

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
error vectors_push(void *self, void *item, size_t type_size, const allocator *mem);

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
void vectors_free(void *self, size_t type_size, freefunc cleaner, const allocator *mem);

/*
 * Downscales a vector to fit in optimum space.
 *
 * A vector-like 'self' must be provided, 
 * as well as it's underlying type as 'type-size'.
 *
 * #to-review
 */
error vectors_fit(void *self, size_t type_size, const allocator *mem);

/* definitions */

vectors_define(size_vec, size_t)
vectors_define_operation(size_vec, size_t)

vectors_define(double_vec, double)
vectors_define_operation(double_vec, double)

#endif
