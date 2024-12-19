#include "vectors.h"

bool vectors_check(const vector *self) {
	#if cels_debug
		if (errors_check(utils_fcat(".self"), self == NULL)) return true;
		if (errors_check(utils_fcat(".self.data"), self->data == NULL)) return true;

		bool bigger = self->size > self->capacity;
		if (errors_check(utils_fcat(".self.size > self.capacity"), bigger)) return true;
	#else
		if (self == NULL) return true;
		if (self->data == NULL) return true;
		if (self->size > self->capacity) return true;
	#endif

    return false;
}

/* implementations */

void size_print_private(size_t *number) {
	printf("%zu\n", *number);
}

vectors_generate_implementation(
	size_t, 
	size_vec, 
	defaults_check,
	size_print_private,
	defaults_compare, 
	defaults_compare, 
	defaults_free
)
