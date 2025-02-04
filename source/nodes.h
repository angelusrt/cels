#ifndef cels_nodes_h
#define cels_nodes_h

#include <stddef.h>

#include "mems.h"
#include "errors.h"

/*
 * The module 'nodes' is a module 
 * that implements different types of 
 * nodes, linked-pools and trees - which 
 * are the basis for other data-structures 
 * like maps and sets.
 */

/* bynary_nodes and bynary_trees */

typedef enum bynary_node_color {
	bynary_node_red_color,
	bynary_node_black_color,
} bynary_node_color;

/*
 * A bynary node data-structure to 
 * serve as basis for a bynary search 
 * algorythm implementation, 
 * know as red-black tree.
 */
#define bynary_nodes(type0, type1) \
	struct type1 { \
		bynary_node_color color; \
		size_t hash; \
		ulong frequency; \
		type1 *parent; \
		type1 *right; \
		type1 *left; \
		type0 data; \
	}

#define bynary_trees(type0) \
	struct { \
		type0 *data; \
		size_t size; \
	}

typedef struct bynary_node bynary_node;
typedef bynary_nodes(void *, bynary_node) bynary_node;
typedef bynary_trees(bynary_node) bynary_tree;

typedef enum bynary_node_iterator_state {
	bynary_node_initial_iterator_state,
	bynary_node_left_most_iterator_state,
	bynary_node_right_most_iterator_state,
	bynary_node_returning_node_iterator_state,
	bynary_node_finished_iterator_state,
} bynary_node_iterator_state;

typedef struct bynary_node_iterator_internal {
	bynary_node *left;
	bynary_node *right;
	bynary_node_iterator_state state;
} bynary_node_iterator_internal;

typedef struct bynary_node_iterator {
	bynary_node *data;
	bynary_node_iterator_internal internal;
} bynary_node_iterator;

/*
 * Makes a bynary-node of type of 'self' and from data 
 * item and hash id, which will be allocated to 
 * 'self' (a pointer).
 *
 * #allocates #may-panic #to-review
 */
#define bynary_nodes_make(self, item, id, mem) { \
	typeof(*self) node = { \
		.hash=id, \
		.data=item, \
		.color=bynary_node_black_color, \
		.frequency=1}; \
	\
	self = mems_alloc(mem, sizeof(*self)); \
	errors_abort("self", !self); \
	*self = node; \
}

/*
 * Checks if bynary_node is correct.
 *
 * #to-review
 */
bool bynary_nodes_check(const bynary_node *self);

/*
 * Pushes a new node to self. If self is null 
 * then new_bynary_node becomes root, thus being returned.
 *
 * If function errors to push new_bynary_node 
 * (and, thus, ownership) error will be 
 * set to true. Variable error may be set 
 * to null to silently error.
 *
 * #to-edit
 */
bool bynary_nodes_push(bynary_node **self, bynary_node *new_node);

/*
 * Gets node with the same hash as hash, if 
 * it fails or it doesn't find it, it returns 
 * null.
 *
 * #to-review
 */
bynary_node* bynary_nodes_get(bynary_node* self, size_t hash);

/*
 * Gets only the data inside node.
 *
 * #to-review
 */
void *bynary_nodes_get_data(bynary_node *self, size_t hash);

/*
 * Gets only the frequency inside node.
 *
 * #to-review
 */
size_t bynary_nodes_get_frequency(bynary_node *self, size_t hash);

/*
 * Traverses self in-order executing callback.
 *
 * #to-review
 */
void bynary_nodes_traverse(bynary_node *self, callfunc callback);

/*
 * Iterates self in-order executing callback.
 *
 * #to-review
 */
bool bynary_nodes_next(bynary_node *self, bynary_node_iterator *iterator);

/*
 * Traverses self in-order to free all nodes
 *
 * #to-review
 */
void bynary_nodes_free_all(bynary_node *self, const allocator *mem, freefunc cleanup);

/* sets */

#define sets(type, name) bynary_nodes(type, name) 

/*
 * A macro-template code-generator to create 
 * set's functions for a certain type.
 *
 * #to-review
 */
