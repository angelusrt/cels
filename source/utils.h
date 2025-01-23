#ifndef utils_h
#define utils_h

#include <stddef.h>
#include <stdlib.h>
#include <sys/cdefs.h>
#include <time.h>
#include <stddef.h>
#include "errors.h"

/* function-type-definitions */

typedef void *(*callfunc) (void *);
typedef void *(*selffunc)(void *, void *);
typedef bool (*compfunc)(void *, void *);
typedef void (*shoutfunc) (void *, void *);
typedef size_t (*hashfunc)(void *);
typedef void (*cleanfunc)(void *);
typedef void (*printfunc)(void *);
typedef bool (*filterfunc)(void *);
typedef void (*reportfunc)(error_report *);
typedef clock_t (*benchfunc) (size_t);

/* functor */

typedef struct functor {
	callfunc func;
	void *params;
} functor;

typedef struct enfunctor {
	selffunc func;
	void *params;
} enfunctor;

cels_warn_unused
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

#define defaults_hash(a) (*a)

#define defaults_seems(a, b) (tolower(*a) == tolower(*b))

#define defaults_clone(a, mem) *a

#define defaults_free(a, mem)

/* others */

#define own

#define null NULL

typedef unsigned char uchar;
typedef unsigned long ulong;

/* utils */

/*
 * A convenience over utils_measure
 * that provides the function name.
 *
 * #to-review
 */
#define utils_measure(callback) \
	utils_measure_helper(#callback, callback)

/*
 * A function that measure the 
 * performance of the callback in 
 * buckets of time and prints to 
 * the terminal.
 *
 * Don't use it, prefer 
 * utils_measure intead.
 *
 * #to-review
 */
void utils_measure_helper(const char *function_name, benchfunc callback);

#endif
