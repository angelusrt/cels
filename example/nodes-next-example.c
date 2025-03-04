#include "../source/nodes.h"
#include "../source/strings.h"

#include "../source/maths.c"
#include "../source/errors.c"
#include "../source/mems.c"
#include "../source/vectors.c"
#include "../source/nodes.c"
#include "../source/strings.c"

int main(void) {
	const allocator mem = arenas_init(1024);

	string_map clients = {0};
	maps_init(clients);

	string_maps_push_with(&clients, "angelus", "{\"age\":\"21\"}", &mem);
	string_maps_push_with(&clients, "rodrigo", "{\"age\":\"32\"}", &mem);
	string_maps_push_with(&clients, "izabela", "{\"age\":\"83\"}", &mem);

	string_map_iterator it = {0};
	while (maps_next(clients.data, &it)) {
		string_map_pair item = it.data->data;
		strings_print(&item.key);
		printf(": ");
		strings_println(&item.value);
	}

	mems_free(&mem, null);
	return 0;
}
