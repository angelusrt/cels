#include "nodes.h" 


/* bynodes */

void bynodes_right_rotate_private(notused bynode *self, bynode *new_node) {
	#if cels_debug
		errors_abort("self", bynodes_check(self));
		errors_abort("new_node", bynodes_check(new_node));
	#endif

    bynode* left = new_node->left;
    new_node->left = left->right;

    if (new_node->left) {
        new_node->left->parent = new_node;
	}

    left->parent = new_node->parent;
    if (!new_node->parent) {
        self = left;
	} else if (new_node == new_node->parent->left) {
        new_node->parent->left = left;
	} else {
        new_node->parent->right = left;
	}

    left->right = new_node;
    new_node->parent = left;
}
 
void bynodes_left_rotate_private(notused bynode *self, bynode *new_node) {
	#if cels_debug
		errors_abort("self", bynodes_check(self));
		errors_abort("new_node", bynodes_check(new_node));
	#endif

    bynode* right = new_node->right;
    new_node->right = right->left;

    if (new_node->right) {
        new_node->right->parent = new_node;
	}

    right->parent = new_node->parent;
    if (!new_node->parent) {
        self = right;
	} else if (new_node == new_node->parent->left) {
        new_node->parent->left = right;
	} else {
        new_node->parent->right = right;
	}

    right->left = new_node;
    new_node->parent = right;
}

/*
 * This function normalizes the red-black tree 
 * violentions made by insertion by bynodes_push.
 *
 * I've got this from gist.github.com/VictorGarritano/5f894be162d39e9bdd5c
 */
void bynodes_normalize_private(bynode *self, bynode *new_node) {
	#if cels_debug
		errors_abort("self", bynodes_check(self));
		errors_abort("new_node", bynodes_check(new_node));
	#endif

	bool is_new_node_unique = 
		new_node != self && new_node != self->left && new_node != self->right;

	bool is_new_node_parent_red = 
		new_node->parent && new_node->parent->color == bynode_red_color;

    // iterate until new_node is not the self and new_node's parent color is red
    while (is_new_node_unique && is_new_node_parent_red) {
        bynode *y = null;

        // Find uncle and store uncle in y
		bool does_parent_exists = 
			new_node->parent && 
			new_node->parent->parent;

		bool does_parent_exists_at_left = 
			does_parent_exists && 
			new_node->parent == new_node->parent->parent->left;

        if (does_parent_exists_at_left) {
            y = new_node->parent->parent->right;
		} else if (does_parent_exists) {
            y = new_node->parent->parent->left;
		}

        // If uncle is RED, do following
        // (i)  Change color of parent and uncle as BLACK
        // (ii) Change color of grandparent as RED
        // (iii) Move new_node to grandparent
        if (!y) {
            new_node = new_node->parent->parent;
		} else if (y->color == bynode_red_color) {
            y->color = bynode_black_color;
            new_node->parent->color = bynode_black_color;
            new_node->parent->parent->color = bynode_red_color;
            new_node = new_node->parent->parent;
        } else {
            // Left-Left (LL) case, do following
            // (i)  Swap color of parent and grandparent
            // (ii) Right Rotate Grandparent
			
			bool does_parent_exists = 
				new_node->parent && 
				new_node->parent->parent;

			bool case1 = 
				does_parent_exists && 
				new_node->parent == new_node->parent->parent->left && 
				new_node == new_node->parent->left;

            if (case1) {
                bynode_color color = new_node->parent->color ;
                new_node->parent->color = new_node->parent->parent->color;
                new_node->parent->parent->color = color;
                bynodes_right_rotate_private(self,new_node->parent->parent);
            }

            // Left-Right (LR) case, do following
            // (i)  Swap color of current node  and grandparent
            // (ii) Left Rotate Parent
            // (iii) Right Rotate Grand Parent

			does_parent_exists = 
				new_node->parent && 
				new_node->parent->parent;

			bool case2 = 
				does_parent_exists && 
				new_node->parent == new_node->parent->parent->left && 
				new_node == new_node->parent->right;

            if (case2) {
                bynode_color color = new_node->color ;
                new_node->color = new_node->parent->parent->color;
                new_node->parent->parent->color = color;
                bynodes_left_rotate_private(self,new_node->parent);
                bynodes_right_rotate_private(self,new_node->parent->parent);
            }

            // Right-Right (RR) case, do following
            // (i)  Swap color of parent and grandparent
            // (ii) Left Rotate Grandparent

			does_parent_exists = 
				new_node->parent && 
				new_node->parent->parent;

			bool case3 = 
				does_parent_exists && 
				new_node->parent == new_node->parent->parent->right && 
				new_node == new_node->parent->right;

            if (case3) {
                bynode_color color = new_node->parent->color;
                new_node->parent->color = new_node->parent->parent->color;
                new_node->parent->parent->color = color;
                bynodes_left_rotate_private(self,new_node->parent->parent);
            }

            // Right-Left (RL) case, do following
            // (i)  Swap color of current node  and grandparent
            // (ii) Right Rotate Parent
            // (iii) Left Rotate Grand Parent

			does_parent_exists = 
				new_node->parent && 
				new_node->parent->parent;

			bool case4 = 
				does_parent_exists && 
				new_node->parent == new_node->parent->parent->right && 
				new_node == new_node->parent->left;

            if (case4) {
                bynode_color color = new_node->color;
                new_node->color = new_node->parent->parent->color;
                new_node->parent->parent->color = color;
                bynodes_right_rotate_private(self,new_node->parent);
                bynodes_left_rotate_private(self,new_node->parent->parent);
            }
        }
    }

    self->color = bynode_black_color; 
}

