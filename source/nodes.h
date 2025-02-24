#ifndef cels_nodes_h
#define cels_nodes_h

#include <stddef.h>
#include <stdint.h>
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

#define bynary_tree_iterators(type0) \
	struct { \
		type0 *data; \
		bynary_tree_iterator_internal internal; \
	}

typedef struct bynary_node bynary_node;

typedef enum bynary_node_color {
	bynary_node_red_color,
	bynary_node_black_color,
} bynary_node_color;

typedef enum bynary_tree_iterator_state {
	bynary_tree_initial_iterator_state,
	bynary_tree_left_most_iterator_state,
	bynary_tree_right_most_iterator_state,
	bynary_tree_returning_node_iterator_state,
	bynary_tree_finished_iterator_state,
} bynary_tree_iterator_state;

typedef struct bynary_tree_iterator_internal {
	bynary_node *left;
	bynary_node *right;
	bynary_node *prev;
	bynary_tree_iterator_state state;
} bynary_tree_iterator_internal;

typedef bynary_nodes(void *, bynary_node) bynary_node;
typedef bynary_trees(bynary_node) bynary_tree;
typedef bynary_tree_iterators(bynary_node) bynary_tree_iterator;

/*
 * Checks if bynary_node is correct.
 *
 * #to-review
 */
bool bynary_nodes_check(const bynary_node *self);

/*
 * Pushes 'item' onto 'self'. 
 * 'self' must be bynary-tree-like whereas 
 * 'item' should be bynary-node-like. 
 *
 * The sizes of the types used must be provided also; 
 * 'type_size' being the size of the underlying type 
 * being hold by the bynary-node 'item', while 
 * 'node_size' is the size of 'item' itself.
 *
 * A hash must be provided to identify uniquely such 
 * data being hold.
 *
 * If function errors to push 'item' 
 * (and, thus, ownership) error will be 
 * set to 'fail' or 1. 
 *
 * #to-review
 */
error bynary_trees_push(void *self, void *item, size_t hash, size_t type_size, size_t node_size, const allocator *mem);

/*
 * Gets node with hash, if it fails or it 
 * doesn't find it, it returns null.
 *
 * 'self' must be a bynary-tree-like structure.
 *
 * Returns a bynary-node-like structure if found 
 * else null.
 *
 * #to-review
 */
void* bynary_trees_get(const void *self, size_t hash);

/*
 * Iterates self in-order executing callback.
 *
 * 'self' must be a bynary-tree-like structure.
 *
 * #to-review
 */
bool bynary_trees_next(const void *self, bynary_tree_iterator *iterator);

/* multiary_nodes and multiary_trees */

/*
 * A multiary node data-structure to 
 * model hierarchy and branching.
 */
#define multiary_nodes(name, type0) \
	struct name { \
		name *parent; \
		name *left; \
		name *down; \
		type0 data; \
	}

#define multiary_trees(type0) \
	struct { \
		type0 *data; \
		size_t size; \
	}

#define multiary_tree_iterators(type0) \
	struct { \
		type0 *data; \
		multiary_tree_iterator_internal internal; \
	}

typedef struct multiary_node multiary_node;
typedef multiary_nodes(multiary_node, void *) multiary_node;
typedef multiary_trees(multiary_node) multiary_tree;

typedef struct multiary_tree_iterator_internal {
	multiary_node *current;
	/* for breadth-iterator */
	uintptr_t *branches;
	size_t size;
	size_t capacity;
	size_t cursor;
} multiary_tree_iterator_internal;

typedef multiary_tree_iterators(multiary_node)  multiary_tree_iterator;

/*
 * Initializes a multiary-tree.
 */
multiary_tree multiary_trees_init();

/*
 * Pushes item in to node within 'self' tree.
 */
error multiary_trees_push(multiary_tree *self, multiary_node *node, multiary_node *item);

/*
 * Attaches item in to node within 'self' tree.
 */
error multiary_trees_attach(multiary_tree *self, multiary_node *node, multiary_node *item);

/*
 * Iterates through 'self' pre-order.
 */
bool multiary_trees_next(multiary_tree *self, multiary_tree_iterator *iterator);

/*
 * Iterates through 'self' breadth-wise.
 */
bool multiary_trees_next_breadth_wise(multiary_tree *self, multiary_tree_iterator *iterator, const allocator *mem);

/* sets*/

