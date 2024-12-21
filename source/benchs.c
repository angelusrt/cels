#include "benchs.h"

void benchs_measure(const char *name, const size_vec *size_buckets, benchfunc callback) {
	#if cels_debug
		errors_panic("benchs_measure.name", name == null);
		errors_panic("benchs_measure.#name", strlen(name) <= 1);
		errors_panic("benchs_measure.size_buckets", vectors_check((const vector *)size_buckets));
	#endif

	for (size_t i = 0; i < size_buckets->size; i++) {
		clock_t time = 0;
		for (size_t j = 0; j < benchs_times; j++) {
			time += callback(size_buckets->data[i]);
		}

		printf(
			"[bucket: %zu]\t %s took: %fs\n", 
			size_buckets->data[i], 
			name, 
			(time/(double)benchs_times)/CLOCKS_PER_SEC);
	}
}
