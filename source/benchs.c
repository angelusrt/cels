#include "benchs.h"

void benchs_measure(const char *name, const size_vec *size_buckets, benchfunc callback) {
	for (size_t i = 0; i < size_buckets->size; i++) {
		clock_t time = 0;
		for (size_t j = 0; j < benchs_times; j++) {
			time += callback(size_buckets->data[i]);
		}

		printf("[bucket: %zu]\t %s took: %fs\n", size_buckets->data[i], name, (time/(double)benchs_times)/CLOCKS_PER_SEC);
	}
}
