#include "./src/utils.c"
#include "./src/errors.c"
#include "./src/mems.c"
#include "./src/vectors.c"
#include "./src/strings.c"
#include "./src/nodes.c"
#include "./src/files.c"

#include "./src/jsons.h"
#include "./src/jsons.c"

int main(void) {
	const allocator mem = arenas_init(4096);

	const string json = strings_premake("{"
		"\"nome\":\"angelus\","
		"\"idade\":21,"
		"\"outros\":{\"cpf\":\"09230808482\"}"
	"}");

	strings_println(&json);
	printf("\n");

	estring_map json_map = jsons_unmake(&json, &mem);
	if (json_map.error != json_successfull) {
		goto cleanup;
	}

	string_maps_print(&json_map.value);
	printf("\n\n");

	const string key = strings_premake("outros");
	string *value = string_maps_get(&json_map.value, key);
	if (!value) {
		goto cleanup;
	}

	strings_println(value);
	printf("\n");

	estring_map outros_map = jsons_unmake(value, &mem);
	if (outros_map.error != json_successfull) {
		goto cleanup;
	}

	string_maps_print(&outros_map.value);
	printf("\n");

	estring new_json = jsons_make(&json_map.value, &mem);
	if (new_json.error != json_successfull) {
		goto cleanup;
	}

	strings_println(&new_json.value);
	printf("\n");

	mem.free(mem.storage);
	return 0;

	cleanup:
	mem.free(mem.storage);
	printf("error happened \n");
	return -1;
}
