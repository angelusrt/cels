#ifndef bnodes_h
#define bnodes_h
#pragma once

#include <stddef.h>
#include "utils.h"

typedef enum bnodes_color {
	bnodes_red_color,
	bnodes_black_color,
} bnodes_color;

/*
 * A bynary node data structure to 
 * serve as basis for a bynary search 
 * algorythm implementation, 
 * know as red-black tree.
 */
typedef struct bnode {
	bnodes_color color;
	size_t hash;
	ulong frequency;
	struct bnode *parent;
	struct bnode *right;
	struct bnode *left;
	void *data;
} bnode;

/*bnode (bbdnode? => bynary-bi-directional-node) => bynary-node
mdnode? => multi-directional-node
bdnode? => bidirectional-node
mbdnode? => multi-bi-directional-node*/

#define bnodes(t0, t1) struct t1 { \
	bnodes_color color; \
	size_t hash; \
	ulong frequency; \
	typeof(t1) *parent; \
	typeof(t1) *right; \
	typeof(t1) *left; \
	typeof(t0) data; \
}

#define nodes_max_recursion 30

/*
 * Makes a bynary-node of type t and from data d and hash h, 
 * which will be allocated to new_bnode (a pointer)
 *
 * #allocates #may-panic #to-edit
 */
#define bnodes_make(new_node, d, h, mem) { \
	typeof(*new_node) node = {.right=null, .left=null, .parent=null, \
		.hash=h, .data=d, .color=bnodes_black_color, .frequency=1}; \
	new_node = mems_alloc(mem, sizeof(typeof(*new_node))); \
	errors_panic("bnodes_make.new_node", new_node == null); \
	*new_node = node; \
}

/*
 * Frees n, having a cleanup function provided 
 * (cleanfunc, defined in utils.h).
 *
 * Doesn't free any subsequent nodes conected to it.
 *
 * #to-edit
 */
void bnodes_free(bnode *n, const allocator *mem, freefunc cleanup);

/*
 * Pushes a new node to n. If n is null 
 * then new_bnode becomes root, thus being returned.
 *
 * If function errors to push new_bnode 
 * (and, thus, ownership) error will be 
 * set to true. Variable error may be set 
 * to null to silently error.
 *
 * #to-edit
 */
void bnodes_push(bnode **n, bnode *new_node, const allocator *mem, freefunc cleanup, bool *error);

/*
 * Gets node with the same hash as hash, if 
 * it fails or it doesn't find it, it returns 
 * null.
 */
bnode* bnodes_get(bnode* n, size_t hash);

void *bnodes_get_data(bnode *n, size_t hash);

size_t bnodes_get_frequency(bnode *n, size_t hash);

/*
 * Traverses n in-order executing callback.
 */
void bnodes_traverse(bnode *n, callfunc callback);

void bnodes_free_all_private(bnode *n, const allocator *mem, freefunc cleanup, size_t stackframe);

void bnodes_free_all(bnode *n, const allocator *mem, freefunc cleanup);

__attribute_warn_unused_result__
size_t bnodes_length(bnode *n);

#define sets(t) bnodes(t, t ## _set)

#define sets_push(s, item, hasher, mem, cleanup, error) { \
	typeof(*s) *new_bnode = null; \
	bnodes_make(new_bnode, item, hasher(&item), mem); \
	bnodes_push((bnode **)&s, (bnode *)new_bnode, mem, (freefunc)cleanup, error); \
}

#define sets_get(ib, it) \
	(typeof(ib->data) *)bnodes_get_data((bnode *)ib, it)

#define sets_traverse(ib, callback) \
	bnodes_traverse((bnode *)ib, (callfunc)callback)

#define sets_free(ib, mem, cleanup) \
	bnodes_free_all((bnode *)ib, mem, (freefunc)cleanup)

#define key_pairs(t0, t1) struct { \
	typeof(t0) key; \
    typeof(t1) value; \
}

#define maps(t0) bnodes(t0, t0 ## _map)

#define maps_push(m, k, v, hasher, mem, cleanup, error) { \
	typeof(m->data) item = {.key=k, .value=v}; \
	typeof(*m) *new_bnode = null; \
	bnodes_make(new_bnode, item, hasher(&item.key), mem); \
	bnodes_push((bnode **)&m, (bnode *)new_bnode, mem, (freefunc)cleanup, error); \
}

#define maps_get(ib, it) sets_get(ib, it)

#define maps_traverse(ib, callback) sets_traverse(ib, callback)

#define maps_free(ib, mem, callback) sets_free(ib, mem, callback)

//

typedef struct node node;
typedef struct node_set node_set;

struct node {
	node_set *next;
	void *data;
};

sets(node);

#define nodes(n, t0, t1) struct n { \
	typeof(t0) *next; \
	typeof(t1) data; \
}

#endif
