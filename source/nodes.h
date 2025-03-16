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


/* binodes and bitrees */

/*
 * A binary node data-structure to 
 * serve as basis for a binary search 
 * algorythm implementation, 
 * know as red-black tree.
 */
#define binodes(type0, type1) \
	struct type1 { \
		binode_color color; \
		size_t hash; \
		ulong frequency; \
		type1 *parent; \
		type1 *right; \
		type1 *left; \
		type0 data; \
	}

#define bitrees(type0) \
	struct { \
		size_t size; \
		size_t type_size; \
		size_t node_size; \
		size_t extra_size; \
		type0 *data; \
	}

#define bitree_iterators(type0) \
	struct { \
		type0 *data; \
		bitree_iterator_internal internal; \
	}

typedef struct binode binode;

typedef enum binode_color {
	binode_red_color,
	binode_black_color,
} binode_color;

typedef enum bitree_iterator_state {
	bitree_initial_iterator_state,
	bitree_left_most_iterator_state,
	bitree_right_most_iterator_state,
	bitree_returning_node_iterator_state,
	bitree_finished_iterator_state,
} bitree_iterator_state;

typedef struct bitree_iterator_internal {
	binode *left;
	binode *right;
	binode *prev;
	bitree_iterator_state state;
} bitree_iterator_internal;

typedef binodes(void *, binode) binode;
typedef bitrees(binode) bitree;
typedef bitree_iterators(binode) bitree_iterator;

/*
 * Checks if binode is correct.
 *
 * #to-review
 */
bool binodes_check(const binode *self);

/*
 * Pushes 'item' onto 'self'. 
 * 'self' must be bitree-like whereas 
 * 'item' should be binode-like. 
 *
 * The sizes of the types used must be provided also; 
 * 'type_size' being the size of the underlying type 
 * being hold bi the binode 'item', while 
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
error bitrees_push(
	void *self, void *item, size_t hash, const allocator *mem);

/*
 * Gets node with hash, if it fails or it 
 * doesn't find it, it returns null.
 *
 * 'self' must be a bitree-like structure.
 *
 * Returns a binode-like structure if found 
 * else null.
 *
 * #to-review
 */
void* bitrees_get(const void *self, size_t hash);

/*
 * Iterates self in-order executing callback.
 *
 * 'self' must be a bitree-like structure and 
 * 'iterator' a bitree-iterator-like one.
 *
 * #to-review
 */
bool bitrees_next(const void *self, void *iterator);


/* munodes and mutrees */

/*
 * A multiary node data-structure to 
 * model hierarchy and branching.
 */
#define munodes(name, type0) \
	struct name { \
		name *parent; \
		name *left; \
		name *down; \
		type0 data; \
	}

#define mutrees(type0) \
	struct { \
		type0 *data; \
		size_t size; \
		size_t node_size; \
	}

#define mutree_iterators(type0) \
	struct { \
		type0 *data; \
		mutree_iterator_internal internal; \
	}

typedef struct munode munode;
typedef munodes(munode, void *) munode;
typedef mutrees(munode) mutree;

typedef struct mutree_iterator_internal {
	munode *current;
	/* for breadth-iterator */
	uintptr_t *branches;
	size_t size;
	size_t capacity;
	size_t cursor;
} mutree_iterator_internal;

typedef mutree_iterators(munode) mutree_iterator;

/*
 * Initializes mutree.
 *
 * #to-review
 */
#define mutrees_init(self) { \
	self.node_size = sizeof(*self.data); \
}

/*
 * Pushes item in to node within 'self' tree.
 *
 * 'self' must be a mutree-like structure, 
 * while 'node' and 'item' shall be a 
 * munode-like one.
 *
 * #to-review
 */
error mutrees_push(void *self, void *node, void *item);

/*
 * Attaches item in to node within 'self' tree.
 *
 * 'self' must be a mutree-like structure, 
 * while 'node' and 'item' shall be a 
 * munode-like one.
 *
 * #to-review
 */
error mutrees_attach(void *self, void *node, void *item);

/*
 * Iterates through 'self' pre-order.
 *
 * 'self' must be a mutree-like structure, 
 * while 'iterator' shall be a 
 * mutree-iterator-like one.
 *
 * #to-review
 */
bool mutrees_next(const void *self, void *iterator);

/*
 * Iterates through 'self' breadth-wise.
 *
 * 'self' must be a mutree-like structure, 
 * while 'iterator' shall be a 
 * mutree-iterator-like one.
 *
 * #to-review
 */
bool mutrees_next_breadth_wise(
	const void *self, void *iterator, const allocator *mem);

/*
 * Frees mutree.
 *
 * 'self' must be a mutree-like structure.
 *
 * A 'cleaner' function may be provided to free underlying data.
 *
 * #to-review
 */
void mutrees_free(
	void *self, freefunc cleaner, const allocator *mem);


/* sets*/

#define sets(name, type0) \
	typedef struct name##_node name##_node; \
	typedef binodes(type0, name##_node) name##_node; \
	typedef bitrees(name##_node) name; \
	typedef bitree_iterators(name) name##_iterator;

/*
 * Initializes map.
 *
 * #to-review
 */
#define sets_init(self) { \
	self.node_size = sizeof(*self.data); \
	self.type_size = sizeof(self.data[0].data); \
}

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
 * Prints 'self' using printer.
 *
 * 'self' must be a set-like structure.
 *
 * #to-review
 */
void sets_print(const void *self, printfunc printer);

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
 * If function errors to push 'item' 
 * (and, thus, ownership) error will be 
 * set to 'fail' or 1. 
 *
 * #to-review
 */
error sets_push(
	void *self, void *item, size_t hash, const allocator *mem);

/*
 * Frees set.
 *
 * 'self' must be a set-like structure.
 *
 * a cleaner may be provided to free the underlying data.
 *
 * #to-review
 */
void sets_free(void *self, freefunc cleaner, const allocator *mem);


/* maps */

#define map_pairs(type0, type1) struct { \
	type0 key; \
    type1 value; \
}

