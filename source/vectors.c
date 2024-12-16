#include "vectors.h"
#include "mems.h"

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

void vectors_upscale(vector *self, size_t object_size, const allocator *mem, bool *error) {
	#if cels_debug
		errors_panic("vectors_upscale.self", vectors_check(self));
	#endif 

	if (error != NULL && *error == true) {
		return;
	}

	size_t new_capacity = self->capacity << 1;
	void *new_data = mems_realloc(
		mem, 
		self->data, 
		(self->capacity) * object_size, 
		(new_capacity) * object_size);

	if (new_data == NULL) {
		#if cels_debug
			printf(colors_warn("vectors_upscale.new_data failed"));
		#endif

		self->size--;

		if (error != NULL) { *error = true; }
		return;
	} 

	self->capacity = new_capacity;
	self->data = new_data;

	return;
}

vectors_generate_implementation(size_t, size_vec)
