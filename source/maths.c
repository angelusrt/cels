#include "maths.h"


/* maths */

size_t maths_nearest_two_power(size_t x) {
	size_t nearest = 1;
	while (nearest < x) {
		nearest <<= 1;
	}

	return nearest;
}

double maths_sigmoid(double x) {
	return 1 / (1 + pow(M_E, -x));
}
