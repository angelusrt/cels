#include <stddef.h>
#include <time.h>
#include <stdlib.h>

#include "../source/nodes.h"
#include "../source/utils.h"

sets(int_set, int);

clock_t sets_bench(size_t size) {
	clock_t start = clock();

	int_set ages = {0};
	sets_init(ages);

	for (size_t i = 0; i < size; i++) {
		int random = rand();
		sets_push(&ages, &random, random, null);
	}

	sets_free(&ages, null, null);
	clock_t end = clock();

	return end - start;
}

void nodes_bench() {
	utils_measure(sets_bench);
}
