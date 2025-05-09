#include "mems.h"


/* allocators */

bool allocators_check(const allocator *self) {
	bool is_group = self->type == allocators_group_type;

	#if cels_debug
		errors_return("self", !self)
		if (is_group) {
			errors_return("self.storage", !self->storage)
		}
	#else
		if (!self) { return true; } 
		if (is_group && !self->storage) { return true; } 
	#endif

    return false;
}


/* arenas */

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

bool arenas_check(const arena *self) {
	#if cels_debug
		errors_return("self", !self)
		errors_return("self.data", !self->data)

		bool is_bigger = self->size > self->capacity;
		errors_return("self.size > self.capacity", is_bigger)
	#else
		if (!self) return true;
		if (!self->data) return true;
		if (self->size > self->capacity) return true;
	#endif

    return false;
}

void *arenas_init_helper(size_t capacity) {
	arena self = {
		.capacity=capacity, 
		.data=malloc(capacity), 
		.next=null};
	errors_abort("arena.data", !self.data);

	arena *self_capsule = malloc(sizeof(arena));
	errors_abort("self_capsule", !self_capsule);
	*self_capsule = self;

	return self_capsule;
}

void *arenas_allocate(arena *self, size_t size) {
	#if cels_debug
		errors_abort("self", arenas_check(self));
	#endif

	if (size == 0) { return null; }

	arena *next = self;
	size_t standard_capacity = self->capacity;

	while (true) {
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

		if (!next->next) {
			break;
		}

		next = next->next;
	}

	if (size > standard_capacity) {
		size_t new_capacity = maths_nearest_two_power(size);
		next->next = arenas_init_helper(new_capacity);
	} else {
		next->next = arenas_init_helper(standard_capacity);
	}

	next->next->size += size;
	return next->next->data;
}

void arenas_debug(arena *self);

error arenas_deallocate(arena *self, void *block, size_t block_size) {
	#if cels_debug
		errors_abort("self", arenas_check(self));
		errors_abort("block", block == null);
	#endif

	bool found_block = false;
	arena *blockin = self;
	while ((blockin)) {
		bool is_within = 
			block >= self->data && 
			block < (void *)((char *)self->data + self->size);

		if (is_within) {
			found_block = true;
			break;
		} 

		blockin = blockin->next;
	}

	if (!found_block) {
		return fail;
	}

	bool is_last = 
		((char *)block + block_size) == 
		(char *)blockin->data + blockin->size;

	fat_pointer *hole = &blockin->hole;

	if (is_last && blockin->size >= block_size) {
		blockin->size -= block_size;
		return ok;
	} else if (!is_last) {
		void *right_border = (void *)((char *)hole->position + hole->size);
		bool equals_right = right_border == block;

		void *left_border = hole->position;
		bool equals_left = (void *)((char *)block + block_size) == left_border;

		bool may_expand_hole_left = hole->position && equals_left;
		bool may_expand_hole_right = hole->position && equals_right;

		if (may_expand_hole_left) {
			hole->position = block;
			hole->size += block_size;
		} else if (may_expand_hole_right) {
			hole->size += block_size;
		}

		bool may_create_hole = !hole->position || hole->size < block_size;
		if (may_create_hole) {
			hole->position = block;
			hole->size = block_size;
		}

		if (may_expand_hole_right || may_expand_hole_left || may_create_hole) {
			#if cels_debug
				memset(block, 0, block_size);
			#endif
			return ok;
		}
	}

	return ok;
}

