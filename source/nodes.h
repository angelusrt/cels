#ifndef cels_nodes_h
#define cels_nodes_h

#include <stddef.h>

#include "mems.h"
#include "errors.h"

/*
 * The module 'nodes' is a module 
 * that implements different types of 
 * nodes, linked-lists and trees - which 
 * are the basis for other data-structures 
 * like maps and sets.
 */

/* bnodes and btrees */

typedef enum bnode_color {
	bnode_red_color,
	bnode_black_color,
} bnode_color;

/*
 * A bynary node data-structure to 
 * serve as basis for a bynary search 
 * algorythm implementation, 
 * know as red-black tree.
 */
#define bnodes(type0, type1) \
	struct type1 { \
		bnode_color color; \
		size_t hash; \
		ulong frequency; \
		type1 *parent; \
		type1 *right; \
		type1 *left; \
		type0 data; \
	}

#define btrees(type0) \
	struct { \
		type0 *data; \
		size_t size; \
	}

typedef struct bnode bnode;
typedef bnodes(void *, bnode) bnode;
typedef btrees(bnode) btree;

/*
 * Makes a bynary-node of type of 'self' and from data 
 * item and hash id, which will be allocated to 
 * 'self' (a pointer).
 *
 * #allocates #may-panic #to-review
 */
#define bnodes_make(self, item, id, mem) { \
	typeof(*self) node = { \
		.hash=id, \
		.data=item, \
		.color=bnode_black_color, \
		.frequency=1}; \
	\
	self = mems_alloc(mem, sizeof(*self)); \
	errors_abort("self", !self); \
	*self = node; \
}

/*
 * Checks if bnode is correct.
 *
 * #to-review
 */
bool bnodes_check(const bnode *self);

/*
 * Pushes a new node to self. If self is null 
 * then new_bnode becomes root, thus being returned.
 *
 * If function errors to push new_bnode 
 * (and, thus, ownership) error will be 
 * set to true. Variable error may be set 
 * to null to silently error.
 *
 * #to-edit
 */
bool bnodes_push(bnode **self, bnode *new_node);

/*
 * Gets node with the same hash as hash, if 
 * it fails or it doesn't find it, it returns 
 * null.
 *
 * #to-review
 */
bnode* bnodes_get(bnode* self, size_t hash);

/*
 * Gets only the data inside node.
 *
 * #to-review
 */
void *bnodes_get_data(bnode *self, size_t hash);

/*
 * Gets only the frequency inside node.
 *
 * #to-review
 */
size_t bnodes_get_frequency(bnode *self, size_t hash);

/*
 * Traverses self in-order executing callback.
 *
 * #to-review
 */
void bnodes_traverse(bnode *self, callfunc callback);

/*
 * Iterates self in-order executing callback.
 *
 * #to-review
 */
void bnodes_iterate(bnode *self, enfunctor func);

/*
 * Traverses self in-order to free all nodes
 *
 * #to-review
 */
void bnodes_free_all(bnode *self, const allocator *mem, freefunc cleanup);

/*
 * Traverses self in-order to get length
 *
 * #to-review
 */
cels_warn_unused
size_t bnodes_length(bnode *self);

/* sets */

#define sets(type, name) bnodes(type, name) 

/*
 * A macro-template code-generator to create 
 * set's functions for a certain type.
 *
 * #to-review
 */
