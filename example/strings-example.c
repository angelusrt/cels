#include "../source/strings.h"
#include "../source/strings.c"

#include "../source/vectors.c"
#include "../source/errors.c"
#include "../source/mems.c"
#include "../source/utils.c"
#include "../source/nodes.c"

void strings_spliting_text() {
	printf("\n");
	printf(__func__);
	printf("\n");

	string text = strings_make("exemplo de texto", null);

	const string sep = strings_premake(" ");
	string_vec text_tokens = strings_split(&text, sep, 0, null);

	for (size_t i = 0; i < text_tokens.size; i++) {
		strings_println(&text_tokens.data[i]);
	}

	string_vecs_free(&text_tokens, null);
	strings_free(&text, null);
}

void strings_iterating_text() {
	printf("\n");
	printf(__func__);
	printf("\n");

	string text = strings_make("exemplo de texto", null);
	string sep = strings_premake(" ");

	string text_current = {0};
	while (!strings_next(&text, sep, &text_current)) {
		strings_println(&text_current);
	}

	strings_free(&text, null);
}

void string_maps_making() {
	printf("\n");
	printf(__func__);
	printf("\n");

	const allocator mem = stack_arenas_init(2048);
	string_map json = string_maps_init();

	string_maps_push_with(&json, "name", "angelus", &mem);
	string_maps_push_with(&json, "age", "10", &mem);

	string key = strings_premake("name");
	string *name = string_maps_get(&json, key);
	if (name != null) { 
		strings_println(name);
	}

	string_maps_push_with(&json, "name", "angelus", &mem);

	string_map_node *node = maps_get(&json, strings_hasherize(&key));
	size_t name_frequency = node ? node->frequency : 0;
	printf("name_frequency: %zu\n", name_frequency);

	string_maps_free(&json, &mem);

	mem.debug(mem.storage);
	mem.free(mem.storage);
}


int main(void) {
	strings_spliting_text();
	strings_iterating_text();
	string_maps_making();

	return 0;
}