#define sets_generate_implementation(type, name, check0, print0, hasher0, cleanup0) \
	void name##s_free_private(name##_bynary_node *self, const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("self", bynary_nodes_check((const bynary_node *)self)); \
		} \
		\
		cleanup0(&self->data, mem); \
		mems_dealloc(mem, self, sizeof(name##_bynary_node)); \
	} \
	\
	void name##s_free_all_private(name##_bynary_node *self, const allocator *mem, size_t stackframe) { \
		if (!self || stackframe > cels_max_recursion) { \
			return; \
		} \
		\
		name##_bynary_node *left = self->left; \
		name##_bynary_node *right = self->right; \
		\
		name##s_free_all_private(left, mem, ++stackframe); \
		name##s_free_private(self, mem); \
		name##s_free_all_private(right, mem, ++stackframe); \
	} \
	\
	void *name##s_print_private(const name##_bynary_node *self) { \
		print0(&self->data); \
		printf(", "); \
		return null; \
	} \
	\
	void *name##s_println_private(const name##_bynary_node *self) { \
		print0(&self->data); \
		printf("\n"); \
		return null; \
	} \
	\
	name name##s_init(void) { \
		return (name){.data=null, .size=0}; \
	} \
	\
	type *name##s_get(const name *self, type item) { \
		if (cels_debug) { \
			errors_abort("self.data", bynary_nodes_check((const bynary_node *)self->data)); \
			errors_abort("item", check0(&item)); \
		} \
		\
		return (type *)bynary_nodes_get_data((bynary_node *)self->data, hasher0(&item)); \
	} \
	\
	bool name##s_push(name *self, type item, const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("item", check0(&item)); \
		} \
		\
		name##_bynary_node *new_bynary_node = null; \
		bynary_nodes_make(new_bynary_node, item, hasher0(&item), mem) \
		\
		bool push_error = bynary_nodes_push((bynary_node **)&self->data, (bynary_node *)new_bynary_node); \
		if (push_error) { \
			name##s_free_private(new_bynary_node, mem); \
		} else { \
			self->size++; \
		}\
		\
		return push_error; \
	} \
	\
	void name##s_free(name *self, const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("self.data", bynary_nodes_check((const bynary_node *)self->data)); \
		} \
		\
		name##s_free_all_private(self->data, mem, 0); \
	} \
	\
	void name##s_traverse(name *self, callfunc callback) { \
		if (cels_debug) { \
			errors_abort("self.data", bynary_nodes_check((const bynary_node *)self->data)); \
		} \
		\
		bynary_nodes_traverse((bynary_node *)self->data, callback); \
	} \
	\
	void name##s_print(const name *self) { \
		if (cels_debug) { \
			errors_abort("self.data", bynary_nodes_check((const bynary_node *)self->data)); \
		} \
		\
		bynary_nodes_traverse((bynary_node *)self->data, (callfunc)name##s_print_private); \
		if (self->size > 0) { \
			putchar('\b'); \
			putchar('\b'); \
			putchar(' '); \
		} \
	} \
	\
	void name##s_println(const name *self) { \
		if (cels_debug) { \
			errors_abort("self.data", bynary_nodes_check((const bynary_node *)self->data)); \
		} \
		\
		bynary_nodes_traverse((bynary_node *)self->data, (callfunc)name##s_println_private); \
	} \
	\
	void name##s_debug(const name *self) { \
		if (cels_debug) { \
			errors_abort("self.data", bynary_nodes_check((const bynary_node *)self->data)); \
		} \
		\
		printf("<"#name">{.size: %zu, .data: {", self->size); \
		bynary_nodes_traverse((bynary_node *)self->data, (callfunc)name##s_print_private); \
		if (self->size > 0) { \
			putchar('\b'); \
			putchar('\b'); \
		} \
		printf("}}"); \
	} \
	\
	void name##s_debugln(const name *self) { \
		if (cels_debug) { \
			errors_abort("self.data", bynary_nodes_check((const bynary_node *)self->data)); \
		} \
		\
		name##s_debug(self); \
		printf("\n"); \
	} \

/*
 * A macro-template code-generator to create 
 * set's definitions for a certain type.
 *
 * #to-review
 */
