#include "../source/vectors.h"
#include "../source/errors.h"
#include "../source/utils.h"

error_report vectors_test_init_and_check() {
	printf("vectors_test_init_and_check\n");
	size_t stat = 0, total = 0;

	vectors(int) v0 = vectors_init(sizeof(int), vectors_min);
	bool isvalid = !vectors_check((vector *)&v0);
	stat += errors_assert("check(vectors_init(int, 16)) == true", isvalid);
	total++;

	vectors(int) v1 = {0};
	isvalid = !vectors_check((vector *)&v1);
	stat += errors_assert("check({0}) == false", !isvalid);
	total++;

	vectors_free(v0);
	return (error_report) {.total=total, .successfull=stat};
}

error_report vectors_test_push_and_free() {
	printf("vectors_test_push_and_free\n");
	size_t stat = 0, total = 0;

	vectors(int) v0 = vectors_init(sizeof(int), vectors_min);
	vectors_push(v0, 10, null);

	stat += errors_assert("push(v0, 10) == [10]", v0.data[0] == 10);
	total++;

	vectors_free(v0);
	stat += errors_assert("free(v0).data == null", v0.data == null);
	total++;

	return (error_report) {.total=total, .successfull=stat};
}

bool _size_compare(size_t *i0, size_t *i1) { return *i0 > *i1; }

error_report vectors_test_premake_and_sort() {
	printf("vectors_test_sort\n");
	size_t stat = 0, total = 0;

	size_vec v0 = vectors_premake(size_t, 4, 4, 3, 2, 1);
	size_vec v1 = vectors_premake(size_t, 4, 1, 2, 3, 4);
	vectors_sort(v0, (compfunc)_size_compare);

	bool matches = vectors_equals((vector *)&v0, (vector *)&v1, (compvecfunc)size_equals);
	stat += errors_assert("sort([4, 3, 2, 1]) == [1, 2, 3, 4]", matches);
	total++;

	return (error_report) {.total=total, .successfull=stat};
}

error_report vectors_test_upscale() {
	printf("vectors_test_upscale\n");
	size_t stat = 0, total = 0;

	size_vec v0 = vectors_init(sizeof(size_t), vectors_min);
	vectors_upscale((vector *)&v0, sizeof(size_t), null);

	stat += errors_assert("upscale({.capacity=16}).capacity == 32", v0.capacity == 32);
	total++;

	vectors_free(v0);
	return (error_report) {.total=total, .successfull=stat};
}

error_report vectors_test_equals() {
	printf("vectors_test_sort\n");
	size_t stat = 0, total = 0;

	size_vec v0 = vectors_premake(size_t, 2, 4, 3);
	size_vec v1 = vectors_premake(size_t, 2, 3, 4);

	bool matches = vectors_equals((vector *)&v0, (vector *)&v0, (compvecfunc)size_equals);
	stat += errors_assert("equals([4, 3], [4, 3]) == true", matches);
	total++;

	matches = vectors_equals((vector *)&v0, (vector *)&v1, (compvecfunc)size_equals);
	stat += errors_assert("equals([4, 3], [3, 4]) == false", !matches);
	total++;

	return (error_report) {.total=total, .successfull=stat};
}

error_report vectors_test_sized_find() {
	printf("vectors_test_sized_find\n");
	size_t stat = 0, total = 0;

	size_vec v0 = vectors_premake(size_t, 4, 4, 3, 1, 9);
	size_t item = 1;

	ssize_t pos = vectors_sized_find((vector *)&v0, (void *)&item, sizeof(size_t));
	printf("%zu\n", pos);

	stat += errors_assert("sized_find([4, 3, 1, 9], 1, 8) == 2", pos == 2);
	total++;

	return (error_report) {.total=total, .successfull=stat};
}

void vectors_test() {
	printf("=======\n");
	printf("vectors\n");
	printf("=======\n\n");

	error_report rep;
	reportfunc functions[] = {
		vectors_test_init_and_check,
		vectors_test_push_and_free,
		vectors_test_premake_and_sort,
		vectors_test_upscale,
		vectors_test_equals,
		vectors_test_sized_find,
		NULL,
	};

	size_t i = 0;
	while (functions[i]) {
		rep = functions[i]();
		error_reports_print(&rep);
		i++;
	}
}
