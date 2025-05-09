#include "utils.h"


/* utils */

void utils_measure_helper(const char *function_name, measurefunc callback) {
	#define bucket_size 5
	static const size_t buckets[bucket_size] = {10, 100, 1000, 10000, 100000};

	#if cels_debug
		errors_abort("function_name", !function_name);
		errors_abort("function_name[0] == '\0'", function_name[0] == '\0');
	#endif

	for (size_t i = 0; i < bucket_size; i++) {
		clock_t time = 0;
		for (size_t j = 0; j < bucket_size; j++) {
			time += callback(buckets[i]);
		}

		printf(
			"[bucket: %6zu] %s took: %fs\n", 
			buckets[i], 
			function_name, 
			(time/(double)bucket_size)/CLOCKS_PER_SEC);
	}

	#undef bucket_size
}


/* next */

bool next(ssize_t start, ssize_t end, size_t step, iterator *iterator) {
	if (!iterator || !step) return false;

	if (!iterator->is_init) {
		iterator->data = start;
		iterator->is_init = true;
		return true;
	}

	bool is_reverse = start > end;

	if (!is_reverse) {
		bool fits = start + (ssize_t)step < end;
		if (!fits) return false;

		bool is_within = iterator->data + (ssize_t)step < end;
		if (!is_within) return false;

		iterator->data += step;
		return true;
	} 

	bool fits = end + (ssize_t)step < start;
	if (!fits) return false;

	bool is_within = end < iterator->data - (ssize_t)step;
	if (!is_within) return false;

	iterator->data -= step;
	return true;
} 
