#ifndef utils_h
#define utils_h

#include <stddef.h>
#include <stdlib.h>
#include <sys/cdefs.h>
#include <time.h>
#include <stddef.h>

#include "errors.h"

typedef void *(*callfunc) (void *);
typedef bool (*compfunc)(void *, void *);
typedef bool (*compvecfunc)(void *, void *, void *);
typedef size_t (*hashfunc)(void *);
typedef void (*cleanfunc)(void *);
typedef void (*printfunc)(void *);
typedef error_report (*reportfunc)(void);
typedef void (*printvecfunc)(void *, void *);
typedef clock_t (*benchfunc) (size_t);

typedef struct functor {
	callfunc func;
	void *params;
} functor;

#define unused __attribute_maybe_unused__

#define null NULL

__attribute_warn_unused_result__
bool functors_check(const functor *f);

__attribute_warn_unused_result__
bool size_equals(const size_t *a, const size_t *b, const size_t *i);

#define maths_min(a, b) a > b ? b : a

#define maths_max(a, b) a > b ? a : b

#define maths_swap(a, b) \
	typeof(a) temp = b; \
	b = a; \
	a = temp; \

size_t maths_nearest_two_power(size_t a);

//benchs

/*
#include "vectors.h"

#define benchs_times 5

void benchs_measure(const char *name, const size_vec *size_buckets, benchfunc callback);
*/

#endif
