#include "vectors.h"


/* vectors */

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

void vectors_debug(const void *self, printfunc printer) { 
	const vector *s = self;

	#if cels_debug
		errors_abort("self", vectors_check(s)); 
	#endif
	
	printf( 
		"<vector>{.size: %zu, .capacity: %zu, .data: %p{",  
		s->size, 
		s->capacity,
		s->data); 
	
	if (printer) {
		for (size_t i = 0; i < s->size; i++) { 
			void *item = (char *)s->data + (i * s->type_size);
			printf("\""); 
			printer(item); 
			printf("\""); 
			
			if (i != s->size - 1) { 
				printf(", "); 
			} 
		} 
	}
	
	printf("}}"); 
} 

error vectors_init(
	void *self, size_t type_size, size_t capacity, const allocator *mem) { 

	vector *s = self;

	s->size = 0;
	s->capacity = capacity;
	s->data = mems_alloc(mem, type_size * capacity);
	s->type_size = type_size;

	#if cels_debug
		errors_abort("self.data", !s->data); 
	#else
		if (!s->data) { return fail; }
	#endif
	
	return ok; 
} 

error vectors_upscale(void *self, const allocator *mem) { 
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
			s->capacity * s->type_size > new_capacity * s->type_size); 
	#endif 
	
	void *new_data = mems_realloc( 
		mem, 
		s->data, 
		s->capacity * s->type_size, 
		new_capacity * s->type_size); 
	
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

