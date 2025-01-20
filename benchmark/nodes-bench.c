#include <stddef.h>
#include <time.h>
#include <stdlib.h>

#include "../source/nodes.h"
#include "../source/utils.h"

void ints_print(const int *a) { 
	printf("%d", *a);
}

sets_generate_definition(int, int_set)
sets_generate_implementation(
	int, 
	int_set, 
	defaults_check, 
	ints_print, 
	defaults_hash,
	defaults_free)

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
	utils_measure(sets_bench);
}
