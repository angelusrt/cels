#ifndef mems_h
#define mems_h

#include <stddef.h>
#include <stdlib.h>
#include <sys/cdefs.h>
#include "errors.h"

/* allocators */

typedef enum allocator_type {
	allocators_individual_type,
	allocators_group_type,
} allocator_type;

typedef void *(*allocfunc)(void *, size_t);
typedef bool (*deallocfunc)(void *, void *, size_t);
typedef void *(*reallocfunc)(void *, void *, size_t, size_t);
typedef void (*debugfunc)(void *);
typedef void (*cleanfunc)(void *);

typedef struct allocator {
	allocator_type type;
	allocfunc alloc;
	reallocfunc realloc;
	cleanfunc free;
	void *storage;
	deallocfunc dealloc;
	debugfunc debug;
} allocator;

typedef void (*freefunc)(void *, const allocator *);

__attribute_warn_unused_result__
bool allocators_check(const allocator *self);

/* arenas */

__attribute_warn_unused_result__
allocator arenas_init(size_t capacity);

/* stack_arenas */

#define stack_arenas_init(cap) \
	stack_arenas_make(cap, alloca(cap))

__attribute_warn_unused_result__
allocator stack_arenas_make(size_t capacity, char *buffer);

/* allocs */

typedef void *(*mallocfunc)(size_t);

typedef struct alloc {
	allocator_type type;
	__attribute_warn_unused_result__
	mallocfunc alloc;
	__attribute_warn_unused_result__
	allocfunc realloc;
	cleanfunc free;
} alloc;

__attribute_warn_unused_result__
alloc allocs_init(void);

/* mems */

__attribute_warn_unused_result__
void *mems_alloc(const allocator *mem, size_t len);

__attribute_warn_unused_result__
void *mems_realloc(const allocator *mem, void *data, size_t old_size, size_t new_size);

bool mems_dealloc(const allocator *mem, void *data, size_t block_size);

void mems_free(const allocator *mem, void *data);

#endif