#define sets_generate_implementation(type, name, check0, print0, hasher0, cleanup0) \
	void name##s_free_private(name##_bnode *self, const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("self", bnodes_check((const bnode *)self)); \
		} \
		\
		cleanup0(&self->data, mem); \
		mems_dealloc(mem, self, sizeof(name##_bnode)); \
	} \
	\
	void name##s_free_all_private(name##_bnode *self, const allocator *mem, size_t stackframe) { \
		if (!self || stackframe > cels_max_recursion) { \
			return; \
		} \
		\
		name##_bnode *left = self->left; \
		name##_bnode *right = self->right; \
		\
		name##s_free_all_private(left, mem, ++stackframe); \
		name##s_free_private(self, mem); \
		name##s_free_all_private(right, mem, ++stackframe); \
	} \
	\
	void *name##s_print_private(const name##_bnode *self) { \
		print0(&self->data); \
		printf(", "); \
		return null; \
	} \
	\
	void *name##s_println_private(const name##_bnode *self) { \
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
			errors_abort("self.data", bnodes_check((const bnode *)self->data)); \
			errors_abort("item", check0(&item)); \
		} \
		\
		return (type *)bnodes_get_data((bnode *)self->data, hasher0(&item)); \
	} \
	\
	bool name##s_push(name *self, type item, const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("item", check0(&item)); \
		} \
		\
		name##_bnode *new_bnode = null; \
		bnodes_make(new_bnode, item, hasher0(&item), mem) \
		\
		bool push_error = bnodes_push((bnode **)&self->data, (bnode *)new_bnode); \
		if (push_error) { \
			name##s_free_private(new_bnode, mem); \
		} else { \
			self->size++; \
		}\
		\
		return push_error; \
	} \
	\
	void name##s_free(name *self, const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("self.data", bnodes_check((const bnode *)self->data)); \
		} \
		\
		name##s_free_all_private(self->data, mem, 0); \
	} \
	\
	void name##s_traverse(name *self, callfunc callback) { \
		if (cels_debug) { \
			errors_abort("self.data", bnodes_check((const bnode *)self->data)); \
		} \
		\
		bnodes_traverse((bnode *)self->data, callback); \
	} \
	\
	void name##s_print(const name *self) { \
		if (cels_debug) { \
			errors_abort("self.data", bnodes_check((const bnode *)self->data)); \
		} \
		\
		bnodes_traverse((bnode *)self->data, (callfunc)name##s_print_private); \
		if (self->size > 0) { \
			putchar('\b'); \
			putchar('\b'); \
			putchar(' '); \
		} \
	} \
	\
	void name##s_println(const name *self) { \
		if (cels_debug) { \
			errors_abort("self.data", bnodes_check((const bnode *)self->data)); \
		} \
		\
		bnodes_traverse((bnode *)self->data, (callfunc)name##s_println_private); \
	} \
	\
	void name##s_debug(const name *self) { \
		if (cels_debug) { \
			errors_abort("self.data", bnodes_check((const bnode *)self->data)); \
		} \
		\
		printf("<"#name">{.size: %zu, .data: {", self->size); \
		bnodes_traverse((bnode *)self->data, (callfunc)name##s_print_private); \
		if (self->size > 0) { \
			putchar('\b'); \
			putchar('\b'); \
		} \
		printf("}}"); \
	} \
	\
	void name##s_debugln(const name *self) { \
		if (cels_debug) { \
			errors_abort("self.data", bnodes_check((const bnode *)self->data)); \
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
	typedef struct name##_bnode name##_bnode; \
	typedef bnodes(type, name##_bnode) name##_bnode; \
	typedef btrees(name##_bnode) name; \
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

#define maps(type, name) bnodes(type, name) 

/*
 * A macro-template code-generator to create 
 * set's functions for a certain type.
 *
 * #to-review
 */
#define maps_generate_implementation( \
	type0, type1, type2, name, check0, check1, print0, print1, hasher0, cleanup0, cleanup1 \
) \
	void name##s_free_private(name##_bnode *self, const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("self", bnodes_check((const bnode *)self)); \
		} \
		\
		cleanup0(&self->data.key, mem); \
		cleanup1(&self->data.value, mem); \
		mems_dealloc(mem, self, sizeof(name)); \
		self = null; \
	} \
	\
	void name##s_free_all_private(name##_bnode *self, const allocator *mem, size_t stackframe) { \
		if (!self || stackframe > cels_max_recursion) { return; } \
		\
		name##_bnode *left = self->left; \
		name##_bnode *right = self->right; \
		\
		name##s_free_all_private(left, mem, ++stackframe); \
		name##s_free_private(self, mem); \
		name##s_free_all_private(right, mem, ++stackframe); \
	} \
	\
	void *name##s_print_private(const name##_bnode *self) { \
		print0(&self->data.key); \
		printf(": "); \
		print1(&self->data.value); \
		printf(", "); \
		return null; \
	} \
	\
	void *name##s_println_private(const name##_bnode *self) { \
		print0(&self->data.key); \
		printf(": "); \
		print1(&self->data.value); \
		printf("\n"); \
		return null; \
	} \
	\
	void *name##s_debug_private(const name##_bnode *self) { \
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
			errors_abort("self.data", bnodes_check((const bnode *)self->data)); \
			errors_abort("item", check0(&item)); \
		} \
		\
		type2 *temp = bnodes_get_data((bnode *)self->data, hasher0(&item)); \
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
		name##_bnode *new_bnode = null; \
		bnodes_make(new_bnode, item, hasher0(&item.key), mem); \
		\
		error push_error = bnodes_push((bnode **)&self->data, (bnode *)new_bnode); \
		if (push_error) { \
			name##s_free_private(new_bnode, mem); \
		} else { \
			self->size++; \
		} \
		\
		return push_error; \
	} \
	\
	void name##s_free(name *self, const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("self.data", bnodes_check((const bnode *)self->data)); \
		} \
		\
		name##s_free_all_private(self->data, mem, 0); \
	} \
	\
	size_t name##s_frequency(name *self, type0 item) { \
		if (cels_debug) { \
			errors_abort("self.data", bnodes_check((const bnode *)self->data)); \
			errors_abort("item", check0(&item)); \
		} \
		\
		return bnodes_get_frequency((bnode *)self->data, hasher0(&item)); \
	} \
	\
	void name##s_traverse(name *self, callfunc callback) { \
		if (cels_debug) { \
			errors_abort("self.data", bnodes_check((const bnode *)self->data)); \
		} \
		\
		bnodes_traverse((bnode *)self->data, callback); \
	} \
	\
	void name##s_print(const name *self) { \
		if (cels_debug) { \
			errors_abort("self.data", bnodes_check((const bnode *)self->data)); \
		} \
		\
		bnodes_traverse((bnode *)self->data, (callfunc)name##s_print_private); \
		if (self->size > 0) { \
			putchar('\b'); \
			putchar('\b'); \
			putchar(' '); \
		} \
	} \
	\
	void name##s_println(const name *self) { \
		if (cels_debug) { \
			errors_abort("self.data", bnodes_check((const bnode *)self->data)); \
		} \
		\
		bnodes_traverse((bnode *)self->data, (callfunc)name##s_println_private); \
	} \
	\
	void name##s_debug(const name *self) { \
		if (cels_debug) { \
			errors_abort("self.data", bnodes_check((const bnode *)self->data)); \
		} \
		\
		printf("<"#name">{.size: %zu, .data: {", self->size); \
		bnodes_traverse((bnode *)self->data, (callfunc)name##s_debug_private); \
		if (self->size > 0) { \
			putchar('\b'); \
			putchar('\b'); \
		} \
		printf("}}"); \
	} \
	\
	void name##s_debugln(const name *self) { \
		if (cels_debug) { \
			errors_abort("self.data", bnodes_check((const bnode *)self->data)); \
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
	typedef struct name##_bnode name##_bnode; \
	typedef bnodes(type2, name##_bnode) name##_bnode; \
	typedef btrees(name##_bnode) name; \
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

/* lists and linked-blocks */

#define litems(name, type0) \
	struct name { \
		int status; \
		type0 data; \
	}

#define lblocks(name, type0) \
	struct name { \
		name *next; \
		type0 *data; \
		size_t size; \
		size_t capacity; \
	}

#define lists(type0) \
	struct { \
		size_t size; \
		size_t capacity; \
		type0 data; \
	}

#define list_iterators(type0, type1) \
	struct { \
		type0 *next; \
		type1 *data; \
		size_t index; \
	}

#define lists_generate_definition(name, type0) \
	typedef struct name##_lblock name##_lblock; \
	typedef litems(name##_item, type0) name##_item; \
	typedef lblocks(name##_lblock, name##_item) name##_lblock; \
	\
	typedef lists(name##_lblock *) name; \
	typedef list_iterators(name##_lblock, type0) name##_iterator; \
	\
	/*
	 * Initializes a list where each block has 
	 * capacity 'capacity' before appealing for
	 * a next block creation of same size.
	 *
	 * #to-review
	 */ \
	name name##s_init(size_t capacity, const allocator *mem); \
	\
	/*
	 * Pushes 'item' to 'self' list at 
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

#define lists_generate_implementation(name, type0, cleanup0) \
	name##_lblock *name##_lblocks_init_private(size_t capacity, const allocator *mem) { \
		name##_item *items = mems_alloc(mem, capacity * sizeof(name##_item)); \
		if (!items) { return null; } \
		\
		for (size_t i = 0; i < capacity; i++) { \
			items[i].status = 0; \
		} \
		\
		name##_lblock node = { \
			.next=null, \
			.data=items, \
			.capacity=capacity, \
			.size=0, \
		}; \
		\
		name##_lblock *node_capsule = mems_alloc(mem, sizeof(name##_lblock)); \
		*node_capsule = node; \
		\
		return node_capsule; \
	} \
	\
	name name##s_init(size_t capacity, const allocator *mem) { \
		name##_lblock *block = name##_lblocks_init_private(capacity, mem); \
		errors_abort("block", !block); \
		\
		name list = { \
			.data=block, \
			.capacity=capacity, \
		}; \
		\
		return list; \
	} \
	\
	error name##s_push(name *self, type0 item, const allocator *mem) { \
		if (!self || !self->capacity) { return fail; } \
		\
		name##_lblock *node = self->data; \
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
				name##_lblock *nodes = name##_lblocks_init_private(self->data->capacity, mem); \
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
		name##_lblock *node = self->data; \
		while (node) { \
			for (size_t i = 0; i < node->capacity; i++) { \
				if (node->data[i].status != 0) { \
					cleanup0(&node->data[i].data, mem); \
				} \
			} \
			\
			mems_dealloc(mem, node->data, node->capacity * sizeof(name##_item)); \
			\
			name##_lblock *prev = node; \
			node = node->next; \
			\
			mems_dealloc(mem, prev, sizeof(name##_lblock)); \
		} \
		\
		self->data = null; \
		self->size = 0; \
		self->capacity = 0; \
	} \

#endif
