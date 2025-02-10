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

priv void sizes_print(size_t *number) {
	#if cels_debug
		errors_abort("number", !number);
	#endif

	printf("%zu\n", *number);
}

vectors_generate(
	size_vec, 
	size_t, 
	defaults_check,
	defaults_clone,
	sizes_print,
	sizes_print,
	defaults_compare, 
	defaults_compare, 
	defaults_free
)

vectors_generate_operation(size_vec, size_t)

void doubles_print_private(double *number) {
	#if cels_debug
		errors_abort("number", !number);
	#endif

	printf("%lf", *number);
}

vectors_generate(
	double_vec, 
	double, 
	defaults_check,
	defaults_clone,
	doubles_print_private,
	doubles_print_private,
	defaults_compare, 
	defaults_compare, 
	defaults_free
)

vectors_generate_operation(double_vec, double)