#define maps(name, type0, type1) \
	typedef map_pairs(type0, type1) name##_pair; \
	typedef struct name##_node name##_node; \
	typedef binodes(name##_pair, name##_node) name##_node; \
	typedef bitrees(name##_node) name; \
	typedef bitree_iterators(name##_node) name##_iterator;

/*
 * Initializes map.
 *
 * #to-review
 */
#define maps_init(self) { \
	self.node_size = sizeof(*self.data); \
	self.type_size = sizeof(self.data[0].data); \
	self.extra_size = (size_t)( \
		(char *)&self.data[0].data.value - \
		(char *)&self.data[0].data \
	); \
}

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
 * Prints 'self' using 'key_printer'
 * and 'value_printer'.
 *
 * 'self' must be a set-like structure.
 *
 * #to-review
 */
void maps_print(
	const void *self, 
	printfunc key_printer, 
	printfunc value_printer);

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
 * If function errors to push 'item' 
 * (and, thus, ownership) error will be 
 * map to 'fail' or 1. 
 *
 * #to-review
 */
error maps_push(
	void *self, void *item, size_t hash, const allocator *mem);

/*
 * Frees map.
 *
 * 'self' must be a set-like structure.
 *
 * A 'key_cleaner' and 'value_cleaner' may be 
 * provided to free the underlying data.
 *
 * #to-review
 */
void maps_free(
	void *self, 
	freefunc key_cleaner, 
	freefunc value_cleaner, 
	const allocator *mem);


/* pools and linked-blocks */

#define pool_block_items(name, type0) \
	struct name { \
		int status; \
		type0 data; \
	}

#define pool_blocks(name, type0) \
	struct name { \
		name *next; \
		type0 *data; \
		size_t size; \
		size_t capacity; \
	}

#define poolls(type0) \
	struct { \
		size_t size; \
		size_t capacity; \
		type0 *data; \
		size_t item_size; \
		size_t type_size; \
		size_t offset_size; \
	}

#define pool_iterators(type0, type1) \
	struct { \
		type0 *next; \
		type1 *data; \
		char *current; \
		char *end; \
	}

#define pools(name, type0) \
	typedef struct name##_block name##_block; \
	typedef pool_block_items(name##_block_item, type0) name##_block_item; \
	typedef pool_blocks(name##_block, name##_block_item) name##_block; \
	typedef poolls(name##_block) name; \
	typedef pool_iterators(name##_block, type0) name##_iterator; \

typedef struct pool_block pool_block;
typedef pool_block_items(pool_block_item, void *) pool_block_item;
typedef pool_blocks(pool_block, pool_block_item) pool_block;
typedef poolls(pool_block) pool;
typedef pool_iterators(pool_block, void) pool_iterator;

/*
 * Initializes a pool with capacity.
 *
 * 'self' must be a pool-like structure.
 *
 * 'item-size' refers to the size of the 
 * underlying pool-block-item-like data;
 *
 * 'type-size' refers to the size of the 
 * underlying type;
 *
 * 'offset-size' refers to the offset size 
 * of the pool-block-item-like data untill the type.
 *
 * #to-review
 */
void pools_init(
	void *self, 
	size_t item_size, 
	size_t type_size, 
	size_t offset_size, 
	size_t capacity, 
	const allocator *mem);

/*
 * Pushes item to pool.
 *
 * 'self' must be a pool-like structure, while 'item' 
 * shall be a pointer for the underlying type.
 *
 * #to-review
 */
error pools_push(
	void *self, void *item, const allocator *mem);

/*
 * Pushes item to a pool-block.
 *
 * 'self' must be a pool-like structure, while 'item' 
 * shall be a pointer for the underlying type.
 * 
 * If 'n' block doesn't exist, it allocates 
 * one until it arrives at that index.
 *
 * #to-review
 */
error pools_push_to(
	void *self, void *item, size_t n, const allocator *mem);

/*
 * Iterates through pool.
 *
 * 'self' must be a pool-like structure, while 'iterator' 
 * shall be a pool-iterator-like one.
 *
 * #to-review
 */
bool pools_next(void *self, void *iterator);

/*
 * Iterates through a pool within a block.
 * 
 * 'self' must be a pool-like structure, while 'iterator' 
 * shall be a pool-iterator-like one.
 * 
 * 'n' must be a valid index of pool-block.
 *
 * #to-review
 */
bool pools_next_in(void *self, size_t n, void *iterator);

/*
 * Frees pool.
 *
 * 'self' must be a pool-like structure.
 *
 * A 'cleaner' function may be provided to free 
 * any underlying allocated data.
 *
 * #to-review
 */
void pools_free(
	void *self, freefunc cleaner, const allocator *mem);


/* #to-deprecate */

typedef struct node node;
typedef struct node_set node_set;

#define nodes(self, type0, type1) \
	struct self { \
		type0 *next; \
		type1 data; \
	}

typedef nodes(node, node_set, void *) node;
binodes(node, node_set);

#endif
