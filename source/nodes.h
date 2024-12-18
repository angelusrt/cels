#ifndef bnodes_h
#define bnodes_h
#pragma once

#include <stddef.h>
#include "utils.h"
#include "mems.h"

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
#define bnodes(t0, t1) struct t1 { \
	bnodes_color color; \
	size_t hash; \
	ulong frequency; \
	typeof(t1) *parent; \
	typeof(t1) *right; \
	typeof(t1) *left; \
	typeof(t0) data; \
}

typedef struct bnode bnode;
typedef bnodes(void *, bnode) bnode;

#define nodes_max_recursion 30

/*
 * Makes a bynary-node of type t and from data d and hash h, 
 * which will be allocated to new_bnode (a pointer)
 *
 * #allocates #may-panic #to-edit
 */
#define bnodes_make(self, item, id, mem) { \
	typeof(*self) node = { \
		.hash=id, \
		.data=item, \
		.color=bnodes_black_color, \
		.frequency=1}; \
	self = mems_alloc(mem, sizeof(typeof(*self))); \
	errors_panic("bnodes_make.self", self == null); \
	*self = node; \
}

/*
 * Frees n, having a cleanup function provided 
 * (cleanfunc, defined in utils.h).
 *
 * Doesn't free any subsequent nodes conected to it.
 *
 * #to-edit
 */
#define bnodes_free(self, mem, cleanup) \
	bnodes_free_all((bnode *)self, mem, (freefunc)cleanup)

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
bool bnodes_push(bnode **self, bnode *new_node);

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

void bnodes_free_all(bnode *n, const allocator *mem, freefunc cleanup);

__attribute_warn_unused_result__
size_t bnodes_length(bnode *n);

//sets

#define sets(t) bnodes(t, t ## _set)

#define sets_generate_implementation(type, name, hasher, cleanup) \
	void name##s_free_private(name *self, const allocator *mem) { \
		cleanup(&self->data, mem); \
		mems_dealloc(mem, self, sizeof(*self)); \
		self = null; \
	} \
	\
	bool name##s_push(name **self, type item, const allocator *mem) { \
		name *new_bnode = null; \
		bnodes_make(new_bnode, item, hasher(&item), mem) \
		\
		bool error = bnodes_push((bnode **)self, (bnode *)new_bnode); \
		if (error) { name##s_free_private(new_bnode, mem); } \
		\
		return error; \
	} \
	\
	type *name##s_get(const name *self, type item) { \
		return (type *)bnodes_get_data((bnode *)self, hasher(&item)); \
	} \
	void name##s_traverse(name *self, callfunc callback) { \
		bnodes_traverse((bnode *)self, callback); \
	} \
	\
	void name##s_free(name *self, const allocator *mem) { \
		bnodes_free(self, mem, name##s_free_private); \
	}

#define sets_generate_definition(type, name) \
	typedef struct name name; \
	typedef bnodes(type, name) name; \
	\
	bool name##s_push(name **self, type item, const allocator *mem); \
	\
	__attribute_warn_unused_result__ \
	type *name##s_get(const name *self, type item); \
	\
	void name##s_traverse(name *self, callfunc callback); \
	\
	void name##s_free(name *self, const allocator *mem);
	
//maps

#define key_pairs(t0, t1) struct { \
	typeof(t0) key; \
    typeof(t1) value; \
}

#define maps(t0) bnodes(t0, t0 ## _map)

#define maps_generate_implementation(type0, type1, type2, name, hasher0, cleanup0, cleanup1) \
	void name##s_free_private(name *self, const allocator *mem) { \
		cleanup0(&self->data.key, mem); \
		cleanup1(&self->data.value, mem); \
		mems_dealloc(mem, self, sizeof(*self)); \
		self = null; \
	} \
	\
	bool name##s_push(name **self, type0 key, type1 value, const allocator *mem) { \
		type2 item = {.key=key, .value=value}; \
		name *new_bnode = null; \
		bnodes_make(new_bnode, item, hasher0(&item.key), mem); \
		\
		bool error = bnodes_push((bnode **)self, (bnode *)new_bnode); \
		if (error) { name##s_free_private(new_bnode, mem); } \
		\
		return error; \
	} \
	\
	type1 *name##s_get(const name *self, type0 item) { \
		type2 *temp = bnodes_get_data((bnode *)self, hasher0(&item)); \
		\
		if (temp == null) { return null; } \
		\
		return &temp->value; \
	} \
	\
	size_t name##s_get_frequency(name *self, type0 item) { \
		return bnodes_get_frequency((bnode *)self, hasher0(&item)); \
	} \
	\
	void name##s_traverse(name *self, callfunc callback) { \
		bnodes_traverse((bnode *)self, callback); \
	} \
	\
	void name##s_free(name *self, const allocator *mem) { \
		bnodes_free(self, mem, name##s_free_private); \
	}

#define maps_generate_definition(type0, type1, type2, name) \
	typedef key_pairs(type0, type1) type2; \
	typedef struct name name; \
	typedef bnodes(type2, name) name; \
	\
	bool name##s_push(name **self, type0 key, type1 value, const allocator *mem); \
	\
	__attribute_warn_unused_result__ \
	type1 *name##s_get(const name *self, type0 item); \
	\
	__attribute_warn_unused_result__ \
	size_t name##s_get_frequency(name *self, type0 item); \
	\
	void name##s_traverse(name *self, callfunc callback); \
	\
	void name##s_free(name *self, const allocator *mem);

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