#define sets_generate_definition(type, name) \
	typedef struct name##_bynary_node name##_bynary_node; \
	typedef bynary_nodes(type, name##_bynary_node) name##_bynary_node; \
	typedef bynary_trees(name##_bynary_node) name; \
	\
	name name##s_init(void); \
	\
	bool name##s_push(name *self, type item, const allocator *mem); \
	\
	cels_warn_unused \
	type *name##s_get(const name *self, type item); \
	\
	void name##s_traverse(name *self, callfunc callback); \
	\
	void name##s_print(const name *self); \
	\
	void name##s_println(const name *self); \
	\
	void name##s_debug(const name *self); \
	\
	void name##s_debugln(const name *self); \
	\
	void name##s_free(name *self, const allocator *mem);
	
/* maps */

#define key_pairs(type0, type1) struct { \
	typeof(type0) key; \
    typeof(type1) value; \
}

#define maps(type, name) bynary_nodes(type, name) 

/*
 * A macro-template code-generator to create 
 * set's functions for a certain type.
 *
 * #to-review
 */
#define maps_generate_implementation( \
	type0, type1, type2, name, check0, check1, print0, print1, hasher0, cleanup0, cleanup1 \
) \
	void name##s_free_private(name##_bynary_node *self, const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("self", bynary_nodes_check((const bynary_node *)self)); \
		} \
		\
		cleanup0(&self->data.key, mem); \
		cleanup1(&self->data.value, mem); \
		mems_dealloc(mem, self, sizeof(name)); \
		self = null; \
	} \
	\
	void name##s_free_all_private(name##_bynary_node *self, const allocator *mem, size_t stackframe) { \
		if (!self || stackframe > cels_max_recursion) { return; } \
		\
		name##_bynary_node *left = self->left; \
		name##_bynary_node *right = self->right; \
		\
		name##s_free_all_private(left, mem, ++stackframe); \
		name##s_free_private(self, mem); \
		name##s_free_all_private(right, mem, ++stackframe); \
	} \
	\
	void *name##s_print_private(const name##_bynary_node *self) { \
		print0(&self->data.key); \
		printf(": "); \
		print1(&self->data.value); \
		printf(", "); \
		return null; \
	} \
	\
	void *name##s_println_private(const name##_bynary_node *self) { \
		print0(&self->data.key); \
		printf(": "); \
		print1(&self->data.value); \
		printf("\n"); \
		return null; \
	} \
	\
	void *name##s_debug_private(const name##_bynary_node *self) { \
		printf("\""); \
		print0(&self->data.key); \
		printf(": "); \
		print1(&self->data.value); \
		printf("\", "); \
		return null; \
	} \
	\
	name name##s_init(void) { \
		return (name){.data=null, .size=0}; \
	} \
	\
	type1 *name##s_get(const name *self, type0 item) { \
		if (cels_debug) { \
			errors_abort("self.data", bynary_nodes_check((const bynary_node *)self->data)); \
			errors_abort("item", check0(&item)); \
		} \
		\
		type2 *temp = bynary_nodes_get_data((bynary_node *)self->data, hasher0(&item)); \
		if (!temp) { return null; } \
		\
		return &temp->value; \
	} \
	\
	error name##s_push(name *self, type0 key, type1 value, const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("key", check0(&key)); \
			errors_abort("value", check1(&value)); \
		} \
		\
		type2 item = {.key=key, .value=value}; \
		name##_bynary_node *new_bynary_node = null; \
		bynary_nodes_make(new_bynary_node, item, hasher0(&item.key), mem); \
		\
		error push_error = bynary_nodes_push((bynary_node **)&self->data, (bynary_node *)new_bynary_node); \
		if (push_error) { \
			name##s_free_private(new_bynary_node, mem); \
		} else { \
			self->size++; \
		} \
		\
		return push_error; \
	} \
	\
	void name##s_free(name *self, const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("self.data", bynary_nodes_check((const bynary_node *)self->data)); \
		} \
		\
		name##s_free_all_private(self->data, mem, 0); \
	} \
	\
	size_t name##s_frequency(name *self, type0 item) { \
		if (cels_debug) { \
			errors_abort("self.data", bynary_nodes_check((const bynary_node *)self->data)); \
			errors_abort("item", check0(&item)); \
		} \
		\
		return bynary_nodes_get_frequency((bynary_node *)self->data, hasher0(&item)); \
	} \
	\
	void name##s_traverse(name *self, callfunc callback) { \
		if (cels_debug) { \
			errors_abort("self.data", bynary_nodes_check((const bynary_node *)self->data)); \
		} \
		\
		bynary_nodes_traverse((bynary_node *)self->data, callback); \
	} \
	\
	void name##s_print(const name *self) { \
		if (cels_debug) { \
			errors_abort("self.data", bynary_nodes_check((const bynary_node *)self->data)); \
		} \
		\
		bynary_nodes_traverse((bynary_node *)self->data, (callfunc)name##s_print_private); \
		if (self->size > 0) { \
			putchar('\b'); \
			putchar('\b'); \
			putchar(' '); \
		} \
	} \
	\
	void name##s_println(const name *self) { \
		if (cels_debug) { \
			errors_abort("self.data", bynary_nodes_check((const bynary_node *)self->data)); \
		} \
		\
		bynary_nodes_traverse((bynary_node *)self->data, (callfunc)name##s_println_private); \
	} \
	\
	void name##s_debug(const name *self) { \
		if (cels_debug) { \
			errors_abort("self.data", bynary_nodes_check((const bynary_node *)self->data)); \
		} \
		\
		printf("<"#name">{.size: %zu, .data: {", self->size); \
		bynary_nodes_traverse((bynary_node *)self->data, (callfunc)name##s_debug_private); \
		if (self->size > 0) { \
			putchar('\b'); \
			putchar('\b'); \
		} \
		printf("}}"); \
	} \
	\
	void name##s_debugln(const name *self) { \
		if (cels_debug) { \
			errors_abort("self.data", bynary_nodes_check((const bynary_node *)self->data)); \
		} \
		\
		name##s_debug(self); \
		printf("\n"); \
	} \

