#include "../source/vectors.h"
#include "../source/errors.h"
#include "../source/utils.h"

void vectors_test_init_and_check(error_report *report) {
	size_vec v0 = size_vecs_init(vector_min, null);
	bool isvalid = !vectors_check((vector *)&v0);
	errors_expect("check(vectors_init(16)) == true", isvalid, report);
	
	isvalid = !vectors_check((vector *)&(size_vec){0});
	errors_expect("check({0}) == false", !isvalid, report);
	
	size_vecs_free(&v0, null);
}

void vectors_test_push_and_free(error_report *report) {
	size_vec v0 = size_vecs_init(vector_min, null);
	size_vecs_push(&v0, 10, null);
	errors_expect("push(v0, 10)[0] == 10", v0.data[0] == 10, report);
	
	size_vecs_free(&v0, null);
	errors_expect("free(v0).data == null", v0.data == null, report);
}

bool _size_compare(size_t *i0, size_t *i1) { return *i0 > *i1; }

void vectors_test_premake_and_sort(error_report *report) {
	size_vec v0 = vectors_premake(size_t, 4, 3, 2, 1);
	size_vec v1 = vectors_premake(size_t, 1, 2, 3, 4);

	size_vecs_sort(&v0, (compfunc)_size_compare);
	bool matches = size_vecs_equals(&v0, &v1);
	errors_expect("sort([4, 3, 2, 1]) == [1, 2, 3, 4]", matches, report);
}

void vectors_test_equals(error_report *report) {
	size_vec v0 = vectors_premake(size_t, 4, 3);
	size_vec v1 = vectors_premake(size_t, 3, 4);

	bool matches = size_vecs_equals(&v0, &v0);
	errors_expect("equals([4, 3], [4, 3]) == true", matches, report);
	
	matches = size_vecs_equals(&v0, &v1);
	errors_expect("equals([4, 3], [3, 4]) == false", !matches, report);
}

void vectors_test_find(error_report *report) {
	size_vec v0 = vectors_premake(size_t, 4, 3, 1, 9);
	size_t item = 1;

	ssize_t pos = size_vecs_find(&v0, item);
	errors_expect("find([4, 3, 1, 9], 1, 8) == 2", pos == 2, report);
}

void vectors_test(void) {
	printf("=======\n");
	printf("vectors\n");
	printf("=======\n\n");

	reportfunc functions[] = {
		vectors_test_init_and_check,
		vectors_test_push_and_free,
		vectors_test_premake_and_sort,
		vectors_test_equals,
		vectors_test_find,
		null,
	};

	size_t i = 0;
	error_report report = {0};
	while (functions[i]) {
		functions[i](&report);
		i++;
		printf("\n");
	}

	error_reports_print(&report);
}
