#include "../cels/source/vectors.h"

#include "../cels/source/vectors.c"
#include "../cels/source/mems.c"
#include "../cels/source/errors.c"
#include "../cels/source/maths.c"

bool doubles_compare(double *a, double *b) { return *a > *b; }
bool doubles_filter(double *a) { return *a > 300; }

int main(void) {
	double_vec ages = vectors_premake(double, 12, 3, 4, 9, 52, 2);
	double_vec weights = vectors_premake(double, 50, 10, 13, 32, 62, 21);
	double_vec coefficients = vectors_premake(double, 0.2, 0.5, 0.7, 0.9, 0.99, 0.4);
	double_vec results = double_vecs_init(6, null);

	double_vecs_power(&weights, &coefficients, &results);
	double_vecs_multiply(&results, &ages, &results);
	double result = double_vecs_summation(&results);
	printf("\nresult: \n%lf\n", result);

	printf("\nsorted: \n");
	double_vecs_sort(&results, (compfunc)doubles_compare);
	double_vecs_println(&results);

	printf("\nfiltered: \n");
	double_vecs_filter(&results, (filterfunc)doubles_filter, null);
	double_vecs_println(&results);

	return 0;
}
