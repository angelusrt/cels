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

error vectors_init(void *self, size_t capacity, size_t type_size, const allocator *mem) { 
	vector *s = self;

	#if cels_debug
		errors_abort("self", vectors_check(s)); 
	#endif

	s->size = 0;
	s->capacity = capacity;
	s->data = mems_alloc(mem, type_size * capacity);

	#if cels_debug
		errors_abort("self.data", !s->data); 
	#else
		if (!s->data) { return fail; }
	#endif
	
	return ok; 
} 

error vectors_upscale(void *self, size_t type_size, const allocator *mem) { 
	vector *s = self;

	#if cels_debug
		errors_abort("self", vectors_check(s)); 
	#endif
	
	size_t new_capacity = s->capacity << 1; 
	
	#if cels_debug
		errors_abort( 
			"new_capacity (overflow)", 
			new_capacity < s->capacity); 

		errors_abort( 
			"new_capacity (overflow)", 
			s->capacity * type_size > new_capacity * type_size); 
	#endif 
	
	void *new_data = mems_realloc( 
		mem, 
		s->data, 
		s->capacity * type_size, 
		new_capacity * type_size); 
	
	#if cels_debug
		errors_inform("new_data", !new_data); 
	#endif
	
	if (!new_data) { 
		s->size--; 
		return fail; 
	} 
	
	s->capacity = new_capacity; 
	s->data = new_data; 
	
	return ok; 
} 

error vectors_downscale(void *self, size_t type_size, const allocator *mem) { 
	vector *s = self;

	#if cels_debug
		errors_abort("self", vectors_check(s)); 
	#endif
	
	if (s->size < s->capacity >> 1) { 
		size_t new_capacity = s->capacity >> 1; 
		void *new_data = mems_realloc( 
			mem, 
			s->data, 
			s->capacity * type_size, 
			new_capacity * type_size); 
		
		#if cels_debug
			errors_inform("new_data", !new_data); 
		#endif
		
		if (!new_data) { 
			s->size++; 
			return fail; 
		} 
		
		s->capacity = new_capacity; 
		s->data = new_data; 
	} 
	
	return ok; 
} 

error vectors_pop(void *self, size_t type_size, freefunc cleaner, const allocator *mem) { 
	vector *s = self;

	#if cels_debug
		errors_abort("self", vectors_check(s)); 
	#endif
	
	bool is_oversized = false; 
	if (s->size < s->capacity >> 1) { 
		is_oversized = true; 
	} 
	
	void *item = (char *)s->data + ((s->size - 1) * type_size);
	if (cleaner) {
		cleaner(item, mem); 
	}

	#if cels_debug
		memset(item, 0, type_size);
	#endif

	s->size--; 
	error downscale_error = ok; 
	
	if (!is_oversized && s->size < s->capacity >> 1) { 
		downscale_error = vectors_downscale(s, type_size, mem); 
	} 
	
	return downscale_error; 
} 

error vectors_push(void *self, void *item, size_t type_size, const allocator *mem) { 
	vector *s = self;

	#if cels_debug
		errors_abort("self", vectors_check(s)); 
		errors_abort("item", !item); 
	#endif
	
	s->size++; 

	void *item_location = (char *)s->data + ((s->size - 1) * type_size);
	memcpy(item_location, item, type_size);

	error upscale_error = ok; 
	if (s->size >= s->capacity) { 
		upscale_error = vectors_upscale(s, type_size, mem); 
	} 
	
	return upscale_error; 
} 

void vectors_free(void *self, size_t type_size, freefunc cleaner, const allocator *mem) { 
	vector *s = self;

	#if cels_debug
		errors_abort("self", vectors_check(s)); 
	#endif
	
	if (!s->data) { 
		if (cleaner) {
			for (size_t i = 0; i < s->size; i++) { 
				void *item = (char *)s->data + ((s->size + i) * type_size);
				cleaner(item, mem); 
			} 
		}

		mems_dealloc(mem, s->data, s->capacity * type_size); 
	} 
} 

error vectors_fit(void *self, size_t type_size, const allocator *mem) { 
	vector *s = self;

	#if cels_debug
		errors_abort("self", vectors_check(s)); 
	#endif
	
	while (true) { 
		if (s->size < s->capacity >> 1 && s->capacity > vector_min) { 
			error downscale_error = vectors_downscale(s, type_size, mem); 
			if (downscale_error) { 
				return downscale_error; 
			} 
		} else { 
			break; 
		} 
	} 
	
	return ok; 
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