void *arenas_reallocate(
	arena *self, void *block, size_t prev_block_size, size_t new_block_size) {

	#if cels_debug
		errors_abort("self", arenas_check(self));
		errors_abort("block", !block);
	#endif

	bool found_block = false;
	arena *blockin = self;
	while ((blockin)) {
		#if cels_debug
			errors_abort("blockin", arenas_check(blockin));
		#endif

		bool is_within = 
			block >= blockin->data && 
			(void *)((char *)block + prev_block_size) <= 
			(void *)((char *)blockin->data + blockin->capacity);

		if (is_within) {
			found_block = true;
			break;
		} 

		blockin = blockin->next;
	}

	#if cels_debug
		errors_abort("!found_block (block not within)", !found_block);
	#endif

	if (!found_block) {
		return null;
	}

	long rest_block_size = new_block_size - prev_block_size;
	size_t rest = blockin->capacity - blockin->size;

	bool is_alone = block == blockin->data && prev_block_size == blockin->size;
	bool is_last = 
		(char *)block + prev_block_size == 
		(char *)blockin->data + blockin->size;

	bool may_fit_rest = (long)rest >= rest_block_size;

	if (is_last && may_fit_rest) {
		blockin->size += rest_block_size;
		return block;
	} else if (is_alone) {
		size_t new_capacity = maths_nearest_two_power(new_block_size);
		void *new_data = realloc(blockin->data, new_capacity);

		if (!new_data) {
			return null;
		}
		
		blockin->data = new_data;
		blockin->capacity = new_capacity;
		blockin->size += rest_block_size;

		return blockin->data;
	} else if (is_last && !may_fit_rest) {
		blockin->size -= prev_block_size;
	} else if (!is_last){
		bool may_fit_all = rest >= new_block_size;
		if (may_fit_all) {
			void *new_data = (char *)blockin->data + blockin->size;
			blockin->size += new_block_size;

			memcpy(new_data, block, prev_block_size);
			arenas_deallocate(blockin, block, prev_block_size);

			return new_data;
		} 	
	} 

	bool create_new = true;
	arena *lastblock = self;
	while (true) {
		#if cels_debug
			errors_abort("lastblock", arenas_check(lastblock));
		#endif

		size_t rest = lastblock->capacity - lastblock->size;
		bool may_fit_all = rest >= new_block_size;

		if (may_fit_all) {
			create_new = false;
			break;
		}

		if (!lastblock->next) {
			break;
		} 

		lastblock = lastblock->next;
	}

	if (!create_new) {
		void *pos = (char *)lastblock->data + lastblock->size;

		memcpy(pos, block, prev_block_size + 1);
		arenas_deallocate(blockin, block, prev_block_size);
		lastblock->size += new_block_size;

		return pos;
	}

	size_t standard_capacity = blockin->capacity;

	if (new_block_size <= standard_capacity) {
		lastblock->next = arenas_init_helper(standard_capacity);
	} else {
		size_t new_capacity = maths_nearest_two_power(new_block_size);
		lastblock->next = arenas_init_helper(new_capacity);
	}

	if (lastblock->next == null) {
		return null;
	}

	memcpy(lastblock->next->data, block, prev_block_size);
	arenas_deallocate(blockin, block, prev_block_size);

	lastblock->next->size += new_block_size;
	return lastblock->next->data;
}

void arenas_debug(arena *self) {
	#if cels_debug
		errors_abort("self", arenas_check(self));
	#endif

	arena *next = self;

	while ((next)) {
		#if cels_debug
			errors_abort("next", arenas_check(next));
		#endif

		char *data = next->data;

		for (size_t i = 0; i < next->capacity; i++) {
			if (data[i] >= 33 && data[i] <= 126) {
				printf("%c ", (unsigned char)data[i]);
			} else {
				printf("%d ", (unsigned char)data[i]);
			}
		}
		printf("\n\n");

		next = next->next;
	}

	printf("\n");
}

void arenas_free(arena *self) {
	#if cels_debug
		errors_abort("self", arenas_check(self));
	#endif

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
	arena *new_arena = arenas_init_helper(capacity);

	return (allocator) {
		.type=allocators_group_type,
		.storage=new_arena,
		.alloc=(allocfunc)arenas_allocate,
		.dealloc=(deallocfunc)arenas_deallocate,
		.realloc=(reallocfunc)arenas_reallocate,
		.free=(cleanfunc)arenas_free,
		.debug=(debugfunc)arenas_debug
	};
}


/* stack_arenas */

typedef struct stack_arena {
	size_t size;
	size_t capacity;
	fat_pointer hole;
	void *data;
} stack_arena;

/* TODO stack_arena_checks */