error vectors_downscale(void *self, const allocator *mem) { 
	vector *s = self;

	#if cels_debug
		errors_abort("self", vectors_check(s)); 
	#endif
	
	if (s->size < s->capacity >> 1) { 
		size_t new_capacity = s->capacity >> 1; 
		void *new_data = mems_realloc( 
			mem, 
			s->data, 
			s->capacity * s->type_size, 
			new_capacity * s->type_size); 
		
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

error vectors_pop(void *self, freefunc cleaner, const allocator *mem) { 
	vector *s = self;

	#if cels_debug
		errors_abort("self", vectors_check(s)); 
	#endif
	
	bool is_oversized = false; 
	if (s->size < s->capacity >> 1) { 
		is_oversized = true; 
	} 
	
	void *item = (char *)s->data + ((s->size - 1) * s->type_size);
	if (cleaner) {
		cleaner(item, mem); 
	}

	#if cels_debug
		memset(item, 0, s->type_size);
	#endif

	s->size--; 
	error downscale_error = ok; 
	
	if (!is_oversized && s->size < s->capacity >> 1) { 
		downscale_error = vectors_downscale(s, mem); 
	} 
	
	return downscale_error; 
} 

error vectors_push(void *self, void *item, const allocator *mem) { 
	vector *s = self;

	#if cels_debug
		errors_abort("self", vectors_check(s)); 
		errors_abort("item", !item); 
	#endif
	
	s->size++; 

	void *item_location = (char *)s->data + ((s->size - 1) * s->type_size);
	memcpy(item_location, item, s->type_size);

	error upscale_error = ok; 
	if (s->size >= s->capacity) { 
		upscale_error = vectors_upscale(s, mem); 
	} 
	
	return upscale_error; 
} 

void vectors_free(void *self, freefunc cleaner, const allocator *mem) { 
	vector *s = self;

	#if cels_debug
		errors_abort("self", vectors_check(s)); 
	#endif
	
	if (!s->data) { 
		if (cleaner) {
			for (size_t i = 0; i < s->size; i++) { 
				void *item = (char *)s->data + ((s->size + i) * s->type_size);
				cleaner(item, mem); 
			} 
		}

		mems_dealloc(mem, s->data, s->capacity * s->type_size); 
	} 
} 

error vectors_fit(void *self, const allocator *mem) { 
	vector *s = self;

	#if cels_debug
		errors_abort("self", vectors_check(s)); 
	#endif
	
	while (true) { 
		if (s->size < s->capacity >> 1 && s->capacity > vector_min) { 
			error downscale_error = vectors_downscale(s, mem); 
			if (downscale_error) { 
				return downscale_error; 
			} 
		} else { 
			break; 
		} 
	} 
	
	return ok; 
}

void *vectors_get(const void *self, ssize_t position) { 
	const vector *s = self;

	#if cels_debug
		errors_abort("self", vectors_check(s));
	#endif
	
	if (s->size == 0) {
		return (char *)s->data;
	} else if (position > (ssize_t)s->size - 1) {
		return (char *)s->data + ((s->size - 1) * s->type_size);
	} else if (position < 0 && ((ssize_t)s->size - position) >= 0) {
		return (char *)s->data + (((ssize_t)s->size - position) * s->type_size);
	} 
	
	return (char *)s->data + (position * s->type_size);
} 

void vectors_sort(void *self, void *temp, compfunc compare) {
	vector *s = self;

	#if cels_debug
		errors_abort("self", vectors_check(s)); 
	#endif
	
	for (size_t i = s->size; i > 0; i--) { 
		for (size_t j = 1; j < i; j++) { 
			void *left = ((char *)s->data) + ((j - 1) * s->type_size);
			void *right = ((char *)s->data) + ((j) * s->type_size);

			bool is_bigger = compare(left, right); 
			if (is_bigger) { 
				memcpy(temp, left, s->type_size);
				memcpy(left, right, s->type_size);
				memcpy(right, temp, s->type_size);
			} 
		} 
	} 
} 

bool vectors_match(const void *self, const void *other, compfunc comparer) { 
	const vector *s = self;
	const vector *o = other;

	#if cels_debug
		errors_abort("self", vectors_check(s)); 
		errors_abort("other", vectors_check(o));
	#endif
	
	if (s->size != o->size) { 
		return false; 
	} 
	
	for (size_t i = 0; i < s->size; i++) { 
		void *s_item = (char *)s->data + (i * s->type_size);
		void *o_item = (char *)o->data + (i * s->type_size);

		if (!comparer(s_item, o_item)) { 
			return false; 
		} 
	} 
	
	return true; 
} 

error vectors_filter(
	void *self, filterfunc filter, freefunc cleaner, const allocator *mem) { 

	vector *s = self;

	#if cels_debug
		errors_abort("self", vectors_check(s)); 
	#endif
	
	vector other = {0};
	vectors_init(&other, s->type_size, vector_min, mem); 
	
	for (size_t i = 0; i < s->size; i++) { 
		void *item = ((char *)s->data) + (i * s->type_size);
		if (filter(item)) { 
			error push_error = vectors_push(&other, item, mem); 
			if (push_error) { 
				vectors_free(&other, cleaner, mem);
				for (size_t j = i; j < s->size; j++) {
					void *item = ((char *)s->data) + (j * s->type_size);
					cleaner(item, mem);
				}

				error dealloc_error = mems_dealloc(mem, s->data, s->capacity); 
				if (dealloc_error) { return fail; } 

				return fail; 
			}
		} else { 
			if (cleaner) {
				cleaner(item, mem);
			}
		} 
	} 

	error dealloc_error = mems_dealloc(mem, s->data, s->capacity); 
	if (dealloc_error) { return fail; } 
	
	*s = other; 
	return ok; 
} 

error vectors_filter_unique(void *self, compfunc comparer, freefunc cleaner, const allocator *mem) {
	vector *s = self;

	#if cels_debug
		errors_abort("self", vectors_check((void *)self));
	#endif
	
	vector other = {0};
	vectors_init(&other, s->type_size, vector_min, mem);
	
	for (size_t i = 0; i < s->size; i++) {
		void *s_item = (char *)s->data + (i * s->type_size);

		bool match = false;
		for (size_t j = 0; j < other.size; j++) {
			void *o_item = (char *)other.data + (j * s->type_size);
			if (comparer(s_item, o_item)) {
				match = true;
				break;
			}
		}
		
		if (!match) {
			error push_error = vectors_push(&other, s_item, mem);
			if (push_error) { return fail; }
		} else {
			cleaner(s_item, mem);
		}
	}
	
	error dealloc_error = mems_dealloc(mem, s->data, s->capacity * s->type_size);
	if (dealloc_error) { return fail; }
	
	*s = other;
	
	#if cels_debug
		errors_abort("s", vectors_check((const vector *)s));
	#endif
	
	return ok;
}

void vectors_shift(
	void *self, 
	size_t position, 
	size_t amount, 
	freefunc cleaner, 
	notused const allocator *mem) { 

	vector *s = self;

	#if cels_debug
		errors_abort("self", vectors_check(s)); 
	#endif
	
	if (position + amount >= s->size || amount == 0) { 
		return; 
	} 
	
	if (cleaner) {
		for (size_t i = position; i < amount; i++) { 
			void *item = (char *)s->data + (i * s->type_size);
			cleaner(item, mem); 
		} 
	}
	
	for (size_t i = position; i < s->size - amount; i++) { 
		void *item = (char *)s->data + (i * s->type_size);
		void *other = (char *)s->data + ((i + amount) * s->type_size);
		memcpy(item, other, s->type_size);
	} 
	
	s->size -= amount; 
	
	return; 
} 

ssize_t vectors_find(const void *self, void *item, compfunc comparer) { 
	const vector *s = self;

	#if cels_debug
		errors_abort("self", vectors_check(s)); 
		errors_abort("item", !item); 
	#endif
	
	for (size_t i = 0; i < s->size; i++) { 
		void *s_item = (char *)s->data + (i * s->type_size);
		if (comparer(s_item, item)) { 
			return i; 
		} 
	} 
	
	return -1; 
} 

error vectors_unite(
	void *self, own void* other, notused const allocator *mem) {

	vector *s = self;
	vector *o = other;

	#if cels_debug
		errors_abort("self", vectors_check(s));
		errors_abort("other", vectors_check(o));
	#endif
	
	for (size_t i = 0; i < o->size; i++) {
		void *item = (char *)o->data + (i * o->type_size);
		error push_error = vectors_push(self, item, mem);
		if (push_error) { return fail; }
	}
	
	error dealloc_error = mems_dealloc(mem, o->data, o->capacity);
	if (dealloc_error) { return fail; }
	
	o->size = 0;
	o->data = null;
	
	return ok;
}

void vectors_do(void *self, dofunc callback) {
	vector *s = self;

	#if cels_debug
		errors_abort("self", vectors_check(s));
	#endif
	
	for (size_t i = 0; i < s->size; i++) {
		void *item = (char *)s->data + (i * s->type_size);
		callback(item);
	}
}

void vectors_range(void *self, shoutfunc callback, void *args) {
	vector *s = self;

	#if cels_debug
		errors_abort("self", vectors_check(s));
	#endif
	
	for (size_t i = 0; i < s->size; i++) {
		void *item = (char *)s->data + (i * s->type_size);
		callback(item, args);
	}
}
