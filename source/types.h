#ifndef cels_types_h
#define cels_types_h

#include <stddef.h>


/*
 * The 'types' module defines a 
 * lot of types and directives.
 */


/* types */

#ifdef __GNUC__
#define cels_warn_unused __attribute__((warn_unused_result))
#define notused __attribute__((unused))
#else
#define cels_warn_unused
#define notused
#endif

#ifndef cels_debug
#define cels_debug true
#endif

#ifndef cels_max_recusion
#define cels_max_recursion 100
#endif

#define cels_vargs(...) , ##__VA_ARGS__


/* functions */

typedef void *(*callfunc) (void *);
typedef void *(*selffunc)(void *, void *);
typedef bool (*compfunc)(void *, void *);
typedef void (*shoutfunc) (void *, void *);
typedef size_t (*hashfunc)(void *);
typedef void (*cleanfunc)(void *);
typedef void (*printfunc)(void *);
typedef void (*dofunc)(void *);
typedef bool (*filterfunc)(void *);

typedef struct functor {
	callfunc func;
	void *params;
} functor;

typedef struct enfunctor {
	selffunc func;
	void *params;
} enfunctor;


/* defaults */

#define defaults_compare(a, b) (*a == *b)

#define defaults_check(a) (false)

#define defaults_hash(a) (*a)

#define defaults_seems(a, b) (tolower(*a) == tolower(*b))

#define defaults_clone(a, mem) *a

#define defaults_free(a, mem)


/* others */

#define own

#define priv

#define null NULL

typedef unsigned char uchar;
typedef unsigned long ulong;

#endif
