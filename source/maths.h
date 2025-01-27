#ifndef cels_maths_h
#define cels_maths_h

#include <stddef.h>
#include <math.h>

/*
 * The 'maths' module provides 
 * functions for popular mathematical 
 * problems.
 */

/*
 * Math's min utility.
 */
#define maths_min(a, b) a > b ? b : a

/*
 * Math's max utility.
 */
#define maths_max(a, b) a > b ? a : b

/*
 * Math's utility to swap a and b 
 * using temporary generic data.
 */
#define maths_swap(a, b) \
	typeof(a) temp = b; \
	b = a; \
	a = temp;

/*
 * Finds nearest two-powered number 
 * bigger than 'number'.
 */
size_t maths_nearest_two_power(size_t x);

/*
 * Gets sigmoid of x.
 */
double maths_sigmoid(double x);

#endif
