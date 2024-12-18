#include <stddef.h>
#include <time.h>
#include <stdlib.h>

#include "../source/nodes.h"
#include "../source/utils.h"
#include "../source/benchs.h"

void defaults_free(unused void *a, unused const allocator *mem) { }
size_t ints_hasherize(int *a) { return *a; }
void *ints_free(int *self, const allocator *mem) {
	mems_dealloc(mem, self, sizeof(int));
	return null;
}

sets_generate_definition(int, int_set)
sets_generate_implementation(int, int_set, ints_hasherize, defaults_free)

clock_t sets_bench(size_t size) {
	clock_t start = clock();

	int_set *ages = null;
	for (size_t i = 0; i < size; i++) {
		int random = rand();
		int_sets_push(&ages, random, null);
	}

	int_sets_free(ages, null);
	clock_t end = clock();

	return end - start;
}

void nodes_bench() {
	size_vec size_buckets = vectors_premake(size_t, 5, 10, 100, 1000, 10000, 100000);
	benchs_measure("sets_bench", &size_buckets, sets_bench);
	size_vecs_free(&size_buckets, null);
}