bynode *bynodes_find_left_most_private(bynode *self) {
	if (!self || !self->left) {
		return null;
	}

	bynode *left_most = self->left;
	while (true) {
		if (left_most->left) {
			left_most = left_most->left;
			continue;
		} 

		break;
	}

	return left_most;
}

bool bynodes_check(const bynode *self) {
	bool is_color_out_of_range = 
		self->color < 0 || self->color > bynode_black_color;

	#if cels_debug
		errors_return("self", !self)
		errors_return("self.color", is_color_out_of_range)
	#else
		if (!self) return true;
		if (is_color_out_of_range) return true;
	#endif

	return false;
}


/* bytrees */

void bytrees_next_right_procedure_private(
	bynode *self, bytree_iterator *iterator) {

	bynode *left_most = bynodes_find_left_most_private(self);

	if (left_most) {
		iterator->data = left_most;
		iterator->internal.left = left_most;
		iterator->internal.right = left_most->parent->right;
		iterator->internal.state = bytree_left_most_iterator_state;
	} else {
		iterator->data = self;
		iterator->internal.right = self;
		iterator->internal.state = bytree_right_most_iterator_state;
	}
}

bool bytrees_next(const void *self, void *iterator) {
	const bytree *s = self;
	bytree_iterator *it = iterator; 

	#if cels_debug
		errors_abort("self.data", bynodes_check(s->data));
	#endif

	if (!s || !s->data) { return false; }

	it->internal.prev = it->data;

	if (it->internal.state == bytree_initial_iterator_state) {
		if (s->data->left) {
			bynode *left_most = bynodes_find_left_most_private(s->data);
			it->data = left_most;
			it->internal.left = left_most;
			it->internal.right = left_most->parent->right;
			it->internal.state = bytree_left_most_iterator_state;
			return true;
		} else if (s->data->right) {
			bytrees_next_right_procedure_private(s->data->right, it);
			return true;
		} else {
			it->data = s->data;
			it->internal.state = bytree_finished_iterator_state;
			return true;
		}
	} else if (it->internal.state == bytree_left_most_iterator_state) {
		if (it->internal.right) {
			bytrees_next_right_procedure_private(it->internal.right, it);
			return true;
		} 

		it->data = it->internal.left->parent;
		it->internal.right = it->internal.left->parent;
		it->internal.state = bytree_returning_node_iterator_state;
		return true;
	} else if (it->internal.state == bytree_right_most_iterator_state) {
		it->data = it->internal.right->parent;
		it->internal.right = it->internal.right->parent;
		it->internal.state = bytree_returning_node_iterator_state;
		return true;
	} else if (it->internal.state == bytree_returning_node_iterator_state) {
		if (!it->internal.right->parent) {
			return false;
		} 

		bool is_left_side = 
			it->internal.right->parent->left == it->internal.right;

		if (is_left_side) {
			if (it->internal.right->parent->right) {
				bytrees_next_right_procedure_private(
					it->internal.right->parent->right, it);
				return true;
			} else {
				it->data = it->internal.right->parent;
				it->internal.right = it->internal.right->parent;
				it->internal.state = bytree_returning_node_iterator_state;
				return true;
			}
		} else {
			it->data = it->internal.right->parent;
			it->internal.right = it->internal.right->parent;
			it->internal.state = bytree_returning_node_iterator_state;
			return true;
		}
	} else {
		return false;
	}

	return false;
}

