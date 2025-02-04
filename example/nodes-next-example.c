#include "../cels/source/nodes.h"
#include "../cels/source/strings.h"

#include "../cels/source/maths.c"
#include "../cels/source/errors.c"
#include "../cels/source/mems.c"
#include "../cels/source/vectors.c"
#include "../cels/source/nodes.c"
#include "../cels/source/strings.c"

int main(void) {
	const allocator mem = arenas_init(1024);

	string_map clients = string_maps_init();
	string_maps_push_with(&clients, "angelus", "{\"age\":\"21\"}", &mem);
	string_maps_push_with(&clients, "rodrigo", "{\"age\":\"32\"}", &mem);
	string_maps_push_with(&clients, "izabela", "{\"age\":\"83\"}", &mem);

	bynary_node_iterator it = {0};
	while (bynary_nodes_next((bynary_node *)clients.data, &it)) {
		string_key_pair *item = &((string_map_bynary_node *)it.data)->data;
		strings_print(&item->key);
		printf(": ");
		strings_println(&item->value);
	}

	mems_free(&mem, null);
	return 0;
}
