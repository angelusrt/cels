#include "vectors.h"

bool vectors_check(const vector *self) {
	#if cels_debug
		errors_return("self", !self)
		errors_return("self.data", !self->data)
		errors_return("self.capacity < 1", self->capacity < 1)

		bool is_bigger = self->size > self->capacity;
		errors_return("self.(size > capacity)", is_bigger)
	#else
		if (!self) return true;
		if (!self->data) return true;
		if (self->capacity < 1) return true;
		if (self->size > self->capacity) return true;
	#endif

    return false;
}

void vectors_debug(const vector *self) {
	#if cels_debug
		errors_abort("self", vectors_check(self));
	#endif

	printf(
		"<vector>{.size: %zu, .capacity: %zu, .data: %p}\n", 
		self->size, self->capacity, self->data);
}

/* implementations */

void size_print(size_t *number) {
	printf("%zu\n", *number);
}

vectors_generate_implementation(
	size_t, 
	size_vec, 
	defaults_check,
	defaults_clone,
	size_print,
	defaults_compare, 
	defaults_compare, 
	defaults_free
)