void *bytrees_get(const void *self, size_t hash) {
	const bytree *s = self;

	#if cels_debug
		errors_abort("self", !s);
		errors_abort("self.data", bynodes_check(s->data));
	#endif

	bynode *node = s->data;
	while (node) {
		if (hash < node->hash) {
			if (node->left && node->frequency > 0) {
				node = node->left;
				continue;
			} 

			return null;
		} else if (hash > node->hash) {
			if (node->right && node->frequency > 0) {
				node = node->right;
				continue;
			} 

			return null;
		} else {
			return node;
		}
	}

	return null;
}

error bytrees_push(void *self, void *item, size_t hash, const allocator *mem) {
	bytree *s = self;

	bynode *node = mems_alloc(mem, s->node_size);
	if (!node) { return fail; }

	node->hash = hash;
	node->color = bynode_black_color;
	node->frequency = 1;
	memcpy(&node->data, item, s->type_size);

	#if cels_debug
		errors_abort("self", !self);
		errors_abort("node", bynodes_check(node));
	#endif

    if (!s->data) { 
		s->data = node; 
		return ok;
	}

	bynode *next = s->data;
	while (next) {
		if (node->hash < next->hash) {
			if (next->left && next->frequency > 0) {
				next = next->left;
				continue;
			} 

			next->left = node;
			next->left->parent = next;
			break;
		} else if (node->hash > next->hash) {
			if (next->right && next->frequency > 0) {
				next = next->right;
				continue;
			} 

			next->right = node;
			next->right->parent = next;
			break;
		} else {
			next->frequency++;
			mems_dealloc(mem, node, s->node_size);
			return fail;
		}
	}

	bynodes_normalize_private(s->data, node);

	return ok;
}


/* mutrees */

error mutrees_push(void *self, void *node, void *item) {
	mutree *s = self;
	munode *i = item;

	#if cels_debug
		errors_abort("self", !self);
		errors_abort("item", !item);
	#endif

	if (!s || !i) { return fail; } 

	if (!s->data) {
		s->data = i;
		s->size++;
		return ok;
	} 

	#if cels_debug
		errors_abort("node", !node);
	#endif

	if (!node) { return fail; } 

	munode *last_of_chain = node;
	while (true) {
		if (last_of_chain->left) {
			last_of_chain = last_of_chain->left;
			continue;
		}

		break;
	}

	i->parent = last_of_chain->parent;
	last_of_chain->left = i;
	s->size++;
	return ok;
}

error mutrees_attach(void *self, void *node, void *item) {
	mutree *s = self;
	munode *n = node;
	munode *i = item;

	#if cels_debug
		errors_abort("self", !self);
		errors_abort("item", !item);
	#endif

	if (!s || !i) { return fail; } 

	if (!s->data) {
		s->data = i;
		s->size++;
		return ok;
	} 

	if (n->down) { return fail; }

	n->down = i;
	i->parent = node;
	s->size++;
	return ok;
}

bool mutrees_next(const void *self, void *iterator) {
	const mutree *s = self;
	mutree_iterator *it = iterator;

	#if cels_debug
		errors_abort("self", !self);
		errors_abort("iterator", !iterator);
	#endif

	if (!s->data || !it) { return false; }

	if (!it->internal.current) {
		it->data = s->data;
		it->internal.current = s->data;
		return true;
	} 

	munode *down = it->internal.current->down;
	if (down) {
		it->data = down;
		it->internal.current = down;
		return true;
	}
	
	munode *left = it->internal.current->left;
	if (left) {
		it->data = left;
		it->internal.current = left;
		return true;
	}

	munode *parent = it->internal.current->parent;
	if (!parent) {
		return false;
	}

	munode *parent_left = parent->left;
	if (parent_left) {
		it->data = parent_left;
		it->internal.current = parent_left;
		return true;
	}

	munode *grandparent = parent;
	while (grandparent) {
		grandparent = grandparent->parent;
		if (grandparent->left) { break; }
	}

	if (grandparent) {
		it->data = grandparent->left;
		it->internal.current = grandparent->left;
		return true;
	}

	return false;
}

