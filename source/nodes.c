#include "nodes.h" 

bool bnodes_check(const bnode *self) {
	#if cels_debug
		errors_return("self", !self)

		bool is_color_out_of_range = self->color > bnode_black_color;
		errors_return("self.color", is_color_out_of_range)
	#else
		if (!self) return true;
		if (self->color > bnode_black_color) return true;
	#endif

	return false;
}

void bnodes_right_rotate_private(notused bnode *self, bnode *new_node) {
	#if cels_debug
		errors_abort("self", bnodes_check(self));
		errors_abort("new_node", bnodes_check(new_node));
	#endif

    bnode* left = new_node->left;
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
 
void bnodes_left_rotate_private(notused bnode *self, bnode *new_node) {
	#if cels_debug
		errors_abort("self", bnodes_check(self));
		errors_abort("new_node", bnodes_check(new_node));
	#endif

    bnode* right = new_node->right;
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
 * violentions made by insertion by bnodes_push.
 *
 * I've got this from gist.github.com/VictorGarritano/5f894be162d39e9bdd5c
 */
void bnodes_normalize_private(bnode *self, bnode *new_node) {
	#if cels_debug
		errors_abort("self", bnodes_check(self));
		errors_abort("new_node", bnodes_check(new_node));
	#endif

	bool is_new_node_unique =  new_node != self && new_node != self->left && new_node != self->right;
	bool is_new_node_parent_red = new_node->parent && new_node->parent->color == bnode_red_color;

    // iterate until new_node is not the self and new_node's parent color is red
    while (is_new_node_unique && is_new_node_parent_red) {
        bnode *y;

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
		} else if (y->color == bnode_red_color) {
            y->color = bnode_black_color;
            new_node->parent->color = bnode_black_color;
            new_node->parent->parent->color = bnode_red_color;
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
                bnode_color color = new_node->parent->color ;
                new_node->parent->color = new_node->parent->parent->color;
                new_node->parent->parent->color = color;
                bnodes_right_rotate_private(self,new_node->parent->parent);
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
                bnode_color color = new_node->color ;
                new_node->color = new_node->parent->parent->color;
                new_node->parent->parent->color = color;
                bnodes_left_rotate_private(self,new_node->parent);
                bnodes_right_rotate_private(self,new_node->parent->parent);
            }

            // Right-Right (RR) case, do following
            // (i)  Swap color of parent and grandparent
            // (ii) Left Rotate Grandparent

			does_parent_exists = new_node->parent && new_node->parent->parent;
			bool case3 = does_parent_exists && 
				new_node->parent == new_node->parent->parent->right && 
				new_node == new_node->parent->right;

            if (case3) {
                bnode_color color = new_node->parent->color;
                new_node->parent->color = new_node->parent->parent->color;
                new_node->parent->parent->color = color;
                bnodes_left_rotate_private(self,new_node->parent->parent);
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
                bnode_color color = new_node->color;
                new_node->color = new_node->parent->parent->color;
                new_node->parent->parent->color = color;
                bnodes_right_rotate_private(self,new_node->parent);
                bnodes_left_rotate_private(self,new_node->parent->parent);
            }
        }
    }
    self->color = bnode_black_color; 
	//keep self always black
}

bnode* bnodes_push_private(bnode *self, bnode *new_node, bool *error, size_t stackframe) {
	#if cels_debug
		errors_abort("new_node", bnodes_check(new_node));
		errors_abort("error", error == null);
	#endif

    if (self == null) { return new_node; }

	if (stackframe > cels_max_recursion) {
		*error = true;

		return self;
	}
 
    if (new_node->hash < self->hash) {
        self->left = bnodes_push_private(self->left, new_node, error, ++stackframe);
        self->left->parent = self;
    } else if (new_node->hash > self->hash) {
        self->right = bnodes_push_private(self->right, new_node, error, ++stackframe);
        self->right->parent = self;
    } else {
		self->frequency++;
		*error = true;
	}

    return self;
}

