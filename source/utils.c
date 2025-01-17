#include "utils.h"

bool functors_check(const functor *f) {
	if (errors_check("functors_check (f)", f == NULL)) return true;
	if (errors_check("functors_check (f.func)", f->func == NULL)) return true;
	if (errors_check("functors_check (f.params)", f->params == NULL)) return true;

	return false;
}

size_t maths_nearest_two_power(size_t a) {
	size_t b = 1;
	while (b < a) {
		b <<= 1;
	}

	return b;
}

//void defaults_free(unused void *a, unused const void *mem) {}
