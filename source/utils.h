#ifndef utils_h
#define utils_h

#include <stddef.h>
#include <stdlib.h>
#include <sys/cdefs.h>
#include <time.h>
#include <stddef.h>

#include "errors.h"

typedef void *(*callfunc) (void *);
typedef void *(*selffunc)(void *, void *);
typedef bool (*compfunc)(void *, void *);
typedef bool (*compvecfunc)(void *, void *, void *);
typedef size_t (*hashfunc)(void *);
typedef void (*cleanfunc)(void *);
typedef void (*printfunc)(void *);
typedef bool (*filterfunc)(void *);
typedef error_report (*reportfunc)(void);
typedef void (*printvecfunc)(void *, void *);
typedef clock_t (*benchfunc) (size_t);

#define notused __attribute__((unused))

#define null NULL

typedef u_char uchar;

/* functor */

typedef struct functor {
	callfunc func;
	void *params;
} functor;

typedef struct enfunctor {
	selffunc func;
	void *params;
} enfunctor;

__attribute_warn_unused_result__
bool functors_check(const functor *f);

/* maths */

#define maths_min(a, b) a > b ? b : a

#define maths_max(a, b) a > b ? a : b

#define maths_swap(a, b) \
	typeof(a) temp = b; \
	b = a; \
	a = temp; \

size_t maths_nearest_two_power(size_t a);

/* defaults */

#define defaults_compare(a, b) (*a == *b)

#define defaults_check(a) (false)

#define defaults_seems(a, b) (tolower(*a) == tolower(*b))

#define defaults_clone(a, mem) *a

#define defaults_free(a, mem)

//void defaults_free(void *a, const void *mem);

/* benchs */

/*
#include "vectors.h"

#define benchs_times 5

void benchs_measure(const char *name, const size_vec *size_buckets, benchfunc callback);
*/

#endif