bool mutrees_next_breadth_wise(
	const void *self, void *iterator, const allocator *mem) {

	const mutree *s = self;
	mutree_iterator *it = iterator;

	#if cels_debug
		errors_abort("self", !self);
		errors_abort("iterator", !iterator);
	#endif

	if (!s->data || !it) { return false; }

	if (!it->internal.current) {
		it->internal.branches = mems_alloc(mem, sizeof(uintptr_t) * 16);
		if (!it->internal.branches) { return false; }

		it->internal.capacity = 16;
		it->internal.size = 0;
		it->internal.cursor = 0;

		it->data = s->data;
		it->internal.current = s->data;
		return true;
	}

	if (it->internal.cursor > it->internal.capacity >> 1) {
		size_t rest = it->internal.capacity - it->internal.cursor;
		for (size_t i = 0; i < rest; i++) {
			size_t offset = i + it->internal.cursor;
			it->internal.branches[i] = it->internal.branches[offset];
		}

		it->internal.size -= it->internal.cursor;
		it->internal.cursor = 0;
	}

	if (it->internal.size <= it->internal.capacity >> 1) {
		size_t capacity = it->internal.capacity * sizeof(uintptr_t);
		size_t new_capacity = capacity >> 1;

		void *new_branches = 
			mems_realloc(mem, it->internal.branches, capacity, new_capacity);
		if (!new_branches) { goto cleanup; }

		it->internal.branches = new_branches;
		it->internal.capacity = new_capacity;
	}

	munode *left = it->internal.current->left;
	if (left) {
		if (left->down) {
			if (++it->internal.size <= it->internal.capacity) {
				size_t offset = it->internal.size - 1;
				it->internal.branches[offset] = (uintptr_t)left->down;
			} else {
				size_t offset = it->internal.size - 1;
				size_t capacity = it->internal.capacity * sizeof(uintptr_t);
				size_t new_capacity = capacity << 1;

				#if cels_debug
					errors_abort(
						"new_capacity (overflow)", 
						new_capacity < capacity);
				#else
					if (new_capacity < capacity) { goto cleanup; }
				#endif

				void *new_branches = mems_realloc(
					mem, it->internal.branches, capacity, new_capacity);

				if (!new_branches) { goto cleanup; }

				it->internal.branches = new_branches;
				it->internal.capacity = new_capacity;
				it->internal.branches[offset] = (uintptr_t)left->down;
			}
		}

		it->data = left;
		it->internal.current = left;
		return true;
	} else {
		if (it->internal.size == 0) { goto cleanup; }

		size_t offset = it->internal.cursor;
		it->data = (void *)it->internal.branches[offset];
		it->internal.current = (void *)it->internal.branches[offset];
		++it->internal.cursor;
		return true;
	}

	return false;

	cleanup:
	mems_dealloc(mem, it->internal.branches, it->internal.capacity);
	return false;
}

void mutrees_free(void *self, freefunc cleaner, const allocator *mem) {
	mutree *s = self;
	mutree_iterator it = {0};
	while(mutrees_next_breadth_wise(self, &it, mem)) {
		if (cleaner) {
			cleaner(&it.data->data, mem);
		}

		mems_dealloc(mem, it.data, s->node_size);
	}
}


/* pools */

pool_block *pool_blocks_init_private(
	size_t item_size, size_t capacity, const allocator *mem) {

	void *items = mems_alloc(mem, capacity * item_size);
	if (!items) { return null; }
	
	pool_block node = {
		.next=null,
		.data=items,
		.capacity=capacity,
		.size=0,
	};
	
	pool_block *node_capsule = mems_alloc(mem, sizeof(pool_block));
	*node_capsule = node;
	
	return node_capsule;
}

void pools_init(
	void *self, 
	size_t item_size, 
	size_t type_size, 
	size_t offset_size, 
	size_t capacity, 
	const allocator *mem) {

	pool *s = self;

	pool_block *block = pool_blocks_init_private(item_size, capacity, mem);
	errors_abort("block", !block);
	
	pool pool = {
		.data=block,
		.capacity=capacity,
		.item_size=item_size,
		.type_size=type_size,
		.offset_size=offset_size,
	};
}

