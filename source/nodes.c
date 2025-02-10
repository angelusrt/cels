#include "nodes.h" 

/* bynary_nodes */

void bynary_nodes_right_rotate_private(notused bynary_node *self, bynary_node *new_node) {
	#if cels_debug
		errors_abort("self", bynary_nodes_check(self));
		errors_abort("new_node", bynary_nodes_check(new_node));
	#endif

    bynary_node* left = new_node->left;
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
 
void bynary_nodes_left_rotate_private(notused bynary_node *self, bynary_node *new_node) {
	#if cels_debug
		errors_abort("self", bynary_nodes_check(self));
		errors_abort("new_node", bynary_nodes_check(new_node));
	#endif

    bynary_node* right = new_node->right;
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
 * violentions made by insertion by bynary_nodes_push.
 *
 * I've got this from gist.github.com/VictorGarritano/5f894be162d39e9bdd5c
 */
void bynary_nodes_normalize_private(bynary_node *self, bynary_node *new_node) {
	#if cels_debug
		errors_abort("self", bynary_nodes_check(self));
		errors_abort("new_node", bynary_nodes_check(new_node));
	#endif

	bool is_new_node_unique =  new_node != self && new_node != self->left && new_node != self->right;
	bool is_new_node_parent_red = new_node->parent && new_node->parent->color == bynary_node_red_color;

    // iterate until new_node is not the self and new_node's parent color is red
    while (is_new_node_unique && is_new_node_parent_red) {
        bynary_node *y;

        // Find uncle and store uncle in y
		bool does_parent_exists = new_node->parent && new_node->parent->parent;
        if (does_parent_exists && new_node->parent == new_node->parent->parent->left) {
            y = new_node->parent->parent->right;
		} else {
            y = new_node->parent->parent->left;
		}

        // If uncle is RED, do following
        // (i)  Change color of parent and uncle as BLACK
        // (ii) Change color of grandparent as RED
        // (iii) Move new_node to grandparent
        if (!y) {
            new_node = new_node->parent->parent;
		} else if (y->color == bynary_node_red_color) {
            y->color = bynary_node_black_color;
            new_node->parent->color = bynary_node_black_color;
            new_node->parent->parent->color = bynary_node_red_color;
            new_node = new_node->parent->parent;
        } else {
            // Left-Left (LL) case, do following
            // (i)  Swap color of parent and grandparent
            // (ii) Right Rotate Grandparent
			
			bool does_parent_exists = new_node->parent && new_node->parent->parent;
			bool case1 = does_parent_exists && 
				new_node->parent == new_node->parent->parent->left && 
				new_node == new_node->parent->left;

            if (case1) {
                bynary_node_color color = new_node->parent->color ;
                new_node->parent->color = new_node->parent->parent->color;
                new_node->parent->parent->color = color;
                bynary_nodes_right_rotate_private(self,new_node->parent->parent);
            }

            // Left-Right (LR) case, do following
            // (i)  Swap color of current node  and grandparent
            // (ii) Left Rotate Parent
            // (iii) Right Rotate Grand Parent

			does_parent_exists = new_node->parent && new_node->parent->parent;
			bool case2 = does_parent_exists && 
				new_node->parent == new_node->parent->parent->left && 
				new_node == new_node->parent->right;

            if (case2) {
                bynary_node_color color = new_node->color ;
                new_node->color = new_node->parent->parent->color;
                new_node->parent->parent->color = color;
                bynary_nodes_left_rotate_private(self,new_node->parent);
                bynary_nodes_right_rotate_private(self,new_node->parent->parent);
            }

            // Right-Right (RR) case, do following
            // (i)  Swap color of parent and grandparent
            // (ii) Left Rotate Grandparent

			does_parent_exists = new_node->parent && new_node->parent->parent;
			bool case3 = does_parent_exists && 
				new_node->parent == new_node->parent->parent->right && 
				new_node == new_node->parent->right;

            if (case3) {
                bynary_node_color color = new_node->parent->color;
                new_node->parent->color = new_node->parent->parent->color;
                new_node->parent->parent->color = color;
                bynary_nodes_left_rotate_private(self,new_node->parent->parent);
            }

            // Right-Left (RL) case, do following
            // (i)  Swap color of current node  and grandparent
            // (ii) Right Rotate Parent
            // (iii) Left Rotate Grand Parent

			does_parent_exists = new_node->parent && new_node->parent->parent;
			bool case4 = does_parent_exists && 
			new_node->parent == new_node->parent->parent->right && 
			new_node == new_node->parent->left;

            if (case4) {
                bynary_node_color color = new_node->color;
                new_node->color = new_node->parent->parent->color;
                new_node->parent->parent->color = color;
                bynary_nodes_right_rotate_private(self,new_node->parent);
                bynary_nodes_left_rotate_private(self,new_node->parent->parent);
            }
        }
    }

    self->color = bynary_node_black_color; 
}

bynary_node *bynary_nodes_find_left_most_private(bynary_node *self) {
	bynary_node *left_most = self->left;
	while (true) {
		if (left_most->left) {
			left_most = left_most->left;
			continue;
		} 

		break;
	}

	return left_most;
}

bool bynary_nodes_check(const bynary_node *self) {
	#if cels_debug
		errors_return("self", !self)

		bool is_color_out_of_range = self->color < 0 || self->color > bynary_node_black_color;
		errors_return("self.color", is_color_out_of_range)
	#else
		if (!self) return true;
		if (self->color < 0 || self->color > bynary_node_black_color) return true;
	#endif

	return false;
}

void bynary_nodes_initialize(bynary_node *self, size_t hash) {
	self->hash = hash;
	self->color = bynary_node_black_color;
	self->frequency = 1;
}

/* bynary_trees */

void bynary_trees_next_right_procedure_private(bynary_node *self, bynary_tree_iterator *iterator) {
	bynary_node *left_most = bynary_nodes_find_left_most_private(self);

	if (left_most) {
		iterator->data = left_most;
		iterator->internal.left = left_most;
		iterator->internal.right = left_most->parent->right;
		iterator->internal.state = bynary_tree_left_most_iterator_state;
	} else {
		iterator->data = self;
		iterator->internal.right = self;
		iterator->internal.state = bynary_tree_right_most_iterator_state;
	}
}

bool bynary_trees_next(bynary_tree *self, bynary_tree_iterator *iterator) {
	#if cels_debug
		errors_abort("self.data", bynary_nodes_check(self->data));
	#endif

	if (!self || !self->data) { return false; }

	if (iterator->internal.state == bynary_tree_initial_iterator_state) {
		if (self->data->left) {
			bynary_node *left_most = bynary_nodes_find_left_most_private(self->data);
			iterator->data = left_most;
			iterator->internal.left = left_most;
			iterator->internal.right = left_most->parent->right;
			iterator->internal.state = bynary_tree_left_most_iterator_state;
			return true;
		} else if (self->data->right) {
			bynary_trees_next_right_procedure_private(self->data->right, iterator);
			return true;
		} else {
			iterator->data = self->data;
			iterator->internal.state = bynary_tree_finished_iterator_state;
			return true;
		}
	} else if (iterator->internal.state == bynary_tree_left_most_iterator_state) {
		if (iterator->internal.right) {
			bynary_trees_next_right_procedure_private(iterator->internal.right, iterator);
			return true;
		} 

		iterator->data = iterator->internal.left->parent;
		iterator->internal.right = iterator->internal.left->parent;
		iterator->internal.state = bynary_tree_returning_node_iterator_state;
		return true;
	} else if (iterator->internal.state == bynary_tree_right_most_iterator_state) {
		iterator->data = iterator->internal.right->parent;
		iterator->internal.right = iterator->internal.right->parent;
		iterator->internal.state = bynary_tree_returning_node_iterator_state;
		return true;
	} else if (iterator->internal.state == bynary_tree_returning_node_iterator_state) {
		if (!iterator->internal.right->parent) {
			return false;
		} 

		bool is_left_side = iterator->internal.right->parent->left == iterator->internal.right;
		if (is_left_side) {
			if (iterator->internal.right->parent->right) {
				bynary_trees_next_right_procedure_private(
					iterator->internal.right->parent->right, iterator);
				return true;
			} else {
				iterator->data = iterator->internal.right->parent;
				iterator->internal.right = iterator->internal.right->parent;
				iterator->internal.state = bynary_tree_returning_node_iterator_state;
				return true;
			}
		} else {
			iterator->data = iterator->internal.right->parent;
			iterator->internal.right = iterator->internal.right->parent;
			iterator->internal.state = bynary_tree_returning_node_iterator_state;
			return true;
		}
	} else {
		return false;
	}

	return false;
}

bynary_node *bynary_trees_get(bynary_tree *self, size_t hash) {
	#if cels_debug
		errors_abort("self", !self);
		errors_abort("self.data", bynary_nodes_check(self->data));
	#endif

	bynary_node *node = self->data;
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

error bynary_trees_push(bynary_tree *self, bynary_node *node) {
	#if cels_debug
		errors_abort("self", !self);
		errors_abort("node", bynary_nodes_check(node));
	#endif

    if (!self->data) { 
		self->data = node; 
		return ok;
	}

	bynary_node *next = self->data;
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
			return fail;
		}
	}

	bynary_nodes_normalize_private(self->data, node);

	return ok;
}

/* multiary_trees */

multiary_tree multiary_trees_init() {
	return (multiary_tree){0};
}

error multiary_trees_push(multiary_tree *self, multiary_node *node, multiary_node *item) {
	#if cels_debug
		errors_abort("self", !self);
		errors_abort("item", !item);
	#endif

	if (!self || !item) { return fail; } 

	if (!self->data) {
		self->data = item;
		self->size++;
		return ok;
	} 

	multiary_node *last_of_chain = node;
	while (true) {
		if (last_of_chain->left) {
			last_of_chain = last_of_chain->left;
			continue;
		}

		break;
	}

	last_of_chain->left = item;
	self->size++;
	return ok;
}

error multiary_trees_attach(multiary_tree *self, multiary_node *node, multiary_node *item) {
	#if cels_debug
		errors_abort("self", !self);
		errors_abort("item", !item);
	#endif

	if (!self || !item) { return fail; } 

	if (!self->data) {
		self->data = item;
		self->size++;
		return ok;
	} 

	if (node->down) { return fail; }

	node->down = item;
	node->parent = node->down;
	self->size++;
	return ok;
}

bool multiary_trees_next(multiary_tree *self, multiary_tree_iterator *iterator) {
	#if cels_debug
		errors_abort("self", !self);
		errors_abort("iterator", !iterator);
	#endif

	if (!self->data || !iterator) { return false; }

	if (!iterator->internal.current) {
		iterator->data = self->data;
		iterator->internal.current = self->data;
		return true;
	} 

	multiary_node *down = iterator->internal.current->down;
	if (down) {
		iterator->data = down;
		iterator->internal.current = down;
		return true;
	}
	
	multiary_node *left = iterator->internal.current->left;
	if (left) {
		iterator->data = left;
		iterator->internal.current = left;
		return true;
	}

	multiary_node *parent = iterator->internal.current->parent;
	if (!parent) {
		return false;
	}

	multiary_node *parent_left = parent->left;
	if (parent_left) {
		iterator->data = parent_left;
		iterator->internal.current = parent_left;
		return true;
	}

	multiary_node *grandparent = parent;
	while (grandparent) {
		grandparent = grandparent->parent;
		if (grandparent->left) { break; }
	}

	if (grandparent) {
		iterator->data = grandparent->left;
		iterator->internal.current = grandparent->left;
		return true;
	}

	return false;
}

bool multiary_trees_next_breadth_wise(
	multiary_tree *self, multiary_tree_iterator *iterator, const allocator *mem
) {
	#if cels_debug
		errors_abort("self", !self);
		errors_abort("iterator", !iterator);
	#endif

	if (!self->data || !iterator) { return false; }

	if (!iterator->internal.current) {
		iterator->internal.branches = mems_alloc(mem, sizeof(uintptr_t) * 16);
		if (!iterator->internal.branches) { return false; }

		iterator->internal.capacity = 16;
		iterator->internal.size = 0;
		iterator->internal.cursor = 0;

		iterator->data = self->data;
		iterator->internal.current = self->data;
		return true;
	}

	if (iterator->internal.cursor > iterator->internal.capacity >> 1) {
		size_t rest = iterator->internal.capacity - iterator->internal.cursor;
		for (size_t i = 0; i < rest; i++) {
			size_t offset = i + iterator->internal.cursor;
			iterator->internal.branches[i] = iterator->internal.branches[offset];
		}

		iterator->internal.size -= iterator->internal.cursor;
		iterator->internal.cursor = 0;
	}

	if (iterator->internal.size <= iterator->internal.capacity >> 1) {
		size_t capacity = iterator->internal.capacity * sizeof(uintptr_t);
		size_t new_capacity = capacity >> 1;

		void *new_branches = 
			mems_realloc(mem, iterator->internal.branches, capacity, new_capacity);
		if (!new_branches) { goto cleanup; }

		iterator->internal.branches = new_branches;
		iterator->internal.capacity = new_capacity;
	}

	multiary_node *left = iterator->internal.current->left;
	if (left) {
		if (left->down) {
			if (++iterator->internal.size <= iterator->internal.capacity) {
				size_t offset = iterator->internal.size - 1;
				iterator->internal.branches[offset] = (uintptr_t)left->down;
			} else {
				size_t offset = iterator->internal.size - 1;
				size_t capacity = iterator->internal.capacity * sizeof(uintptr_t);
				size_t new_capacity = capacity << 1;

				#if cels_debug
					errors_abort("new_capacity (overflow)", new_capacity < capacity);
				#else
					if (new_capacity < capacity) { goto cleanup; }
				#endif

				void *new_branches = 
					mems_realloc(mem, iterator->internal.branches, capacity, new_capacity);
				if (!new_branches) { goto cleanup; }

				iterator->internal.branches = new_branches;
				iterator->internal.capacity = new_capacity;
				iterator->internal.branches[offset] = (uintptr_t)left->down;
			}
		}

		iterator->data = left;
		iterator->internal.current = left;
		return true;
	} else {
		if (iterator->internal.size == 0) { goto cleanup; }

		size_t offset = iterator->internal.cursor;
		iterator->data = (void *)iterator->internal.branches[offset];
		iterator->internal.current = (void *)iterator->internal.branches[offset];
		++iterator->internal.cursor;
		return true;
	}

	return false;

	cleanup:
	mems_dealloc(mem, iterator->internal.branches, iterator->internal.capacity);
	return false;
}
