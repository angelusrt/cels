#include <stddef.h>
#include <time.h>
#include <stdlib.h>

#include "../source/nodes.h"
#include "../source/utils.h"

typedef struct int_set int_set;
sets(int);

size_t _ints_hasherize(int *a) { return *a; }
void *int_sets_free(int_set *s) {
	if(s != null) { free(s); }
	return null;
}

void *_sets_print(int_set *is) {
	//printf("%d ", is->data);
	//printf(".");
	fflush(stdout);

	return null;
}

clock_t sets_bench(size_t size) {
	clock_t start = clock();

	int_set *ages = null;
	for (size_t i = 0; i < size; i++) {
		int random = rand();
		sets_push(ages, random, _ints_hasherize, null, null, null);
	}

	/*printf("\ndata: ");
	sets_traverse(ages, _sets_print);
	printf("\n\n");*/

	sets_traverse(ages, int_sets_free);
	clock_t end = clock();

	return end - start;
}


void nodes_bench() {
	size_vec size_buckets = vectors_premake(size_t, 5, 10, 100, 1000, 10000, 100000);
	benchs_measure("sets_bench", &size_buckets, sets_bench);
}
