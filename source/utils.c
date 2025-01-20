#include "utils.h"

bool functors_check(const functor *self) {
	#if cels_debug
		errors_return("self", !self)
		errors_return("self.func", !self->func)
		errors_return("self.params", !self->params)
	#else
		if (!self) return true;
		if (!self->func) return true;
		if (!self->params) return true;
	#endif

	return false;
}

size_t maths_nearest_two_power(size_t a) {
	size_t b = 1;
	while (b < a) {
		b <<= 1;
	}

	return b;
}

void utils_measure_helper(const char *function_name, benchfunc callback) {
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
