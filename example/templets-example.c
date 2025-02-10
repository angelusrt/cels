
#include "../source/templets.h"

#include "../source/templets.c"
#include "../source/strings.c"
#include "../source/vectors.c"
#include "../source/nodes.c"
#include "../source/mems.c"
#include "../source/errors.c"
#include "../source/maths.c"
#include "../source/files.c"

static const string templ = strings_premake(
	"<| define index |>\n"
	"<html>\n"
	"<head>\n"
	"</head>\n"
	"<body>\n"
	"<h1><| title |></h1>\n"
	"<h2><| subtitle |></h2>\n"
	"<p><| body |></p>\n"
	"</body>\n"
	"</html>"
);

int main(void) {
	const allocator mem = arenas_init(2049);

	templet_map templets = templet_maps_init();
	templet_error parse_error = templets_parse(&templets, &templ, &mem);
	if (parse_error != ok) {
		printf("error '%d' happened while parsing.\n", parse_error);
		goto cleanup0;
	}

	templet_maps_println(&templets);

	cleanup0:
	mem.debug(mem.storage);

	mems_free(&mem, null);
	return 0;
}
