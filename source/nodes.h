#ifndef bnodes_h
#define bnodes_h
#pragma once

#include <stddef.h>
#include "utils.h"
#include "mems.h"
#include "errors.h"

//

#ifndef cels_debug
#define cels_debug false
#endif

//

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
#define bnodes(type0, type1) struct type1 { \
	bnodes_color color; \
	size_t hash; \
	ulong frequency; \
	typeof(type1) *parent; \
	typeof(type1) *right; \
	typeof(type1) *left; \
	typeof(type0) data; \
}

typedef struct bnode bnode;
typedef bnodes(void *, bnode) bnode;

#define nodes_max_recursion 30

/*
 * Makes a bynary-node of type of 'self' and from data 
 * item and hash id, which will be allocated to 
 * 'self' (a pointer).
 *
 * #allocates #may-panic
 * #to-review
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
#define sets_generate_implementation(type, name, check0, print0, hasher, cleanup) \
	void name##s_free_private(name *self, const allocator *mem) { \
		if (cels_debug) { \
			errors_panic(#name"s_free_private.self", bnodes_check((const bnode *)self)); \
		} \
		\
		cleanup(&self->data, mem); \
		mems_dealloc(mem, self, sizeof(name)); \
		self = null; \
	} \
	\
	void name##s_free_all_private(name *self, const allocator *mem, size_t stackframe) { \
		if (self == null || stackframe > nodes_max_recursion) { return; } \
		\
		name *left = self->left; \
		name *right = self->right; \
		\
		name##s_free_all_private(left, mem, ++stackframe); \
		name##s_free_private(self, mem); \
		name##s_free_all_private(right, mem, ++stackframe); \
	} \
	\
	void *name##s_print_private(const name *self) { \
		print0(&self->data); \
		printf("\n"); \
		return null; \
	} \
	\
	bool name##s_push(name **self, type item, const allocator *mem) { \
		if (cels_debug) { \
			errors_panic(#name"s_push.item", check0(&item)); \
		} \
		\
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
		if (cels_debug) { \
			errors_panic(#name"s_get.self", bnodes_check((const bnode *)self)); \
			errors_panic(#name"s_get.item", check0(&item)); \
		} \
		\
		return (type *)bnodes_get_data((bnode *)self, hasher(&item)); \
	} \
	\
	void name##s_traverse(name *self, callfunc callback) { \
		if (cels_debug) { \
			errors_panic(#name"s_traverse.self", bnodes_check((const bnode *)self)); \
		} \
		\
		bnodes_traverse((bnode *)self, callback); \
	} \
	\
	void name##s_print(const name *self) { \
		if (cels_debug) { \
			errors_panic(#name"s_print.self", bnodes_check((const bnode *)self)); \
		} \
		\
		bnodes_traverse((bnode *)self, (callfunc)name##s_print_private); \
	} \
	\
	void name##s_free(name *self, const allocator *mem) { \
		if (cels_debug) { \
			errors_panic(#name"s_free.self", bnodes_check((const bnode *)self)); \
		} \
		\
		name##s_free_all_private(self, mem, 0); \
	}

/*
 * A macro-template code-generator to create 
 * set's definitions for a certain type.
 *
 * #to-review
 */
#define sets_generate_definition(type, name) \
	typedef struct name name; \
	typedef bnodes(type, name) name; \
	\
	bool name##s_push(name **self, type item, const allocator *mem); \
	\
	cels_warn_unused \
	type *name##s_get(const name *self, type item); \
	\
	void name##s_traverse(name *self, callfunc callback); \
	\
	void name##s_print(const name *self); \
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
	void name##s_free_private(name *self, const allocator *mem) { \
		if (cels_debug) { \
			errors_panic(#name"s_free_private.self", bnodes_check((const bnode *)self)); \
		} \
		\
		cleanup0(&self->data.key, mem); \
		cleanup1(&self->data.value, mem); \
		mems_dealloc(mem, self, sizeof(name)); \
		self = null; \
	} \
	\
	void name##s_free_all_private(name *self, const allocator *mem, size_t stackframe) { \
		if (self == null || stackframe > nodes_max_recursion) { return; } \
		\
		name *left = self->left; \
		name *right = self->right; \
		\
		name##s_free_all_private(left, mem, ++stackframe); \
		name##s_free_private(self, mem); \
		name##s_free_all_private(right, mem, ++stackframe); \
	} \
	\
	void *name##s_print_private(const name *self) { \
		printf("{key: "); \
		print0(&self->data.key); \
		printf(", value: "); \
		print1(&self->data.value); \
		printf("}\n"); \
		return null; \
	} \
	\
	bool name##s_push(name **self, type0 key, type1 value, const allocator *mem) { \
		if (cels_debug) { \
			errors_panic(#name"s_push.key", check0(&key)); \
			errors_panic(#name"s_push.value", check1(&value)); \
		} \
		\
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
		if (cels_debug) { \
			errors_panic(#name"s_get.self", bnodes_check((const bnode *)self)); \
			errors_panic(#name"s_get.item", check0(&item)); \
		} \
		\
		type2 *temp = bnodes_get_data((bnode *)self, hasher0(&item)); \
		\
		if (temp == null) { return null; } \
		\
		return &temp->value; \
	} \
	\
	size_t name##s_get_frequency(name *self, type0 item) { \
		if (cels_debug) { \
			errors_panic(#name"s_get_frequency.self", bnodes_check((const bnode *)self)); \
			errors_panic(#name"s_get_frequency.item", check0(&item)); \
		} \
		\
		return bnodes_get_frequency((bnode *)self, hasher0(&item)); \
	} \
	\
	void name##s_traverse(name *self, callfunc callback) { \
		if (cels_debug) { \
			errors_panic(#name"s_traverse.self", bnodes_check((const bnode *)self)); \
		} \
		\
		bnodes_traverse((bnode *)self, callback); \
	} \
	\
	void name##s_print(const name *self) { \
		if (cels_debug) { \
			errors_panic(#name"s_print.self", bnodes_check((const bnode *)self)); \
		} \
		\
		bnodes_traverse((bnode *)self, (callfunc)name##s_print_private); \
	} \
	\
	void name##s_free(name *self, const allocator *mem) { \
		if (cels_debug) { \
			errors_panic(#name"s_free.self", bnodes_check((const bnode *)self)); \
		} \
		\
		name##s_free_all_private(self, mem, 0); \
	}

/*
 * A macro-template code-generator to create 
 * set's definitions for a certain type.
 *
 * #to-review
 */
#define maps_generate_definition(type0, type1, type2, name) \
	typedef key_pairs(type0, type1) type2; \
	typedef struct name name; \
	typedef bnodes(type2, name) name; \
	\
	bool name##s_push(name **self, type0 key, type1 value, const allocator *mem); \
	\
	cels_warn_unused \
	type1 *name##s_get(const name *self, type0 item); \
	\
	cels_warn_unused \
	size_t name##s_get_frequency(name *self, type0 item); \
	\
	void name##s_traverse(name *self, callfunc callback); \
	\
	void name##s_print(const name *self); \
	\
	void name##s_free(name *self, const allocator *mem);

//

typedef struct node node;
typedef struct node_set node_set;

struct node {
	node_set *next;
	void *data;
};

sets(node, node_set);

#define nodes(self, type0, type1) struct self { \
	typeof(type0) *next; \
	typeof(type1) data; \
}

#endif
