#include "vectors.h"

bool vectors_check(const vector *v) {
	#if cels_debug
		if (errors_check("vectors_check.v", v == NULL)) return true;
		if (errors_check("vectors_check.v.data", v->data == NULL)) return true;
		if (errors_check("vectors_check.v.size > v.capacity", v->size > v->capacity)) return true;
	#else
		if (v == NULL) return true;
		if (v->data == NULL) return true;
		if (v->size > v->capacity) return true;
	#endif

    return false;
}

void vectors_upscale(vector *v, size_t type_size, bool *error) {
	#if cels_debug
		errors_panic("vectors_upscale.v", vectors_check(v));
	#endif 

	if (error != NULL && *error == true) {
		return;
	}

	v->capacity = v->capacity << 1;
	void *new_data = realloc(v->data, (v->capacity) * type_size);
	if (new_data == NULL) {
		#if cels_debug
			printf(colors_warn("vectors_upscale.new_data failed"));
		#endif

		v->size--;
		v->capacity = v->capacity >> 1;

		if (error != NULL) { *error = true; }
		return;
	} 

	v->data = new_data;

	return;
}

bool vectors_equals(const vector *v0, const vector *v1, compvecfunc compare) {
	#if cels_debug
		errors_panic("vectors_equals.v0", vectors_check((vector *)v0));
		errors_panic("vectors_equals.v1", vectors_check((vector *)v1));
	#endif

	if (v0->size != v1->size) return false;

	for (size_t i = 0; i < v0->size; i++) {
		if (!compare(v0->data, v1->data, &i)) {
			return false;
		}
	}

	return true;
}

bool vectors_sized_equals(const vector *v0, const vector *v1, size_t object_size) {
	#if cels_debug
		errors_panic("vectors_sized_equals.v0", vectors_check((vector *)v0));
		errors_panic("vectors_sized_equals.v1", vectors_check((vector *)v1));
		errors_panic("vectors_sized_equals.object_size == 0", object_size == 0);
	#endif

	if (v0->size != v1->size) {
		return false;
	}

	char *v0data = v0->data;
	char *v1data = v1->data;

	for (size_t i = 0; i < v0->size * object_size; i++) {
		if (v0data[i] != v1data[i]) {
			return false;
		}
	}

	return true;
}

ssize_t vectors_sized_find(const vector *v, void *item, size_t object_size) {
	#if cels_debug
		errors_panic("vectors_sized_find.v", vectors_check((vector *)v));
		errors_panic("vectors_sized_find.item", item == null);
		errors_panic("vectors_sized_find.object_size == 0", object_size == 0);
	#endif

	char *vdata = v->data;
	char *citem = item;
	bool matched = true;
	for (size_t i = 0, j = 0; i < v->size * object_size; i++) {
		if (vdata[i] != citem[j]) {
			matched = false;
		}

		if (j == object_size - 1) {
			j = 0;

			if (matched) {
				return i/object_size;
			}

			matched = true;
		} else {
			j++;
		}
	}

	return -1;
}