/*
 * A macro-template code-generator to create 
 * set's definitions for a certain type.
 *
 * #to-review
 */
#define maps_generate_definition(type0, type1, type2, name) \
	typedef key_pairs(type0, type1) type2; \
	typedef struct name##_bynary_node name##_bynary_node; \
	typedef bynary_nodes(type2, name##_bynary_node) name##_bynary_node; \
	typedef bynary_trees(name##_bynary_node) name; \
	\
	name name##s_init(void); \
	\
	error name##s_push(name *self, type0 key, type1 value, const allocator *mem); \
	\
	cels_warn_unused \
	type1 *name##s_get(const name *self, type0 item); \
	\
	cels_warn_unused \
	size_t name##s_frequency(name *self, type0 item); \
	\
	void name##s_traverse(name *self, callfunc callback); \
	\
	void name##s_print(const name *self); \
	\
	void name##s_println(const name *self); \
	\
	void name##s_debug(const name *self); \
	\
	void name##s_debugln(const name *self); \
	\
	void name##s_free(name *self, const allocator *mem);

/* nodes and trees */

/*#to-deprecate*/
typedef struct node node;
typedef struct node_set node_set;

#define nodes(self, type0, type1) \
	struct self { \
		type0 *next; \
		type1 data; \
	}

typedef nodes(node, node_set, void *) node;
sets(node, node_set);

/* linked-nodes */

#define lnodes(name, type0) \
	struct name { \
		name *next; \
		type0 *data; \
	}

/* pools and linked-blocks */

#define block_items(name, type0) \
	struct name { \
		int status; \
		type0 data; \
	}

#define blocks(name, type0) \
	struct name { \
		name *next; \
		type0 *data; \
		size_t size; \
		size_t capacity; \
	}

#define pools(type0) \
	struct { \
		size_t size; \
		size_t capacity; \
		type0 data; \
	}

#define pool_iterators(type0, type1) \
	struct { \
		type0 *next; \
		type1 *data; \
		size_t index; \
	}

#define pools_generate_definition(name, type0) \
	typedef struct name##_block name##_block; \
	typedef block_items(name##_item, type0) name##_item; \
	typedef blocks(name##_block, name##_item) name##_block; \
	\
	typedef pools(name##_block *) name; \
	typedef pool_iterators(name##_block, type0) name##_iterator; \
	\
	/*
	 * Initializes a pool where each block has 
	 * capacity 'capacity' before appealing for
	 * a next block creation of same size.
	 *
	 * #to-review
	 */ \
	name name##s_init(size_t capacity, const allocator *mem); \
	\
	/*
	 * Pushes 'item' to 'self' pool at 
	 * first hole found.
	 *
	 * #to-review
	 */ \
	error name##s_push(name *self, type0 item, const allocator *mem); \
	\
	/*
	 * Iterates through 'self', yielding 
	 * non-empty variables.
	 *
	 * Iterator must be initialized to '{0}';
	 *
	 * #to-review
	 */ \
	bool name##s_next(name *self, name##_iterator *iterator); \
	\
	/*
	 * Frees 'self' and its data.
	 *
	 * #to-review
	 */ \
	void name##s_free(name *self, const allocator *mem);

#define pools_generate_implementation(name, type0, cleanup0) \
	name##_block *name##_blocks_init_private(size_t capacity, const allocator *mem) { \
		name##_item *items = mems_alloc(mem, capacity * sizeof(name##_item)); \
		if (!items) { return null; } \
		\
		for (size_t i = 0; i < capacity; i++) { \
			items[i].status = 0; \
		} \
		\
		name##_block node = { \
			.next=null, \
			.data=items, \
			.capacity=capacity, \
			.size=0, \
		}; \
		\
		name##_block *node_capsule = mems_alloc(mem, sizeof(name##_block)); \
		*node_capsule = node; \
		\
		return node_capsule; \
	} \
	\
	name name##s_init(size_t capacity, const allocator *mem) { \
		name##_block *block = name##_blocks_init_private(capacity, mem); \
		errors_abort("block", !block); \
		\
		name pool = { \
			.data=block, \
			.capacity=capacity, \
		}; \
		\
		return pool; \
	} \
	\
	error name##s_push(name *self, type0 item, const allocator *mem) { \
		if (!self || !self->capacity) { return fail; } \
		\
		name##_block *node = self->data; \
		while (node) { \
			for (size_t i = 0; i < node->capacity; i++) { \
				if (node->data[i].status == 0) { \
					node->data[i].data = item; \
					node->data[i].status = 1; \
					return ok; \
				} \
			} \
			\
			if (!node->next) { \
				name##_block *nodes = name##_blocks_init_private(self->data->capacity, mem); \
				if (!nodes) { return fail; } \
				\
				node->next = nodes; \
				node->next->data[0].data = item; \
				node->next->data[0].status = 1; \
				return ok; \
			} \
			\
			node = node->next; \
		} \
		\
		return ok; \
	} \
	\
	bool name##s_next(name *self, name##_iterator *iterator) { \
		if (!self) { return false; } \
		\
		if (!iterator->next) { \
			iterator->next = self->data; \
			iterator->index = 0; \
		} \
		\
		while (iterator->next) { \
			for (size_t i = iterator->index; i < iterator->next->capacity; i++) { \
				if (iterator->next->data[i].status > 0) { \
					iterator->data = &iterator->next->data[i].data; \
					iterator->index = ++i; \
					return true; \
				} \
			} \
			\
			iterator->next = iterator->next->next; \
			iterator->index = 0; \
		} \
		\
		return false; \
	} \
	\
	void name##s_free(name *self, const allocator *mem) { \
		if (!self) { return; } \
		\
		name##_block *node = self->data; \
		while (node) { \
			for (size_t i = 0; i < node->capacity; i++) { \
				if (node->data[i].status != 0) { \
					cleanup0(&node->data[i].data, mem); \
				} \
			} \
			\
			mems_dealloc(mem, node->data, node->capacity * sizeof(name##_item)); \
			\
			name##_block *prev = node; \
			node = node->next; \
			\
			mems_dealloc(mem, prev, sizeof(name##_block)); \
		} \
		\
		self->data = null; \
		self->size = 0; \
		self->capacity = 0; \
	} \

#endif
