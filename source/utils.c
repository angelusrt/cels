#include "utils.h"

bool functors_check(const functor *f) {
	if (errors_check("functors_check (f)", f == NULL)) return true;
	if (errors_check("functors_check (f.func)", f->func == NULL)) return true;
	if (errors_check("functors_check (f.params)", f->params == NULL)) return true;

	return false;
}

bool size_equals(const size_t *a, const size_t *b, const size_t *i) {
	return a[*i] == b[*i];
}

size_t maths_nearest_two_power(size_t a) {
	size_t b = 1;
	while (b < a) {
		b <<= 1;
	}

	return b;
}

//benchs

void benchs_measure(const char *name, const size_vec *size_buckets, benchfunc callback) {
	for (size_t i = 0; i < size_buckets->size; i++) {
		clock_t time = 0;
		for (size_t j = 0; j < benchs_times; j++) {
			time += callback(size_buckets->data[i]);
		}

		printf("[bucket: %zu]\t %s took: %fs\n", size_buckets->data[i], name, (time/(double)benchs_times)/CLOCKS_PER_SEC);
	}
}
