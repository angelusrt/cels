#include "../source/errors.h"
#include "../source/utils.h"
#include "../source/mems.h"
#include "../source/strings.h"

void arenas_test_init(error_report *report) {
	allocator mem = arenas_init(2048);

	string text0 = strings_make("exemplo de texto", &mem);
	string text1 = strings_make(" ", &mem);
	string_vec text2 = strings_split(&text0, text1, 0, &mem);

	errors_expect("split('exemplo de texto', ' ', 0, &mem).size == 3", text2.size == 3, report);
	
	//mem.debug(mem.storage);
	mem.free(mem.storage);
}

void mems_test(void) {
	printf("=======\n");
	printf("mems\n");
	printf("=======\n\n");

	reportfunc functions[] = {
		arenas_test_init,
		null,
	};

	size_t i = 0;
	error_report report = {0};
	while (functions[i]) {
		functions[i](&report);
		i++;
	}

	error_reports_print(&report);
}