bool bnodes_push(bnode **self, bnode *new_node) {
	#if cels_debug
		errors_abort("new_node", bnodes_check(new_node));
	#endif

    if (self == null) { 
		*self = new_node; 
	}

	bool err = false;
	*self = bnodes_push_private(*self, new_node, &err, 0);

	if (err) {
		return true;
	}

	bnodes_normalize_private(*self, new_node);

	return false;
}

bnode* bnodes_get_private(bnode *self, size_t hash, size_t stackframe) {
	if (self == null) {
		return null;
	}

	if (stackframe > cels_max_recursion) {
		return null;
	}

    if (hash < self->hash) {
        return bnodes_get_private(self->left, hash, ++stackframe);
    } else if (hash > self->hash) {
        return bnodes_get_private(self->right, hash, ++stackframe);
    } else {
		return self;
	}

    return null;
}

bnode *bnodes_get(bnode *self, size_t hash) {
	#if cels_debug
		errors_abort("self", bnodes_check(self));
	#endif

	return bnodes_get_private(self, hash, 0);
}

void *bnodes_get_data(bnode *self, size_t hash) {
	#if cels_debug
		errors_abort("self", bnodes_check(self));
	#endif

	bnode *node = bnodes_get_private(self, hash, 0);
	if (node == null) {
		return null;
	} 

	return &node->data;
}

size_t bnodes_get_frequency(bnode *self, size_t hash) {
	#if cels_debug
		errors_abort("self", bnodes_check(self));
	#endif

	bnode *node = bnodes_get_private(self, hash, 0);
	if (node == null) { return 0; } 

	return node->frequency;
}

void bnodes_traverse_private(bnode *self, callfunc callback, size_t stackframe) {
    if (self == null || stackframe > cels_max_recursion) { 
		return; 
	}

	bnode *left = self->left;
	bnode *right = self->right;

    bnodes_traverse_private(left, callback, ++stackframe);
	callback(self);
    bnodes_traverse_private(right, callback, ++stackframe);
}

void bnodes_traverse(bnode *self, callfunc callback) {
	#if cels_debug
		errors_abort("self", bnodes_check(self));
	#endif

	bnodes_traverse_private(self, callback, 0);
}

void bnodes_iterate_private(bnode *self, enfunctor func, size_t stackframe) {
    if (self == null || stackframe > cels_max_recursion) { 
		return; 
	}

	bnode *left = self->left;
	bnode *right = self->right;

    bnodes_iterate_private(left, func, ++stackframe);
	func.func(self, func.params);
    bnodes_iterate_private(right, func, ++stackframe);
}

void bnodes_iterate(bnode *self, enfunctor func) {
	#if cels_debug
		errors_abort("self", bnodes_check(self));
	#endif

	bnodes_iterate_private(self, func, 0);
}

void bnodes_free_all_private(
	bnode *self, const allocator *mem, freefunc cleanup, size_t stackframe
) {
    if (self == null || stackframe > cels_max_recursion) { 
		return; 
	}

	bnode *left = self->left;
	bnode *right = self->right;

    bnodes_free_all_private(left, mem, cleanup, ++stackframe);
	cleanup(self, mem);
    bnodes_free_all_private(right, mem, cleanup, ++stackframe);
}

void bnodes_free_all(bnode *self, const allocator *mem, freefunc cleanup) {
	#if cels_debug
		errors_abort("self", bnodes_check(self));
	#endif

	bnodes_free_all_private(self, mem, cleanup, 0);
}

size_t bnodes_length_private(bnode *self, size_t stackframe) {
    if (self == null || stackframe > cels_max_recursion) { 
		return 0; 
	}

	size_t count = 1;
    count += bnodes_length_private(self->left, ++stackframe);
    count += bnodes_length_private(self->right, ++stackframe);

	return count;
}

size_t bnodes_length(bnode *self) {
	#if cels_debug
		errors_abort("self", bnodes_check(self));
	#endif

	return bnodes_length_private(self, 0);
}
