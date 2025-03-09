#include "../source/https.h"
#include "../source/files.h"
#include "../source/strings.h"
#include "../source/vectors.h"
#include "../source/bytes.h"

#include "../source/https.c"
#include "../source/files.c"
#include "../source/errors.c"
#include "../source/vectors.c"
#include "../source/strings.c"
#include "../source/bytes.c"
#include "../source/utils.c"
#include "../source/mems.c"
#include "../source/nodes.c"
#include "../source/maths.c"

static const byte_vec default_response = byte_vecs_premake("not_found :(");

void handle_index(
	notused byte_map *request, int response, notused void *params) {

	byte_vec message = byte_vecs_premake("Hello World");
	https_send(
		response, 
		https_default_head,
		message);
}

void handle_hello(
	notused byte_map *request, int response, notused void *params) {

	byte_vec message = byte_vecs_premake("Hello World2");
	https_send(
		response, 
		https_default_head,
		message);
}

void handle_wild(
	notused byte_map *request, int response, notused void *params) {

	const allocator mem = arenas_init(2048);

	file *index_html = fopen(
		"." file_sep "static" file_sep "index.html", "r");

	if (!index_html) { 
		printf(colors_error("handle_wild.index_html não abriu"));
		goto cleanup0;
	}

	ebyte_vec file_read = files_read(index_html, &mem);
	fclose(index_html);

	if (file_read.error != ok) {
		printf("error: %d", file_read.error);
		https_send(response, https_default_head, default_response);
		goto cleanup0;
	}

	https_send(response, https_default_head, file_read.value);

	cleanup0:
	mems_free(&mem, null);
}

typedef struct wild2_param { byte_vec file; } wild2_param;
void handle_wild2(
	notused byte_map *request, int response, void *params) {

	wild2_param *arg = params;
	https_send(response, https_default_head, arg->file);
}

void handle_hallo(
	byte_map *request, int response, notused void *params) {

	size_t key_hash = strings_prehash("vars_country");
	byte_vec *country = maps_get(request, key_hash);
	if (!country) {
		https_send(response, https_default_head, default_response);
		return;
	}

	https_send(response, https_default_head, *country);
}

void handle_file(
	byte_map *request, int response, notused void *params) {

	size_t key_hash = strings_prehash("vars_file");
	byte_vec *filename = maps_get(request, key_hash);

	if (!filename) {
		https_send(response, https_default_head, default_response);
		return;
	}

	https_send(response, https_default_head, *filename);
}

int main(void) {
	allocator mem = arenas_init(4096);

	file *index_html = fopen("static/index.html", "r");
	errors_panic("main.index_html não abriu)", index_html == null);

	ebyte_vec file_read = files_read(index_html, &mem);
	fclose(index_html);

	if (file_read.error != file_successfull) {
		printf("file_read_error: %d\n", file_read.error);
		goto cleanup0;
	}


	router_vec calls = {0};
	vectors_init(&calls, sizeof(router), vector_min, &mem);


	wild2_param p = {file_read.value};
	char hallo_path[] = "/api/country:[a-z]+/city:[a-z]+";
	char file_path[] = "/static/file:[a-z]+\\.(txt|jpg)";

	router routes[] = {
		{.location=s("/not_found"), .func=https_send_not_found},
		{.location=s("/"), .func=handle_index},
		{.location=s("/hello"), .func=handle_hello},
		{.location=s("/wild"), .func=handle_wild},
		{.location=s("/wild2"), .func=handle_wild2, .param=&p},
		{.location=s(hallo_path), .func=handle_hallo},
		{.location=s(file_path), .func=handle_file},
	};

	for (size_t i = 0; i < sizeof(routes)/sizeof(router); i++) {
		vectors_push(&calls, &routes[i], &mem);
	}


	http_error serve_error = https_serve(8080, &calls, &mem);
	if (serve_error) {
		printf("serve_error: %d\n", serve_error);
	}


	cleanup0:
	mems_free(&mem, null);
	return 0;
}
