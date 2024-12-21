#include "../source/errors.h"
#include "../source/utils.h"
#include "../source/mems.h"
#include "../source/strings.h"

__attribute_warn_unused_result__
error_report arenas_test_init(void) {
	printf(__func__);
	printf("\n");

	size_t stat = 0, total = 0;

	allocator mem = arenas_init(2048);
	string text0 = strings_make("exemplo de texto", &mem);
	string text1 = strings_make(" ", &mem);
	string_vec text2 = strings_make_split(&text0, &text1, 0, &mem);
	unused string text3 = strings_make("more text", &mem);
	unused string text4 = strings_make("and even more text", &mem);

	stat += errors_assert("strings_make_split(&text0, &text1, 0, &mem).size == 3", text2.size == 3);
	total++;

	//mem.debug(mem.storage);
	mem.free(mem.storage);
	return (error_report) {.total=total, .successfull=stat};
}

void mems_test(void) {
	printf("=======\n");
	printf(__func__);
	printf("\n");
	printf("=======\n\n");

	error_report rep;
	reportfunc functions[] = {
		arenas_test_init,
		null,
	};

	size_t i = 0;
	while (functions[i]) {
		rep = functions[i]();
		error_reports_print(&rep);
		i++;
	}
}
