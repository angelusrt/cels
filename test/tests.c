#include "strings-test.c"
#include "vectors-test.c"
#include "mems-test.c"

#include "../source/vectors.c"
#include "../source/errors.c"
#include "../source/strings.c"
#include "../source/utils.c"
#include "../source/nodes.c"
#include "../source/mems.c"

int main() {
	strings_test();
	vectors_test();
	mems_test();

	return 0;
}
