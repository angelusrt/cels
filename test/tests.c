#include "strings-test.c"
#include "vectors-test.c"

#include "../source/vectors.c"
#include "../source/errors.c"
#include "../source/strings.c"
#include "../source/utils.c"
#include "../source/nodes.c"

int main() {
	strings_test();
	vectors_test();

	return 0;
}
