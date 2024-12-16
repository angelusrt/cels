#include "mems.h"
#include "utils.h"

typedef struct fat_pointer {
	void *position;
	size_t size;
} fat_pointer;

typedef struct arena {
	size_t size;
	size_t capacity;
	fat_pointer hole;
	void *data;
	struct arena *next;
} arena;

//

void *arenas_init_private(size_t capacity) {
	arena a = { .capacity=capacity, .data=malloc(capacity), .next=null };
	errors_panic("arenas_init_private.a.data", a.data == null);

	arena *a_capsule = malloc(sizeof(arena));
	errors_panic("arenas_init_private.a_capsule", a_capsule == null);
	*a_capsule = a;

	return a_capsule;
}

void *arenas_allocate_private(arena *self, size_t size) {
	//TODO: checks

	if (size == 0) { return null; }

	arena *next = self;
	size_t standard_capacity = self->capacity;

	while ((next)) {
		if (next->hole.position && next->hole.size >= size) {
			void *old_pos = next->hole.position;

			next->hole.position = (char *)next->hole.position + size;
			next->hole.size -= size;

			if (next->hole.size == 0) {
				next->hole.position = null;
			}

			return old_pos;
		}

		size_t space_left = next->capacity - next->size;
		if (space_left >= size) {
			void *old_pos = (char *)next->data + next->size;

			next->size += size;
			return old_pos;
		}
	}

	if (size > standard_capacity) {
		size_t new_capacity = maths_nearest_two_power(size);
		next = arenas_init_private(new_capacity);
	} else {
		next = arenas_init_private(standard_capacity);
	}

	next->size += size;
	return self->next->data;
}

bool arenas_deallocate_private(arena *self, void *block, size_t block_size) {
	if (!block) {
		return true;
	}


	bool found_block = false;
	arena *blockin = self;
	while ((blockin)) {
		if (block >= self->data && block < (void *)((char *)self->data + self->size)) {
			found_block = true;
			break;
		} 

		blockin = blockin->next;
	}

	if (!found_block) {
		return true;
	}


	bool is_last = (char *)block + block_size == (char *)blockin->data + blockin->size;

	if (is_last && blockin->size >= block_size) {
		blockin->size -= block_size;
		return false;
	} else if (!is_last) {
		bool equals_right = (void *)((char *)blockin->hole.position + blockin->hole.size) == block;
		bool equals_left = (void *)((char *)block + block_size) == blockin->hole.position;

		bool may_expand_hole_left = blockin->hole.position && equals_left;
		bool may_expand_hole_right = blockin->hole.position && equals_right;
		if (may_expand_hole_left) {
			blockin->hole.position = block;
			blockin->hole.size += block_size;
		} else if (may_expand_hole_right) {
			blockin->hole.size += block_size;
		}

		bool may_create_hole = !blockin->hole.position || blockin->hole.size < block_size;
		if (may_create_hole) {
			blockin->hole.position = block;
			blockin->hole.size = block_size;
		}

		if (may_expand_hole_right || may_expand_hole_left || may_create_hole) {
			#if cels_debug
				memset(block, 0, block_size);
			#endif
			return false;
		}
	}

	return true;
}

void *arenas_reallocate_private(arena *self, void *block, size_t prev_block_size, size_t new_block_size) {
	if (!block) {
		return arenas_allocate_private(self, new_block_size);
	}

	bool found_block = false;
	arena *blockin = self;
	while ((blockin)) {
		if (block > self->data && block < (void *)((char *)self->data + self->size)) {
			found_block = true;
			break;
		} 

		blockin = blockin->next;
	}

	if (!found_block) {
		return null;
	}

	long rest_block_size = new_block_size - prev_block_size;
	size_t rest = blockin->capacity - blockin->size;

	bool is_last = (char *)block + prev_block_size == (char *)blockin->data + blockin->size;
	bool may_fit_rest = (long)rest >= rest_block_size;

	if (is_last && may_fit_rest) {
		blockin->size += rest_block_size;
		return block;
	} else if (is_last && !may_fit_rest) {
		blockin->size -= prev_block_size;
	} else if (!is_last){
		arenas_deallocate_private(blockin, block, prev_block_size);


		bool may_fit_all = rest >= new_block_size;
		if (may_fit_all) {
			void *new_data = (char *)blockin->data + blockin->size;
			blockin->size += new_block_size;
			memcpy(new_data, block, prev_block_size);

			return new_data;
		}
	} 

	arena *new_block = blockin;
	while ((new_block)) {
		bool may_fit_all = (new_block->capacity - new_block->size) >= new_block_size;

		if (may_fit_all) {
			break;
		}
	}

	if (new_block) {
		void *pos = (char *)new_block->data + new_block->size;
		memcpy(pos, block, prev_block_size);
		new_block->size += new_block_size;

		return pos;
	}

	size_t standard_capacity = blockin->capacity;

	if (new_block_size <= standard_capacity) {
		new_block = arenas_init_private(standard_capacity);
	} else {
		size_t new_capacity = maths_nearest_two_power(new_block_size);
		new_block = arenas_init_private(new_capacity);
	}

	new_block->size += new_block_size;
	return new_block->data;
}