void *stack_arenas_allocate(stack_arena *self, size_t size) {
	if (size > self->capacity) { 
		#if cels_debug
			errors_print(
				errors_error_mode, 
				"stack_arenas_allocate.(size > self->capacity) "
				"(%zu more than total - %zu)", 
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
		errors_print(
			errors_error_mode, 
			"stack_arenas_allocate.(size > self->capacity - self->size) "
			"(%zu more than available space)", 
			size - (self->capacity - self->size));
	#endif

	return null;
}

error stack_arenas_deallocate(
	stack_arena *self, void *block, size_t block_size) {

	if (!block) { 
		return fail; 
	}

	bool is_out = 
		block < self->data || block >= 
		(void *)((char *)self->data + self->size);

	if (is_out) { 
		return fail; 
	} 

	bool is_last = 
		(char *)block + block_size == 
		(char *)self->data + self->size;

	fat_pointer *hole = &self->hole;

	if (is_last && self->size >= block_size) {
		self->size -= block_size;
		return ok;
	} else if (!is_last) {
		bool equals_right = 
			(void *)((char *)hole->position + hole->size) == block;

		bool equals_left = 
			(void *)((char *)block + block_size) == hole->position;

		bool may_expand_hole_left = hole->position && equals_left;
		bool may_expand_hole_right = hole->position && equals_right;

		if (may_expand_hole_left) {
			hole->position = block;
			hole->size += block_size;
		} else if (may_expand_hole_right) {
			hole->size += block_size;
		}

		bool may_create_hole = !hole->position || block_size > hole->size;
		if (may_create_hole) {
			hole->position = block;
			hole->size = block_size;
		}

		if (may_expand_hole_right || may_expand_hole_left || may_create_hole) {
			#if cels_debug
				memset(block, 0, block_size);
			#endif
			return ok;
		}
	}

	return fail;
}

void *stack_arenas_reallocate(
	stack_arena *self, void *block, size_t prev_size, size_t new_size) {

	if (!block) {
		return stack_arenas_allocate(self, new_size);
	}

	bool is_out = 
		self->data >= block || 
		block >= (void *)((char *)self->data + self->size);

	if (is_out) { 
		return null; 
	}

	size_t rest = self->capacity - self->size;
	long rest_block_size = new_size - prev_size;

	bool is_last = (char *)self->data + self->size == (char *)block + prev_size;
	bool may_fit_rest = (long)rest >= rest_block_size;
	bool may_fit_all = rest >= new_size;

	if (is_last && may_fit_rest) {
		self->size += rest_block_size;
		return block;
	} else if (is_last && !may_fit_rest) {
		return null;
	} else if (!is_last) {
		stack_arenas_deallocate(self, block, prev_size);

		if (may_fit_all) {
			void *new_data = (char *)self->data + self->size;
			self->size += new_size;
			memcpy(new_data, block, prev_size);

			return new_data;
		}
	} 

	return null;
}

void stack_arenas_debug(stack_arena *self) {
	char *data = self->data;

	for (size_t i = 0; i < self->size; i++) {
		if (data[i] >= 33 && data[i] <= 126) {
			printf("%c ", (unsigned char)data[i]);
		} else {
			switch(data[i]) {
				case '\n': printf("\\n"); break;
				case '\t': printf("\\t"); break;
				case '\r': printf("\\r"); break;
				case '\b': printf("\\b"); break;
				case '\a': printf("\\a"); break;
				case '\v': printf("\\v"); break;
				case '\f': printf("\\f"); break;
				case '\\': printf("\\\\"); break;
				case '\'': printf("\\'"); break;
				case '\"': printf("\\\""); break;
				default: printf("\\x%02X", data[i]); break;
			}

			printf(" ");
		}
	}

	printf("\n");
}

void stack_arenas_free(stack_arena *self) {
	free(self);
}

allocator stack_arenas_init_helper(size_t capacity, char *buffer) {
	stack_arena new_arena = (stack_arena){.capacity=capacity, .data=buffer};
	stack_arena *capsule = malloc(sizeof(stack_arena));
	errors_abort("capsule", !capsule);
	*capsule = new_arena;

	return (allocator) {
		.type=allocators_group_type,
		.storage=capsule,
		.alloc=(allocfunc)stack_arenas_allocate,
		.dealloc=(deallocfunc)stack_arenas_deallocate,
		.realloc=(reallocfunc)stack_arenas_reallocate,
		.free=(cleanfunc)stack_arenas_free,
		.debug=(debugfunc)stack_arenas_debug
	};
}


/* allocs */

void *allocs_allocate(notused void *storage, size_t size) {
	return malloc(size);
}

void *allocs_reallocate(
	notused void *storage, void *data, notused size_t prev, size_t size) {
	return realloc(data, size);
}

void allocs_free(void *data) {
	free(data);
}

allocator allocs_init(void) {
	static allocator alloc = {
		.type=allocators_individual_type,
		.alloc=(allocfunc)allocs_allocate,
		.realloc=(reallocfunc)allocs_reallocate,
		.free=(cleanfunc)allocs_free,
	};

	return alloc;
}


/* mems */

void *mems_alloc(const allocator *mem, size_t len) {
	void *data = null;
	if (!mem) { 
		data = malloc(len);
	} else if (mem->type == allocators_individual_type) {
		data = mem->alloc(null, len);
	} else if (mem->type == allocators_group_type) {
		data = mem->alloc(mem->storage, len);
	}

	if (data) {
		return data;
	}

	#if cels_debug
		errors_abort("data", true);
	#endif

	return null;
}

void *mems_realloc(
	const allocator *mem, void *data, size_t old_size, size_t new_size) {

	void *new_data = null;
	if (!mem) { 
		new_data = realloc(data, new_size); 
	} else if (mem->type == allocators_individual_type) {
		new_data =  mem->realloc(null, data, 0, new_size);
	} else if (mem->type == allocators_group_type) {
		new_data =  mem->realloc(mem->storage, data, old_size, new_size);
	}

	if (new_data) {
		return new_data;
	}

	#if cels_debug
		errors_abort("new_data", true);
	#endif

	return NULL;
}

error mems_dealloc(const allocator *mem, void *data, size_t block_size) {
	#if cels_debug
		errors_abort("data", !data);
	#endif

	if (!data) {
		return fail;
	}

	#if cels_debug
		memset(data, 0, block_size);
	#endif

	if (!mem) { 
		free(data); 
		return ok;
	} else if (mem->type == allocators_individual_type) {
		mem->free(data);
		return ok;
	} else if (mem->type == allocators_group_type) {
		return mem->dealloc(mem->storage, data, block_size);
	}

	return fail;
}

void mems_free(const allocator *mem, void *data) {
	if (!mem) { 
		if (!data) { return; }

		free(data); 
	} else if (mem->type == allocators_individual_type) {
		if (!data) { return; }

		mem->free(data);
	} else if (mem->type == allocators_group_type) {
		mem->free(mem->storage);
	}
}