#define sets(name, type0) \
	typedef struct name##_node name##_node; \
	typedef bynary_nodes(type0, name##_node) name##_node; \
	typedef bynary_trees(name##_node) name; \
	typedef bynary_tree_iterators(name) name##_iterator;

#define set_sizes_make(set) \
	{.node_size=sizeof(*set.data), .type_size=sizeof(set.data->data)}

typedef struct set_size {
	size_t node_size;
	size_t type_size;
} set_size;

/*
 * Iterates through set.
 *
 * 'self' must be a set-like structure whereas 
 * 'iterator' must be set-iterator-like.
 *
 * If eligible to continue, it returns true.
 *
 * #to-review
 */
bool sets_next(const void *self, void *iterator);

/*
 * Gets item from set provided item's hash.
 *
 * 'self' must be a set-like structure.
 *
 * Returns a set-node-like structure if 
 * found else null.
 *
 * #to-review
 */
void *sets_get(const void *self, size_t hash);

/*
 * Pushes item to set.
 *
 * 'self' must be a set-like structure whereas 
 * 'item' must be set-node-like.
 *
 * A hash must be provided to uniquely identify 
 * the 'item' being pushed.
 *
 * A set-size with sizes must be provided - it 
 * can be obtained with set-sizes-make.
 *
 * If function errors to push 'item' 
 * (and, thus, ownership) error will be 
 * set to 'fail' or 1. 
 *
 * #to-review
 */
error sets_push(void *self, void *item, size_t hash, set_size size, const allocator *mem);

/* maps */

#define map_pairs(type0, type1) struct { \
	type0 key; \
    type1 value; \
}

#define maps(name, type0, type1) \
	typedef map_pairs(type0, type1) name##_pair; \
	typedef struct name##_node name##_node; \
	typedef bynary_nodes(name##_pair, name##_node) name##_node; \
	typedef bynary_trees(name##_node) name; \
	typedef bynary_tree_iterators(name##_node) name##_iterator;

#define map_sizes_make(map) \
	{.node_size=sizeof(*map.data), .pair_size=sizeof(map.data->data)}

typedef struct map_size {
	size_t node_size;
	size_t pair_size;
} map_size;

/*
 * Iterates through map.
 *
 * 'self' must be a map-like structure whereas 
 * 'iterator' must be map-iterator-like.
 *
 * If eligible to continue, it returns true.
 *
 * #to-review
 */
bool maps_next(const void *self, void *iterator);

/*
 * Gets item from map provided item's hash.
 *
 * 'self' must be a map-like structure.
 *
 * Returns map-node-like structure if found 
 * else null.
 *
 * #to-review
 */
void *maps_get(const void *self, size_t hash);

/*
 * Pushes item to map.
 *
 * 'self' must be a map-like structure whereas 
 * 'item' must be map-node-like.
 *
 * A hash must be provided to uniquely identify 
 * the 'item' being pushed.
 *
 * A map-size with sizes must be provided - it 
 * can be obtained with map-sizes-make.
 *
 * If function errors to push 'item' 
 * (and, thus, ownership) error will be 
 * map to 'fail' or 1. 
 *
 * #to-review
 */
error maps_push(void *self, void *item, size_t hash, map_size size, const allocator *mem);

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

/* macro-generators */

/*
 * Defines aliases of a multiary-tree for 'name' and 'type'.
 */
#define trees_define(name, type) \
	typedef struct name##_node name##_node; \
	multiary_nodes(name##_node, type); \
	typedef multiary_trees(name##_node) name; \
	typedef multiary_tree_iterators(name##_node) name##_iterator; \
	\
	cels_warn_unused \
	name name##s_init(void); \
	\
	error name##s_push(name *self, name##_node *node, name##_node *item); \
	\
	error name##s_attach(name *self, name##_node *node, name##_node *item); \
	\
	bool name##s_check(const name *self); \
	\
	void name##s_free(name *self, const allocator *mem); \
	\
	bool name##s_next(const name *self, name##_iterator *iterator); \
	\
	bool name##s_next_breadth_wise(const name *self, name##_iterator *iterator, const allocator *mem);

/*
 * Generates the basic functions of 'trees_define'
 */
#define trees_generate(name, type, checker, freer) \
	name name##s_init(void) { return (name){0}; } \
	\
	error name##s_push(name *self, name##_node *node, name##_node *item) { \
		return multiary_trees_push( \
			(multiary_tree *)self, (multiary_node *)node, (multiary_node *)item); \
	} \
	\
	error name##s_attach(name *self, name##_node *node, name##_node *item) { \
		return multiary_trees_attach( \
			(multiary_tree *)self, (multiary_node *)node, (multiary_node *)item); \
	} \
	\
	bool name##s_next(const name *self, name##_iterator *iterator) { \
		return multiary_trees_next( \
			(multiary_tree *)self, (multiary_tree_iterator *)iterator); \
	} \
	\
	bool name##s_next_breadth_wise(const name *self, name##_iterator *iterator, const allocator *mem) { \
		return multiary_trees_next_breadth_wise( \
			(multiary_tree *)self, (multiary_tree_iterator *)iterator, mem); \
	} \
	\
	bool name##s_check(const name *self) { \
		name##_iterator it = {0}; \
		while(name##s_next(self, &it)) { \
			if (cels_debug) { errors_return("self.data[i].data", checker(&it.data->data)); } \
			if (checker(&it.data->data)) { return true; } \
		} \
		return false; \
	} \
	\
	void name##s_free(name *self, const allocator *mem) { \
		name##_iterator it = {0}; \
		while(name##s_next_breadth_wise(self, &it, mem)) { \
			if (cels_debug) { errors_abort("self.data[i].data", checker(&it.data->data)); } \
			freer(&it.data->data, mem); \
			mems_dealloc(mem, it.data, sizeof(name##_node)); \
		} \
	}

/*
 * A macro-template code-generator to create 
 * set's definitions for a certain type.
 *
 * #to-review
 */
#define sets_define(name, type) \
	typedef struct name##_node name##_node; \
	typedef bynary_nodes(type, name##_node) name##_node; \
	typedef bynary_trees(name##_node) name; \
	typedef bynary_tree_iterators(name##_node) name##_iterator; \
	\
	name name##s_init(void); \
	\
	cels_warn_unused \
	type *name##s_get(const name *self, type item); \
	\
	bool name##s_push(name *self, type item, const allocator *mem); \
	\
	void name##s_free(name *self, const allocator *mem); \
	\
	void name##s_print(const name *self); \
	\
	void name##s_println(const name *self); \
	\
	void name##s_debug(const name *self); \
	\
	void name##s_debugln(const name *self);

/*
 * A macro-template code-generator to create 
 * set's functions for a certain type.
 *
 * #to-review
 */
#define sets_generate(name, type, checker, printer, hasher, cleaner) \
	name name##s_init(void) { return (name){0}; } \
	\
	type *name##s_get(const name *self, type item) { \
		return sets_get(self, hasher(&item)); \
	} \
	\
	bool name##s_push(name *self, type item, const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("self.data", bynary_nodes_check((const bynary_node *)self->data)); \
			errors_abort("item", checker(&item)); \
		} \
		\
		set_size sizes = {.node_size = sizeof(name##_node), .type_size = sizeof(type)}; \
		error push_error = sets_push(self, &item, hasher(&item), sizes, mem); \
		if (push_error) { \
			cleaner(&item, mem); \
			return push_error; \
		} \
		self->size++; \
		return ok; \
	} \
	\
	void name##s_free(name *self, const allocator *mem) { \
		name##_iterator it = {0}; \
		while (sets_next(self, &it)) { \
			cleaner(&it.data->data, mem); \
			mems_dealloc(mem, it.data, sizeof(name##_node)); \
		} \
	} \
	void name##s_print(const name *self) { \
		name##_iterator it = {0}; \
		while (sets_next(self, &it)) { \
			printer(&it.data->data); \
			printf(", "); \
		} \
		if (self->size > 0) { \
			putchar('\b'); \
			putchar('\b'); \
			putchar(' '); \
		} \
	} \
	\
	void name##s_println(const name *self) { \
		name##_iterator it = {0}; \
		while (sets_next(self, &it)) { \
			printer(&it.data->data); \
			printf("\n"); \
		} \
	} \
	\
	void name##s_debug(const name *self) { \
		printf("<"#name">{.size: %zu, .data: {", self->size); \
		name##_iterator it = {0}; \
		while (sets_next(self, &it)) { \
			printer(&it.data->data); \
			printf(", "); \
		} \
		if (self->size > 0) { \
			putchar('\b'); \
			putchar('\b'); \
		} \
		printf("}}"); \
	} \
	\
	void name##s_debugln(const name *self) { \
		name##s_debug(self); \
		printf("\n"); \
	}

/*
 * A macro-template code-generator to create 
 * set's definitions for a certain type.
 *
 * #to-review
 */
#define maps_define(name, type0, type1) \
	typedef map_pairs(type0, type1) name##_pair; \
	typedef struct name##_node name##_node; \
	typedef bynary_nodes(name##_pair, name##_node) name##_node; \
	typedef bynary_trees(name##_node) name; \
	typedef bynary_tree_iterators(name##_node) name##_iterator; \
	\
	name name##s_init(void); \
	\
	cels_warn_unused \
	type1 *name##s_get(const name *self, type0 item); \
	\
	error name##s_push(name *self, type0 key, type1 value, const allocator *mem); \
	\
	void name##s_free(name *self, const allocator *mem); \
	\
	void name##s_print(const name *self); \
	\
	void name##s_println(const name *self); \
	\
	void name##s_debug(const name *self); \
	\
	void name##s_debugln(const name *self);

/*
 * A macro-template code-generator to create 
 * set's functions for a certain type.
 *
 * #to-review
 */
#define maps_generate( \
	name, type0, type1, checker0, checker1, printer0, printer1, hasher0, cleaner0, cleaner1) \
	name name##s_init(void) { \
		return (name){.data=null, .size=0}; \
	} \
	\
	type1 *name##s_get(const name *self, type0 item) { \
		name##_pair *pair = maps_get(self, hasher0(&item)); \
		return !pair ? null : &pair->value; \
	} \
	\
	error name##s_push(name *self, type0 key, type1 value, const allocator *mem) { \
		if (cels_debug) { \
			errors_abort("key", checker0(&key)); \
			errors_abort("value", checker1(&value)); \
		} \
		\
		map_size sizes = {.node_size=sizeof(name##_node), .pair_size=sizeof(name##_pair)}; \
		name##_pair pair = {.key=key, .value=value}; \
		error push_error = maps_push(self, &pair, hasher0(&key), sizes, mem); \
		if (push_error) { \
			cleaner0(&key, mem); \
			cleaner1(&value, mem); \
			return push_error; \
		} \
		\
		self->size++; \
		return ok; \
	} \
	\
	void name##s_free(name *self, const allocator *mem) { \
		name##_iterator it = {0}; \
		while (maps_next(self, &it)) { \
			cleaner0(&it.data->data.key, mem); \
			cleaner1(&it.data->data.value, mem); \
			\
			if (it.internal.prev) { \
				mems_dealloc(mem, it.internal.prev, sizeof(name##_node)); \
			} \
			if (it.data == self->data) { \
				mems_dealloc(mem, it.data, sizeof(name##_node)); \
			} \
		} \
	} \
	\
	void name##s_print(const name *self) { \
		name##_iterator it = {0}; \
		while (maps_next(self, &it)) { \
			printer0(&it.data->data.key); \
			printf(": "); \
			printer1(&it.data->data.value); \
			printf(", "); \
		} \
		if (self->size > 0) { \
			putchar('\b'); \
			putchar('\b'); \
			putchar(' '); \
		} \
	} \
	\
	void name##s_println(const name *self) { \
		name##_iterator it = {0}; \
		while (maps_next(self, &it)) { \
			printer0(&it.data->data.key); \
			printf(": "); \
			printer1(&it.data->data.value); \
			printf("\n"); \
		} \
	} \
	\
	void name##s_debug(const name *self) { \
		if (cels_debug) { errors_abort("!self", !self); } \
		\
		printf("<"#name">{.size: %zu, .data: {", self->size); \
		name##_iterator it = {0}; \
		while (maps_next(self, &it)) { \
			printf("\""); \
			printer0(&it.data->data.key); \
			printf(": "); \
			printer1(&it.data->data.value); \
			printf("\", "); \
		} \
		if (self->size > 0) { \
			putchar('\b'); \
			putchar('\b'); \
		} \
		printf("}}"); \
	} \
	\
	void name##s_debugln(const name *self) { \
		name##s_debug(self); \
		printf("\n"); \
	}

/*#to-deprecate*/
typedef struct node node;
typedef struct node_set node_set;

#define nodes(self, type0, type1) \
	struct self { \
		type0 *next; \
		type1 data; \
	}

typedef nodes(node, node_set, void *) node;
bynary_nodes(node, node_set);

/* linked-nodes */

#define lnodes(name, type0) \
	struct name { \
		name *next; \
		type0 *data; \
	}

#define pools_define(name, type0) \
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

#define pools_generate(name, type0, cleanup0) \
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