void arenas_debug_private(arena *self) {
	arena *next = self;

	while ((next)) {
		char *data = next->data;

		for (size_t i = 0; i < next->size; i++) {
			printf("%d ", data[i]);
		}
		printf("\n");

		next = next->next;
	}

	printf("\n");
}

void arenas_free_private(arena *self) {
	arena *next = self->next;
	
	free(self->data);

	while ((next)) {
		free(next->data);
		arena *n = next->next;
		free(next);
		next = n;
	}

	free(self);
}

allocator arenas_init(size_t capacity) {
	arena *new_arena = arenas_init_private(capacity);

	return (allocator) {
		.type=allocators_group_type,
		.storage=new_arena,
		.alloc=(allocfunc)arenas_allocate_private,
		.dealloc=(deallocfunc)arenas_deallocate_private,
		.realloc=(reallocfunc)arenas_reallocate_private,
		.free=(cleanfunc)arenas_free_private,
		.debug=(debugfunc)arenas_debug_private
	};
}

//

typedef struct stack_arena {
	size_t size;
	size_t capacity;
	fat_pointer hole;
	void *data;
} stack_arena;

void *stack_arenas_allocate_private(stack_arena *self, size_t size) {
	if (size > self->capacity) { 
		#if cels_debug
			errors_note(
				"stack_arenas_allocate_private.(size > self->capacity) "
				"(%zu more than total - %zu)", 
				errors_error_mode, 
				self->capacity - size, 
				self->capacity);
		#endif

		return null; 
	}

	size_t fit_in_hole = self->hole.position || self->hole.size >= size;
	if (fit_in_hole) {
		void *old_pos = self->hole.position;

		self->hole.position = (char *)old_pos + size;
		self->hole.size -= size;

		if (self->hole.size == 0) {
			self->hole.position = null;	
		}

		return old_pos;
	}

	size_t space_left = self->capacity - self->size;
	if (size <= space_left) {
		void *old_pos = (char *)self->data + self->size;

		self->size += size;
		return old_pos;
	} 

	#if cels_debug
		errors_note(
			"stack_arenas_allocate_private.(size > self->capacity - self->size) "
			"(%zu more than available space)", 
			errors_error_mode, 
			size - (self->capacity - self->size));
	#endif

	return null;
}

bool stack_arenas_deallocate_private(stack_arena *self, void *block, size_t block_size) {
	if (!block) { return true; }

	bool is_out = block < self->data || block >= (void *)((char *)self->data + self->size);
	if (is_out) { return true; } 

	bool is_last = (char *)block + block_size == (char *)self->data + self->size;

	if (is_last && self->size >= block_size) {
		self->size -= block_size;
		return false;
	} else if (!is_last) {
		bool equals_right = (void *)((char *)self->hole.position + self->hole.size) == block;
		bool equals_left = (void *)((char *)block + block_size) == self->hole.position;

		bool may_expand_hole_left = self->hole.position && equals_left;
		bool may_expand_hole_right = self->hole.position && equals_right;
		if (may_expand_hole_left) {
			self->hole.position = block;
			self->hole.size += block_size;
		} else if (may_expand_hole_right) {
			self->hole.size += block_size;
		}

		bool may_create_hole = !self->hole.position || block_size > self->hole.size;
		if (may_create_hole) {
			self->hole.position = block;
			self->hole.size = block_size;
		}

		if (may_expand_hole_right || may_expand_hole_left || may_create_hole) {
			#if cels_debug
				memset(block, 0, block_size);
			#endif
			return false;
		}
	}

	return true;
}

