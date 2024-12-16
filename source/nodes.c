#include "nodes.h" 
#include "utils.h"

void bnodes_right_rotate_private(unused bnode *n, bnode *new_node) {
	//TODO: checks
    bnode* left = new_node->left;
    new_node->left = left->right;

    if (new_node->left) {
        new_node->left->parent = new_node;
	}

    left->parent = new_node->parent;
    if (!new_node->parent) {
        n = left;
	} else if (new_node == new_node->parent->left) {
        new_node->parent->left = left;
	} else {
        new_node->parent->right = left;
	}

    left->right = new_node;
    new_node->parent = left;
}
 
void bnodes_left_rotate_private(unused bnode *n, bnode *new_node) {
	//TODO: checks
    bnode* right = new_node->right;
    new_node->right = right->left;

    if (new_node->right) {
        new_node->right->parent = new_node;
	}

    right->parent = new_node->parent;
    if (!new_node->parent) {
        n = right;
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
 * I've robbed this from gist.github.com/VictorGarritano/5f894be162d39e9bdd5c
 */
void bnodes_normalize_private(bnode *n, bnode *new_node) {
	bool is_new_node_unique =  new_node != n && new_node != n->left && new_node != n->right;
	bool is_new_node_parent_red = new_node->parent && new_node->parent->color == bnodes_red_color;

    // iterate until new_node is not the n and new_node's parent color is red
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
		} else if (y->color == bnodes_red_color) {
            y->color = bnodes_black_color;
            new_node->parent->color = bnodes_black_color;
            new_node->parent->parent->color = bnodes_red_color;
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
                bnodes_color color = new_node->parent->color ;
                new_node->parent->color = new_node->parent->parent->color;
                new_node->parent->parent->color = color;
                bnodes_right_rotate_private(n,new_node->parent->parent);
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
                bnodes_color color = new_node->color ;
                new_node->color = new_node->parent->parent->color;
                new_node->parent->parent->color = color;
                bnodes_left_rotate_private(n,new_node->parent);
                bnodes_right_rotate_private(n,new_node->parent->parent);
            }

            // Right-Right (RR) case, do following
            // (i)  Swap color of parent and grandparent
            // (ii) Left Rotate Grandparent

			does_parent_exists = new_node->parent && new_node->parent->parent;
			bool case3 = does_parent_exists && 
				new_node->parent == new_node->parent->parent->right && 
				new_node == new_node->parent->right;

            if (case3) {
                bnodes_color color = new_node->parent->color;
                new_node->parent->color = new_node->parent->parent->color;
                new_node->parent->parent->color = color;
                bnodes_left_rotate_private(n,new_node->parent->parent);
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
                bnodes_color color = new_node->color;
                new_node->color = new_node->parent->parent->color;
                new_node->parent->parent->color = color;
                bnodes_right_rotate_private(n,new_node->parent);
                bnodes_left_rotate_private(n,new_node->parent->parent);
            }
        }
    }
    n->color = bnodes_black_color; 
	//keep n always black
}

void bnodes_free(bnode *n, const allocator *mem, freefunc cleanup) {
	if (n == null) {
		return;
	}

	if (cleanup) {
		cleanup(&n->data, mem);
	}

	mems_dealloc(mem, n, sizeof(bnode));
	n = null;
}

bnode* bnodes_push_private(bnode *n, bnode *new_node, bool *error, size_t stackframe) {
	//TODO: check new_node

    if (n == null) { return new_node; }

	if (stackframe > nodes_max_recursion) {
		*error = true;

		return n;
	}
 
    if (new_node->hash < n->hash) {
        n->left = bnodes_push_private(n->left, new_node, error, ++stackframe);
        n->left->parent = n;
    } else if (new_node->hash > n->hash) {
        n->right = bnodes_push_private(n->right, new_node, error, ++stackframe);
        n->right->parent = n;
    } else {
		n->frequency++;
		*error = true;
	}

    return n;
}

void bnodes_push(bnode **n, bnode *new_node, const allocator *mem, freefunc cleanup, bool *error) {
    if (n == null) { 
		*n = new_node; 
	}

	bool err = false;
	*n = bnodes_push_private(*n, new_node, &err, 0);

	if (!err) {
		bnodes_normalize_private(*n, new_node);
	}

	if (err) {
		bnodes_free(new_node, mem, cleanup);
	}

	if (err && error) {
		*error = err;	
	}
}

bnode* bnodes_get_private(bnode *n, size_t hash, size_t stackframe) {
	//TODO: check n 
 
	if (n == null) {
		return null;
	}

	if (stackframe > nodes_max_recursion) {
		return null;
	}

    if (hash < n->hash) {
        return bnodes_get_private(n->left, hash, ++stackframe);
    } else if (hash > n->hash) {
        return bnodes_get_private(n->right, hash, ++stackframe);
    } else {
		return n;
	}

    return null;
}

bnode *bnodes_get(bnode *n, size_t hash) {
	return bnodes_get_private(n, hash, 0);
}

void *bnodes_get_data(bnode *n, size_t hash) {
	bnode *node = bnodes_get_private(n, hash, 0);
	if (node == null) {
		return null;
	} 

	return &node->data;
}

size_t bnodes_get_frequency(bnode *n, size_t hash) {
	bnode *node = bnodes_get_private(n, hash, 0);
	if (node == null) { return 0; } 

	return node->frequency;
}

void bnodes_traverse_private(bnode *n, callfunc callback, size_t stackframe) {
    if (n == null || stackframe > nodes_max_recursion) { 
		return; 
	}

	bnode *left = n->left;
	bnode *right = n->right;

    bnodes_traverse_private(left, callback, ++stackframe);
	callback(n);
    bnodes_traverse_private(right, callback, ++stackframe);
}

void bnodes_traverse(bnode *n, callfunc callback) {
	bnodes_traverse_private(n, callback, 0);
}

void bnodes_free_all_private(bnode *n, const allocator *mem, freefunc cleanup, size_t stackframe) {
    if (n == null || stackframe > nodes_max_recursion) { 
		return; 
	}

	bnode *left = n->left;
	bnode *right = n->right;

    bnodes_free_all_private(left, mem, cleanup, ++stackframe);
	cleanup(n, mem);
    bnodes_free_all_private(right, mem, cleanup, ++stackframe);
}

void bnodes_free_all(bnode *n, const allocator *mem, freefunc cleanup) {
	bnodes_free_all_private(n, mem, cleanup, 0);
}

size_t bnodes_length_private(bnode *n, size_t stackframe) {
    if (n == null || stackframe > nodes_max_recursion) { 
		return 0; 
	}

	size_t count = 1;
    count += bnodes_length_private(n->left, ++stackframe);
    count += bnodes_length_private(n->right, ++stackframe);

	return count;
}

size_t bnodes_length(bnode *n) {
	return bnodes_length_private(n, 0);
}
