#ifndef mems_h
#define mems_h

#include <stddef.h>
#include <stdlib.h>
#include <sys/cdefs.h>
#include "errors.h"

/* definitions */
typedef void *(*allocfunc)(void *, size_t);
typedef error (*deallocfunc)(void *, void *, size_t);
typedef void *(*reallocfunc)(void *, void *, size_t, size_t);
typedef void (*debugfunc)(void *);
typedef void (*cleanfunc)(void *);
typedef void *(*mallocfunc)(size_t);

/* allocators */

typedef enum allocator_type {
	allocators_individual_type,
	allocators_group_type,
} allocator_type;

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

/* 
 * Checks the validity of an allocator.
 *
 * #to-review
 */
cels_warn_unused
bool allocators_check(const allocator *self);

/* arenas */

/* 
 * Initializes a group allocator on the heap
 * called an arena. When you successfully 
 * allocates data with it, you move the ownership 
 * to the arena data structure where the cleanup 
 * is done with it.
 *
 * #to-review
 */
cels_warn_unused
allocator arenas_init(size_t capacity);

/* stack_arenas */

/*
 * Initializes a group allocator that allocates 
 * to the stack, meaning that the variables 
 * allocated with it are ultimately automatic
 * and so, it should'nt be freed.
 *
 * #to-review
 */
#define stack_arenas_init(cap) \
	stack_arenas_make(cap, alloca(cap))

/*
 * Use stack_arenas_init instead.
 *
 * #private #shouldnt-be-used
 */
cels_warn_unused
allocator stack_arenas_make(size_t capacity, char *buffer);

/* allocs */

typedef struct alloc {
	allocator_type type;
	cels_warn_unused
	mallocfunc alloc;
	cels_warn_unused
	allocfunc realloc;
	cleanfunc free;
} alloc;

/*
 * It returns a static individual allocator 
 * that allocates to the heap. Cleanup should 
 * be done in an element-basis.
 *
 * #to-review
 */
cels_warn_unused
alloc allocs_init(void);

/* mems */

/*
 * An adapter that manages (generically)
 * the allocation of data with
 * group/individual/default allocators.
 *
 * When you provide mem with null, 
 * the default allocator (malloc) is used.
 *
 * If an error happens, it returns null.
 *
 * #to-review
 */
cels_warn_unused
void *mems_alloc(const allocator *mem, size_t len);

/*
 * An adapter that manages (generically)
 * the reallocation of data with
 * group/individual/default allocators.
 *
 * When you provide mem with null, 
 * the default reallocator (realloc) is used.
 *
 * If an error happens, it returns null.
 *
 * #to-review
 */
cels_warn_unused
void *mems_realloc(const allocator *mem, void *data, size_t old_size, size_t new_size);

/*
 * An adapter that manages (generically)
 * the annotation of data as to-be-used with
 * group/individual/default allocators.
 *
 * When you provide mem with null, 
 * the default cleanup (free) is used.
 *
 * If an individual allocator is provided, 
 * the data is cleared and block_size is ignored.
 *
 * If an error happens, it returns 1.
 *
 * #to-review
 */
error mems_dealloc(const allocator *mem, void *data, size_t block_size);

/*
 * An adapter that manages (generically)
 * the cleanup of data with
 * group/individual/default allocators.
 *
 * When you provide mem with null, 
 * the default cleanup (free) is used.
 *
 * #to-review
 */
void mems_free(const allocator *mem, void *data);

#endif