void *stack_arenas_reallocate_private(stack_arena *self, void *block, size_t prev_block_size, size_t new_block_size) {
	if (block == null) {
		return stack_arenas_allocate_private(self, new_block_size);
	}

	bool is_out = self->data >= block || block >= (void *)((char *)self->data + self->size);
	if (is_out) { return null; }

	size_t rest = self->capacity - self->size;
	long rest_block_size = new_block_size - prev_block_size;

	bool is_last = (char *)self->data + self->size == (char *)block + prev_block_size;
	bool may_fit_rest = (long)rest >= rest_block_size;
	bool may_fit_all = rest >= new_block_size;

	if (is_last && may_fit_rest) {
		self->size += rest_block_size;
		return block;
	} else if (is_last && !may_fit_rest) {
		return null;
	} else if (!is_last) {
		stack_arenas_deallocate_private(self, block, prev_block_size);

		if (may_fit_all) {
			void *new_data = (char *)self->data + self->size;
			self->size += new_block_size;
			memcpy(new_data, block, prev_block_size);

			return new_data;
		}
	} 

	return null;
}

void stack_arenas_debug_private(stack_arena *self) {
	char *data = self->data;

	for (size_t i = 0; i < self->size; i++) {
		printf("%d ", data[i]);
	}

	printf("\n");
}

void stack_arenas_free_private(unused stack_arena *self) {
	free(self);
}

allocator stack_arenas_make(size_t capacity, char *buffer) {
	stack_arena new_arena = (stack_arena){.capacity=capacity, .data=buffer};
	stack_arena *capsule = malloc(sizeof(stack_arena));
	errors_panic("stack_arenas_make.capsule", capsule == null);
	*capsule = new_arena;

	return (allocator) {
		.type=allocators_group_type,
		.storage=capsule,
		.alloc=(allocfunc)stack_arenas_allocate_private,
		.dealloc=(deallocfunc)stack_arenas_deallocate_private,
		.realloc=(reallocfunc)stack_arenas_reallocate_private,
		.free=(cleanfunc)stack_arenas_free_private,
		.debug=(debugfunc)stack_arenas_debug_private
	};
}

//

void *allocs_allocate_private(size_t size) {
	return malloc(size);
}

void *allocs_reallocate_private(void *data, size_t size) {
	return realloc(data, size);
}

void allocs_free_private(void *data) {
	free(data);
}

simple_allocator allocs_init(void) {
	return (simple_allocator) {
		.type=allocators_individual_type,
		.alloc=(mallocfunc)allocs_allocate_private,
		.realloc=(allocfunc)allocs_reallocate_private,
		.free=(cleanfunc)allocs_free_private,
	};
}

//

void *mems_alloc(const allocator *mem, size_t len) {
	if (!mem) { return malloc(len); } 

	if (mem->type == allocators_individual_type) {
		return ((simple_allocator *)mem)->alloc(len);
	} else if (mem->type == allocators_group_type) {
		return mem->alloc(mem->storage, len);
	}

	return NULL;
}

void *mems_realloc(const allocator *mem, void *data, size_t old_size, size_t new_size) {
	if (!mem) { return realloc(data, new_size); } 

	if (mem->type == allocators_individual_type) {
		return ((simple_allocator *)mem)->realloc(data, new_size);
	} else if (mem->type == allocators_group_type) {
		return mem->realloc(mem->storage, data, old_size, new_size);
	}

	return NULL;
}

bool mems_dealloc(const allocator *mem, void *data, size_t block_size) {
	if (!mem) { 
		free(data); 
		return true;
	} 

	if (mem->type == allocators_individual_type) {
		((simple_allocator *)mem)->free(data);
		return true;
	} else if (mem->type == allocators_group_type) {
		return mem->dealloc(mem->storage, data, block_size);
	}

	return false;
}

void mems_free(const allocator *mem, void *data) {
	if (!mem) { free(data); } 

	if (mem->type == allocators_individual_type) {
		((simple_allocator *)mem)->free(data);
	} else if (mem->type == allocators_group_type) {
		mem->free(mem->storage);
	}
}
