#include "vectors.h"

bool vectors_check(const vector *self) {
	#if cels_debug
		if (errors_check("vectors_check.self", self == NULL)) return true;
		if (errors_check("vectors_check.self.data", self->data == NULL)) return true;
		if (errors_check("vectors_check.self.size > self.capacity", self->size > self->capacity)) return true;
	#else
		if (self == NULL) return true;
		if (self->data == NULL) return true;
		if (self->size > self->capacity) return true;
	#endif

    return false;
}

/* implementations */

vectors_generate_implementation(size_t, size_vec)
