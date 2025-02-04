#include "nodes.h" 

bool bynary_nodes_check(const bynary_node *self) {
	#if cels_debug
		errors_return("self", !self)

		bool is_color_out_of_range = self->color > bynary_node_black_color;
		errors_return("self.color", is_color_out_of_range)
	#else
		if (!self) return true;
		if (self->color > bynary_node_black_color) return true;
	#endif

	return false;
}

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
	//keep self always black
}

bynary_node* bynary_nodes_push_private(bynary_node *self, bynary_node *new_node, bool *error, size_t stackframe) {
	#if cels_debug
		errors_abort("new_node", bynary_nodes_check(new_node));
		errors_abort("error", error == null);
	#endif

    if (self == null) { return new_node; }

	if (stackframe > cels_max_recursion) {
		*error = true;

		return self;
	}
 
    if (new_node->hash < self->hash) {
        self->left = bynary_nodes_push_private(self->left, new_node, error, ++stackframe);
        self->left->parent = self;
    } else if (new_node->hash > self->hash) {
        self->right = bynary_nodes_push_private(self->right, new_node, error, ++stackframe);
        self->right->parent = self;
    } else {
		self->frequency++;
		*error = true;
	}

    return self;
}

bool bynary_nodes_push(bynary_node **self, bynary_node *new_node) {
	#if cels_debug
		errors_abort("new_node", bynary_nodes_check(new_node));
	#endif

    if (self == null) { 
		*self = new_node; 
	}

	bool err = false;
	*self = bynary_nodes_push_private(*self, new_node, &err, 0);

	if (err) {
		return true;
	}

	bynary_nodes_normalize_private(*self, new_node);

	return false;
}

bynary_node* bynary_nodes_get_private(bynary_node *self, size_t hash, size_t stackframe) {
	if (self == null) {
		return null;
	}

	if (stackframe > cels_max_recursion) {
		return null;
	}

    if (hash < self->hash) {
        return bynary_nodes_get_private(self->left, hash, ++stackframe);
    } else if (hash > self->hash) {
        return bynary_nodes_get_private(self->right, hash, ++stackframe);
    } else {
		return self;
	}

    return null;
}

bynary_node *bynary_nodes_get(bynary_node *self, size_t hash) {
	#if cels_debug
		errors_abort("self", bynary_nodes_check(self));
	#endif

	return bynary_nodes_get_private(self, hash, 0);
}

void *bynary_nodes_get_data(bynary_node *self, size_t hash) {
	#if cels_debug
		errors_abort("self", bynary_nodes_check(self));
	#endif

	bynary_node *node = bynary_nodes_get_private(self, hash, 0);
	if (node == null) {
		return null;
	} 

	return &node->data;
}

size_t bynary_nodes_get_frequency(bynary_node *self, size_t hash) {
	#if cels_debug
		errors_abort("self", bynary_nodes_check(self));
	#endif

	bynary_node *node = bynary_nodes_get_private(self, hash, 0);
	if (node == null) { return 0; } 

	return node->frequency;
}

void bynary_nodes_traverse_private(bynary_node *self, callfunc callback, size_t stackframe) {
    if (self == null || stackframe > cels_max_recursion) { 
		return; 
	}

	bynary_node *left = self->left;
	bynary_node *right = self->right;

    bynary_nodes_traverse_private(left, callback, ++stackframe);
	callback(self);
    bynary_nodes_traverse_private(right, callback, ++stackframe);
}

void bynary_nodes_traverse(bynary_node *self, callfunc callback) {
	#if cels_debug
		errors_abort("self", bynary_nodes_check(self));
	#endif

	bynary_nodes_traverse_private(self, callback, 0);
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

void bynary_nodes_next_right_procedure_private(bynary_node *self, bynary_node_iterator *iterator) {
	bynary_node *left_most = bynary_nodes_find_left_most_private(self);

	if (left_most) {
		iterator->data = left_most;
		iterator->internal.left = left_most;
		iterator->internal.right = left_most->parent->right;
		iterator->internal.state = bynary_node_left_most_iterator_state;
	} else {
		iterator->data = self;
		iterator->internal.right = self;
		iterator->internal.state = bynary_node_right_most_iterator_state;
	}
}

bool bynary_nodes_next(bynary_node *self, bynary_node_iterator *iterator) {
	#if cels_debug
		errors_abort("self", bynary_nodes_check(self));
	#endif

	if (!self) { return false; }

	if (iterator->internal.state == bynary_node_initial_iterator_state) {
		if (self->left) {
			bynary_node *left_most = bynary_nodes_find_left_most_private(self);
			iterator->data = left_most;
			iterator->internal.left = left_most;
			iterator->internal.right = left_most->parent->right;
			iterator->internal.state = bynary_node_left_most_iterator_state;
			return true;
		} else if (self->right) {
			bynary_nodes_next_right_procedure_private(self->right, iterator);
			return true;
		} else {
			iterator->data = self;
			iterator->internal.state = bynary_node_finished_iterator_state;
			return false;
		}
	} else if (iterator->internal.state == bynary_node_left_most_iterator_state) {
		if (iterator->internal.right) {
			bynary_nodes_next_right_procedure_private(self->right, iterator);
			return true;
		} 

		iterator->data = iterator->internal.left->parent;
		iterator->internal.right = iterator->internal.left->parent;
		iterator->internal.state = bynary_node_returning_node_iterator_state;
		return true;
	} else if (iterator->internal.state == bynary_node_right_most_iterator_state) {
		iterator->data = iterator->internal.right->parent;
		iterator->internal.right = iterator->internal.right->parent;
		iterator->internal.state = bynary_node_returning_node_iterator_state;
		return true;
	} else if (iterator->internal.state == bynary_node_returning_node_iterator_state) {
		if (!iterator->internal.right->parent) {
			return false;
		} 

		bool is_left_side = iterator->internal.right->parent->left == iterator->internal.right;
		if (is_left_side) {
			if (iterator->internal.right->parent->right) {
				bynary_nodes_next_right_procedure_private(
					iterator->internal.right->parent->right, iterator);
				return true;
			} else {
				iterator->data = iterator->internal.right->parent;
				iterator->internal.right = iterator->internal.right->parent;
				iterator->internal.state = bynary_node_returning_node_iterator_state;
				return true;
			}
		} else {
			iterator->data = iterator->internal.right->parent;
			iterator->internal.right = iterator->internal.right->parent;
			iterator->internal.state = bynary_node_returning_node_iterator_state;
			return true;
		}
	} else {
		return false;
	}

	return false;
}

void bynary_nodes_free_all_private(
	bynary_node *self, const allocator *mem, freefunc cleanup, size_t stackframe
) {
    if (self == null || stackframe > cels_max_recursion) { 
		return; 
	}

	bynary_node *left = self->left;
	bynary_node *right = self->right;

    bynary_nodes_free_all_private(left, mem, cleanup, ++stackframe);
	cleanup(self, mem);
    bynary_nodes_free_all_private(right, mem, cleanup, ++stackframe);
}

void bynary_nodes_free_all(bynary_node *self, const allocator *mem, freefunc cleanup) {
	#if cels_debug
		errors_abort("self", bynary_nodes_check(self));
	#endif

	bynary_nodes_free_all_private(self, mem, cleanup, 0);
}
