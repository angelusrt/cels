#include "../source/templets.h"

#include "../source/templets.c"
#include "../source/strings.c"
#include "../source/vectors.c"
#include "../source/nodes.c"
#include "../source/mems.c"
#include "../source/errors.c"
#include "../source/maths.c"
#include "../source/files.c"
#include "../source/jsons.c"

static const string templ = strings_premake(
	"<| define index |>\n"
	"<html>\n"
	"<head>\n"
	"</head>\n"
	"<body>\n"
	"<h1><| title |></h1>\n"
	"<h2><| subtitle |></h2>\n"
	"<| for item in items |>\n"
		"<h2><| item.title |></h2>\n"
		"<h3><| item.subtitle |></h3>\n"
		"<p><| item.body |></p>\n"
		"<| for info in item.info |>\n"
			"<br>\n"
			"<h4>name: <| info.name |></h2>\n"
		"<| end |>\n"
	"<| end |>"
	"</body>\n"
	"</html>"
);

static const string option_json = strings_premake("{"
	"\"title\":\"fortnite\","
	"\"subtitle\":\"is the game, man\","
	"\"body\":\"*tun tun tun * * clap *\","
	"\"items\":[{"
		"\"title\":\"section-title\","
		"\"subtitle\":\"section-subtitle\","
		"\"body\":\"lorem ipsum\""
		"\"info\":[{\"name\":\"angelus\"},{\"name\":\"roberto\"}]"
	"}]"
"}");

int main(void) {
	const allocator mem = arenas_init(2049);

	templet_map templets = {0};
	maps_init(templets);

	templet_error parse_error = templets_parse(&templets, &templ, &mem);
	if (parse_error != ok) {
		printf("error '%d' happened while parsing.\n", parse_error);
		goto cleanup0;
	}

	printf("templet_generated:\n");

	templet_map_iterator it = {0};
	while (maps_next(&templets, &it)) {
		strings_println(&it.data->data.key);
		printf(":");
		templet_trees_println(&it.data->data.value);
		printf("\n");
	}

	printf("\n");

	estring document = templets_unmake_with(&templets, "index", &option_json, &mem);
	if (document.error != ok) {
		printf("error '%d' happened while unmaking.\n", document.error);
		goto cleanup0;
	}

	printf("templet:\n");
	strings_println(&document.value);

	cleanup0:
	printf("memory:\n");
	mem.debug(mem.storage);

	mems_free(&mem, null);
	return 0;
}
