#include "../source/mems.h"
#include "../source/mems.c"

#include "../source/vectors.h"
#include "../source/vectors.c"

#include "../source/strings.h"
#include "../source/strings.c"

#include "../source/errors.c"
#include "../source/utils.c"

int main(void) {
	allocator mem = stack_arenas_init(128); 
	errors_panic("main.storage.data", ((stack_arena *)mem.storage)->data == null);

	string text0 = strings_make("teste, de, string, alocada", &mem);
	string text1 = strings_make(",", &mem);
	string text2 = strings_make(";", &mem);
	string text3 = strings_make_replace(&text0, &text1, &text2, -1, &mem);

	string_vec texts = strings_make_split(&text0, &text1, -1, &mem);

	printf("text3: %s\n", text3.data);
	mem.debug(mem.storage);

	mem.dealloc(mem.storage, text2.data, text2.capacity);
	mem.dealloc(mem.storage, text1.data, text1.capacity);
	mem.dealloc(mem.storage, text0.data, text0.capacity);

	mem.debug(mem.storage);
	mem.free(mem.storage);

	return 0;
}