error pools_push(void *self, void *item, const allocator *mem) {
	pool *s = self;

	if (!s || !s->capacity) { return fail; }
	
	const int zero_value = 0;
	pool_block *node = s->data;
	while (node) {
		char *end = (char *)node->data + (node->capacity * s->item_size);
		for (char *i = (char *)node->data; i < end; i += s->item_size) {
			int *item = (int *)i;

			if (*item == 0) {
				*item = 1;
				memcpy(i + s->offset_size, item, s->type_size);
				return ok;
			}
		}
		
		if (!node->next) {
			pool_block *nodes = pool_blocks_init_private(
				s->item_size, s->data->capacity, mem);
			if (!nodes) { return fail; }
			
			node->next = nodes;
			void *data = node->next->data;

			int *data_status = data;
			*data_status = 1;

			memcpy(data + s->offset_size, item, s->type_size);
			return ok;
		}
		
		node = node->next;
	}
	
	return ok;
}

bool pools_next(void *self, void *iterator) {
	pool *s = self;
	pool_iterator *it = iterator;

	if (!s) { return false; }
	
	if (!it->next) {
		it->next = s->data;
		it->current = (char *)it->next->data;
		it->end = (char *)it->next->data + (it->next->capacity * s->item_size);
	}
	
	while (it->next) {
		for (char *i = it->current; i < it->end; i += s->item_size) {
			int *data_status = (int *)i;
			if (*data_status > 0) {
				it->data = i + s->offset_size;
				it->current = i;
				return true;
			}
		}
		
		it->next = it->next->next;
		it->current = (char *)it->next->data;
		it->end = (char *)it->next->data + (it->next->capacity * s->item_size);
	}
	
	return false;
}

void pools_free(void *self, freefunc cleaner, const allocator *mem) {
	pool *s = self;

	if (!self) { return; }
	
	pool_block *node = s->data;
	while (node) {
		if (cleaner) {
			char *current = (char *)node->data;
			char *end = (char *)node->data + (node->capacity * s->item_size);

			for (char *i = current; i < end; i += s->item_size) {
				int *data_status = (int *)i;
				if (data_status != 0) {
					void *item = i + s->offset_size;
					cleaner(item, mem);
				}
			}
		}
		
		mems_dealloc(mem, node->data, node->capacity * s->item_size);
		
		pool_block *prev = node;
		node = node->next;
		
		mems_dealloc(mem, prev, sizeof(pool_block));
	}
	
	s->data = null;
	s->size = 0;
	s->capacity = 0;
}


/* sets */

bool sets_next(const void *self, void *iterator) {
	return bytrees_next(self, iterator);
}

void *sets_get(const void *self, size_t hash) {
	bynode *node = bytrees_get(self, hash);
	if (!node) {
		return null;
	}

	return &node->data;
}

error sets_push(void *self, void *item, size_t hash, const allocator *mem) {
	return bytrees_push(self, item, hash, mem);
}

void sets_free(void *self, freefunc cleaner, const allocator *mem) {
	const bytree *s = self;

	bytree_iterator it = {0};
	while (bytrees_next(self, &it)) {
		if (cleaner) {
			cleaner(&it.data->data, mem);
		}

		if (it.internal.prev) {
			mems_dealloc(mem, it.internal.prev, s->node_size);
		}

		if (it.data == s->data) {
			mems_dealloc(mem, it.data, s->node_size);
		}
	}
}


/* maps */

bool maps_next(const void *self, void *iterator) {
	return bytrees_next(self, iterator);
}

void *maps_get(const void *self, size_t hash) {
	const bytree *s = self;

	bynode *node = bytrees_get(self, hash);
	if (!node) { return null; }
	if (!node->data) { return null; }

	void *offset = &node->data;
	return (char *)offset + s->extra_size;
}

error maps_push(void *self, void *item, size_t hash, const allocator *mem) {
	return bytrees_push(self, item, hash, mem);
}

void maps_free(
	void *self, 
	freefunc key_cleaner, 
	freefunc value_cleaner, 
	const allocator *mem) {

	const bytree *s = self;

	bytree_iterator it = {0};
	while (bytrees_next(self, &it)) {
		if (key_cleaner) {
			key_cleaner(&it.data->data, mem);
		}

		if (value_cleaner) {
			void *v = (char *)&it.data->data + s->extra_size;
			value_cleaner(v, mem);
		}

		if (it.internal.prev) {
			mems_dealloc(mem, it.internal.prev, s->node_size);
		}

		if (it.data == s->data) {
			mems_dealloc(mem, it.data, s->node_size);
		}
	}
}
