#include "../source/utils.c"
#include "../source/errors.c"
#include "../source/mems.c"
#include "../source/vectors.c"
#include "../source/strings.c"
#include "../source/nodes.c"
#include "../source/files.c"

#include "../source/jsons.h"
#include "../source/jsons.c"

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

	string_map_iterator it0 = {0};
	while (maps_next(&json_map.value, &it0)) {
		strings_print(&it0.data->data.key);
		printf(":");
		strings_print(&it0.data->data.value);
		printf("\n");
	}

	printf("\n");

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

	string_map_iterator it = {0};
	while (maps_next(&outros_map.value, &it)) {
		strings_print(&it.data->data.key);
		printf(":");
		strings_print(&it.data->data.value);
		printf("\n");
	}
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
