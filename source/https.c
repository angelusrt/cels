#include "https.h"
#include "bytes.h"
#include "nodes.h"


/* constants */

const byte_vec line_sep = byte_vecs_premake("\r\n");
const byte_vec section_sep = byte_vecs_premake("\r\n\r\n");
const byte_vec token_sep = byte_vecs_premake(" ");
const byte_vec prop_sep = byte_vecs_premake(": ");
const byte_vec route_sep = byte_vecs_premake("/");

const string route_var_sep = strings_premake(":");

const string variable_charset = strings_premake(
	"abcdefghijklmnopqrstuvwxyz0123456789_");
const string regex_charset = strings_premake(
	"abcdefghijklmnopqrstuvwxyz"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"0123456789+[]|()\\-*._");


/* router_privates */

void router_privates_free(router_private *self, const allocator *mem) {
	strings_free(&self->location, mem);

	if (self->name.data) {
		strings_free(&self->name, mem);
	}

	if (self->regex.__buffer) {
		regfree(&self->regex);
	}
}


/* routers */

bool routers_check(const router *self) {
	#if cels_debug
		errors_return("self.location", strings_check_extra(&self->location))	
	#else
		if (strings_check_extra(&self->location)) return true;
	#endif

	return false;
}


/* router_nodes */

bool router_nodes_check(const router_node *self) {
	return routers_check((router *)&self->data);
}


/* router_node_vecs */

error router_vecs_push_with(
	router_vec *self, 
	char *location, 
	httpfunc callback, 
	void *param, 
	const allocator *mem) {

	#if cels_debug
		errors_abort("self", vectors_check((const vector *)self));
		errors_abort("location", strs_check(location));
	#endif

	string loc = strings_encapsulate(location);
	router route = {.location=loc, .func=callback, .param=param};

	return vectors_push(self, &route, mem);
}

cels_warn_unused
router_node *router_nodes_find_hash_private(router_node *self, size_t hash) {
	/*#if cels_debug
		errors_abort("self", munodes_check((const munode *)self));
	#endif*/

	router_node *route = self;
	while (route) {
		#if cels_debug
			errors_abort("self.data[i]", router_nodes_check(route));
		#endif

		if (route->data.hash == hash) {
			return route;
		}

		route = route->left;
	}

	return null;
}


/* private */

#define http_header_size 3
#define http_method_hash_size 9
#define http_protocol_hash_size 2

static const string http_headers[http_header_size] = {
	strings_premake("Method"), 
	strings_premake("Location"), 
	strings_premake("Protocol") 
};

static size_t http_header_hashs[http_header_size] = {0};
static size_t http_method_hashs[http_method_hash_size] = {0};
static size_t http_protocol_hashs[http_protocol_hash_size] = {0};
static size_t http_body_hash = 0;

void https_initialize_private(void) {
	http_header_hashs[0] = strings_hash(&http_headers[0]);
	http_header_hashs[1] = strings_hash(&http_headers[1]);
	http_header_hashs[2] = strings_hash(&http_headers[2]);

	http_method_hashs[0] = strings_prehash("GET");
	http_method_hashs[1] = strings_prehash("POST");
	http_method_hashs[2] = strings_prehash("PUT");
	http_method_hashs[3] = strings_prehash("DELETE");
	http_method_hashs[4] = strings_prehash("PATCH");
	http_method_hashs[5] = strings_prehash("HEAD");
	http_method_hashs[6] = strings_prehash("OPTIONS");
	http_method_hashs[7] = strings_prehash("TRACE");
	http_method_hashs[8] = strings_prehash("CONNECT");

	http_protocol_hashs[0] = strings_prehash("HTTP/1.0");
	http_protocol_hashs[1] = strings_prehash("HTTP/1.1");

	http_body_hash = strings_prehash("Body");
}

cels_warn_unused
ebyte_map https_make_head_private(byte_vec *head, const allocator *mem) {
	#if cels_debug
		errors_abort("head", byte_vecs_check(head));
	#endif


	/* creating head (method, location protocol) */

	error err = ok;
	byte_mat head_props = byte_vecs_split(head, token_sep, 0, mem);

	if (head_props.size != http_header_size) {
		err = http_head_size_error;
		goto cleanup0;
	}


	/* check if head is correct */

	bool is_method_valid = false;
	if (byte_vecs_is_string(&head_props.data[0])) {
		size_t m_hash = strings_hash((string *)&head_props.data[0]);

		for (size_t i = 0; i < http_method_hash_size; i++) {
			if (http_method_hashs[i] == m_hash) {
				is_method_valid = true;
				break;
			}
		}
	}

	if (!is_method_valid) {
		err = http_method_invalid_error;
		goto cleanup0;
	}


	bool is_location_valid = 
		byte_vecs_is_string(&head_props.data[1]) && 
		head_props.data[1].data[0] == '/';

	if (!is_location_valid) {
		err = http_location_invalid_error;
		goto cleanup0;
	}


	bool is_protocol_valid = false;
	if (byte_vecs_is_string(&head_props.data[2])) {
		size_t p_hash = strings_hash((string *)&head_props.data[2]);

		for (size_t i = 0; i < http_protocol_hash_size; i++) {
			if (http_protocol_hashs[i] == p_hash) {
				is_protocol_valid = true;
				break;
			}
		}
	}

	if (!is_protocol_valid) {
		err = http_protocol_invalid_error;
		goto cleanup0;
	}


	/* creating map */

	byte_map header = {0};
	maps_init(header);


	byte_map_pair pair0 = {
		.key=strings_make("Method", mem), 
		.value=head_props.data[0]};

	error push_error = maps_push(
		&header, &pair0, http_header_hashs[0], mem);

	if (push_error) {
		strings_free(&pair0.key, mem);
		goto cleanup1;
	}


	byte_map_pair pair1 = {
		.key=strings_make("Location", mem), 
		.value=head_props.data[1]};

	push_error = maps_push(
		&header, &pair1, http_header_hashs[1], mem);

	if (push_error) {
		strings_free(&pair1.key, mem);
		goto cleanup1;
	}


	byte_map_pair pair2 = {
		.key=strings_make("Protocol", mem), 
		.value=head_props.data[2]};

	push_error = maps_push(
		&header, &pair2, http_header_hashs[2], mem);

	if (push_error) {
		strings_free(&pair2.key, mem);
		goto cleanup1;
	}


	/* cleanup */

	mems_dealloc(
		mem, 
		head_props.data, 
		head_props.type_size * head_props.capacity);

	return (ebyte_map) {.value=header};

	cleanup1:
	maps_free(&header, (freefunc)strings_free, null, mem);

	cleanup0:
	vectors_free(&head_props, (freefunc)byte_vecs_free, mem);
	return (ebyte_map) {.error=err};
}

cels_warn_unused
ebyte_map https_tokenize_private(byte_vec *request, const allocator *mem) {
	#if cels_debug
		errors_abort("request", byte_vecs_check(request));
	#endif

	/* TODO: reduce copies */

	error err = ok;

	if (byte_vecs_check(request)) {
		err = http_request_invalid_error;
		goto cleanup0;
	}

	byte_mat sections = byte_vecs_split(request, section_sep, 1, mem);
	byte_mat header_props = byte_vecs_split(&sections.data[0], line_sep, 0, mem);
	ebyte_map request_props = https_make_head_private(&header_props.data[0], mem);
	if (request_props.error != http_successfull) {
		err = request_props.error;
		goto cleanup1;
	}

	for (size_t i = 1; i < header_props.size; i++) {
		byte_mat terms = byte_vecs_split(
			&header_props.data[i], prop_sep, 1, mem);

		if (terms.size != 2) {
			err = http_property_size_invalid_error;

			vectors_free(&terms, (freefunc)byte_vecs_free, mem);
			goto cleanup2;
		}

		if (!byte_vecs_is_string(&terms.data[0])) {
			err = http_property_mal_formed_error;

			vectors_free(&terms, (freefunc)byte_vecs_free, mem);
			goto cleanup2;
		}

		byte_map_pair pair = {
			.key=*(string *)&terms.data[0], 
			.value=terms.data[1]};

		bool push_error = maps_push(
			&request_props.value, &pair, strings_hash(&pair.key), mem);

		if (push_error) {
			err = http_property_mal_formed_error;

			vectors_free(&terms, (freefunc)byte_vecs_free, mem);
			goto cleanup2;
		}

		mems_dealloc(mem, terms.data, terms.capacity * terms.type_size);
	}

	if (sections.size == 2) {
		byte_map_pair pair = {
			.key=strings_make("Body", mem), 
			.value=sections.data[1]};

		bool push_error = maps_push(
			&request_props.value, &pair, http_body_hash, mem);

		if (push_error) {
			strings_free(&pair.key, mem);
			goto cleanup2;
		}
	}


	byte_vecs_free(&sections.data[0], mem);
	mems_dealloc(mem, sections.data, sections.capacity * sections.type_size);

	vectors_free(&header_props, (freefunc)byte_vecs_free, mem);
	return request_props;

	cleanup2:
	maps_free(
		&request_props, 
		(freefunc)strings_free, 
		(freefunc)byte_vecs_free, 
		mem);

	cleanup1:
	vectors_free(&sections, (freefunc)byte_vecs_free, mem);
	vectors_free(&header_props, (freefunc)strings_free, mem);

	cleanup0:
	return (ebyte_map){.error=err};
}

cels_warn_unused
erouter_private https_find_route_private(
	router_tree *router, byte_map *request, const allocator *mem) {

	#if cels_debug
		errors_abort("request", !request);
		errors_abort("request", binodes_check((binode *)request->data));
	#endif

	error err = ok;
	byte_vec *location_value = maps_get(request, http_header_hashs[1]);
	if (!location_value && !byte_vecs_is_string(location_value)) { 
		return (erouter_private) {.error=http_location_invalid_error};
	}

	string *l = (string *)location_value;
	string *rs = (string *)&route_sep;

	string_vec routes = strings_split(l, *rs, 0, mem);

	if (location_value->data[0] == '/' && location_value->size == 2) {
		vectors_free(&routes, (freefunc)strings_free, mem);

		#if cels_debug
			string *loc = &router->data->data.location;

			errors_abort(
				"router.data[0]", 
				strings_equals(loc, &strings_do("/")));
		#endif

		return (erouter_private){.value=router->data->data};
	}

	size_t i = 0;
	router_node *route = router->data->down;
	size_t route_hash = strings_hash(&routes.data[i]);

	bool has_matched = false;
	while (route) {
		if (route->data.has_regex) {
			int regex_status = regexec(
				&route->data.regex,
				routes.data[i].data,
				0, null, 0);

			has_matched = regex_status == 0;

			if (has_matched) {
				string key = strings_clone(&route->data.name, mem);
				string value = strings_clone(&routes.data[i], mem);

				byte_vec v = strings_to_byte_vec(&value);
				byte_map_pair pair = {.key=key, .value=v};

				error push_error = maps_push(
					request, &pair, strings_hash(&pair.key), mem);

				if (push_error) {
					strings_free(&key, mem);
					strings_free(&value, mem);

					err = http_property_probably_duplicated_error;
					goto cleanup0;
				}
			}
		} else {
			if (route->data.hash == route_hash) {
				has_matched = true;
			}
		}

		if (has_matched) {
			if (i < routes.size - 1) {
				++i;
				route_hash = strings_hash(&routes.data[i]);
				route = route->down;
			} else if (i == routes.size - 1) {
				break;
			}
		} else {
			route = route->left;
		}

		if (!route) { 
			err = http_not_found_error;
			goto cleanup0; 
		}
	}

	vectors_free(&routes, (freefunc)strings_free, mem);
	return (erouter_private){.value=route->data};

	cleanup0:
	vectors_free(&routes, (freefunc)strings_free, mem);

	return (erouter_private){.error=err};
}

typedef struct client_param {
	int client;
	router_tree *routes;
	const allocator *mem;
} client_param;

void *https_handle_client_private(void *args) {
    client_param *arg = args;
	int client_descriptor = arg->client;
	router_tree *routes = arg->routes;
	const allocator *mem = arg->mem;


	/* Receiving request */
	
	//ebyte_vec request = byte_vecs_receive(client_descriptor, MSG_WAITALL, 1024, mem);
	byte_vec request = {0};
	vectors_init(&request, sizeof(byte), string_small_size, mem);
	long bytes = recv(
		client_descriptor, 
		request.data + request.size, 
		string_small_size, 0);

	if (bytes < 0) {
		#if cels_debug
			fprintf(
				stderr, 
				"recv-error: %s (%d), client_descriptor: %d\n", 
				strerror(errno), 
				errno, 
				client_descriptor);
		#endif

		goto cleanup0; 
	} 

	request.size += bytes;
	error push_error = vectors_push(&request, &(byte){'\0'}, mem);
	if (push_error) { goto cleanup0; }

	#if cels_debug
		printf("request:\n");
		byte_vecs_print(&request);
		printf("\n");
	#endif


	/* tokenizing */

	ebyte_map request_props = https_tokenize_private(
		&request, mem);

	if (request_props.error != http_successfull) { 
		#if cels_debug
			fprintf(
				stderr, 
				colors_error("client '%d' had error '%d'\n"), 
				client_descriptor,
				request_props.error);
		#endif

		goto cleanup1; 
	} 

	#if cels_debug
		printf("request: \n");
		maps_print(
			&request_props.value, 
			(printfunc)strings_print, 
			(printfunc)byte_vecs_print);
		printf("\n");
	#endif


	/* routing */

	erouter_private callback = https_find_route_private(
		routes, &request_props.value, mem);

	if (callback.error != http_successfull) {
		#if cels_debug
			fprintf(
				stderr, 
				colors_error("'%d' had error '%d'\n"), 
				client_descriptor,
				callback.error);
		#endif

		if (callback.error == http_not_found_error) {
			if (routes->data && routes->data->down) {
				callback.value = routes->data->down->data;
			} else {
				https_send_not_found(
					null, client_descriptor, null);

				goto cleanup2;
			}
		} else {
			https_send_not_found(
				null, client_descriptor, null);

			goto cleanup2;
		}
	} 

	errors_abort("callback.func", !callback.value.func);
	callback.value.func(
		&request_props.value, 
		client_descriptor, 
		callback.value.param);


	cleanup2:
	maps_free(
		&request_props.value, 
		(freefunc)strings_free, 
		(freefunc)byte_vecs_free, 
		mem);

	cleanup1:
	vectors_free(&request, null, mem);

	cleanup0:
	printf("m\n");
    close(client_descriptor);

    return null;
}

router_private https_find_root_private(router_vec *callbacks) {
	#if cels_debug
		errors_abort("callbacks", vectors_check((const vector *)callbacks));
	#endif

	for (size_t i = 0; i < callbacks->size; i++) {
		string *l = &callbacks->data[i].location;
		if (l->size == 2 && l->data[0] == '/') {
			router r = callbacks->data[i];

			return (router_private) {
				.location=r.location,
				.param=r.param,
				.func=r.func
			};
		}
	}

	return (router_private) {0};
}

error https_insert_route_private(
	router_tree router, 
	router_node **route, 
	const string node_name, 
	const struct router callback,
	bool is_last,
	const allocator *mem) {

	size_t hash = strings_hash(&node_name);
	router_node *hash_route = router_nodes_find_hash_private(*route, hash);

	if (hash_route) {
		router_private *r = &hash_route->data;
		if (!r->func && is_last) {
			r->param = callback.param;
			r->func = callback.func;
		} else if (r->func && is_last) {
			return http_route_collision_error;
		}

		*route = hash_route;
	} else {
		string loc = strings_clone(&node_name, mem);
		router_node node = {
			.data={
				.location=loc,
				.has_regex=false,
				.hash=hash,
			}
		};

		if (is_last) {
			node.data.param = callback.param;
			node.data.func = callback.func;
		} 

		router_node *node_capsule = mems_alloc(mem, sizeof(router_node));
		errors_abort("node_capsule", !node_capsule);

		*node_capsule = node;
		if (*route && (*route)->down) {
			mutrees_push(&router, (*route)->down, node_capsule);
		} else {
			mutrees_attach(&router, *route, node_capsule);
		}

		*route = node_capsule;
	}

	return ok;
}

error https_insert_route_with_regex_private(
	router_tree router, 
	router_node **route, 
	const string node_name, 
	const struct router callback,
	bool is_last,
	const string_vec var_terms, 
	const allocator *mem) {

	bool is_regex_valid = strings_check_charset(
		&var_terms.data[1], regex_charset);

	if (!is_regex_valid) {
		return http_route_regex_invalid_error;
	}

	size_t hash = strings_hash(&var_terms.data[0]);
	string name = strings_format(
		"vars_%s", mem, var_terms.data[0].data);

	router_node *hash_route = router_nodes_find_hash_private(
		*route, hash);

	regex_t regex = {0};
	int reg_status = regcomp(
		&regex, var_terms.data[1].data, REG_EXTENDED);

	if (reg_status != 0) {
		strings_free(&name, mem);
		return http_route_regex_invalid_error;
	}

	if (hash_route) {
		router_private *router = &hash_route->data;
		bool has_regex = router->has_regex;

		if (!has_regex && is_last) {
			router->has_regex = true;
			router->regex = regex;
			router->name = name;

			bool has_func_added = router->func;
			if (has_func_added) {
				strings_free(&name, mem);
				return http_route_collision_error;
			}

			router->param = callback.param;
			router->func = callback.func;
		} else {
			regfree(&regex);
			strings_free(&name, mem);
		}

		*route = hash_route;
	} else {
		string loc = strings_clone(&node_name, mem);
		router_node node = {
			.data={
				.location=loc,
				.has_regex=true,
				.regex=regex,
				.hash=hash,
				.name=name,
			}
		};

		if (is_last) {
			node.data.param = callback.param;
			node.data.func = callback.func;
		} 

		router_node *node_capsule = mems_alloc(mem, sizeof(router_node));
		errors_abort("node_capsule", !node_capsule);

		*node_capsule = node;
		if (*route && (*route)->down) {
			mutrees_push(&router, (*route)->down, node_capsule);
		} else {
			mutrees_attach(&router, *route, node_capsule);
		}

		*route = node_capsule;
	} 

	return ok;
}

/* #to-review */
cels_warn_unused
erouter_tree https_create_router_private(
	router_vec *callbacks, const allocator *mem) {

	#if cels_debug
		errors_abort(
			"callbacks", 
			vectors_check((const vector *)callbacks));
	#endif

	error err = ok;

	router_tree router = {0};
	mutrees_init(router);

	router_private index = https_find_root_private(callbacks);
	router_node node = { .data=index };

	router_node *node_capsule = mems_alloc(mem, sizeof(router_node));
	errors_abort("node_capsule", !node_capsule);

	*node_capsule = node;
	mutrees_push(&router, null, node_capsule);


	for (size_t i = 0; i < callbacks->size; i++) {
		#if cels_debug
			printf("callback: %zu/%zu\n", i, callbacks->size);
		#endif

		string location_normalized = strings_replace(
			&callbacks->data[i].location, 
			strings_do(" "), 
			strings_do(""), 
			0, mem);

		string *rs = (string *)&route_sep;
		string_vec location_terms = strings_split(
			&location_normalized, *rs, 0, mem);

		router_node *route = router.data;
		for (size_t j = 0; j < location_terms.size; j++) {
			string_vec var_terms = strings_split(
				&location_terms.data[j], route_var_sep, 0, mem);

			if (var_terms.size > 2) {
				vectors_free(&var_terms, (freefunc)strings_free, mem);
				err = http_route_name_mal_formed_error;
				goto cleanup1;
			}

			#if cels_debug
				vectors_debug(&var_terms, (printfunc)strings_print);
				printf("\n");
			#endif

			bool is_variable_valid = strings_check_charset(
				&var_terms.data[0], variable_charset);

			if (!is_variable_valid) {
				vectors_free(&var_terms, (freefunc)strings_free, mem);
				err = http_route_name_invalid_error;
				goto cleanup1;
			}

			error insert_error = ok;
			bool is_last = j == location_terms.size - 1;
			if (var_terms.size < 2) {
				insert_error = https_insert_route_private(
					router, 
					&route, 
					location_terms.data[j], 
					callbacks->data[i], 
					is_last, 
					mem);

				vectors_free(&var_terms, (freefunc)strings_free, mem);
			} else {
				insert_error = https_insert_route_with_regex_private(
					router, 
					&route, 
					location_terms.data[j], 
					callbacks->data[i], 
					is_last, 
					var_terms,
					mem);

				vectors_free(&var_terms, (freefunc)strings_free, mem);
			}

			if (insert_error) {
				err = insert_error;
				goto cleanup1;
			}
		}


		cleanup1:
		vectors_free(&location_terms, (freefunc)strings_free, mem);
		strings_free(&location_normalized, mem);

		if (err) { 
			goto cleanup0; 
		}
	}


	return (erouter_tree){.value=router};

	cleanup0:
	mutrees_free(&router, (freefunc)router_privates_free, mem);

	return (erouter_tree){.error=err};
}

http_error https_serve(short port, router_vec *callbacks, const allocator *mem) {
	#if cels_debug
		errors_abort("callbacks", vectors_check((const vector *)callbacks));
	#endif

	https_initialize_private();
	erouter_tree router = https_create_router_private(callbacks, mem);
	if (router.error != http_successfull) {
		return router.error;
	}

	#if cels_debug
		printf("\nroutes: \n");

		router_tree_iterator it = {0};
		while (mutrees_next(&router.value, &it)) {
			printf("location: ");
			strings_println(&it.data->data.location);
		}

		printf("\n");
	#endif
	
    struct sockaddr_in address = {
		.sin_family=AF_INET, 
		.sin_addr={.s_addr=INADDR_ANY},
		.sin_port=htons(port)
	};

    short socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_descriptor == -1) {
		return http_socket_failed_error;
	}

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    short bind_status = bind(
		socket_descriptor, 
		(struct sockaddr *)&address, 
		sizeof(address));

	if (bind_status == -1) {
		return http_bind_failed_error;
	}

	#define https_request_maximum 200
    short listen_status = listen(socket_descriptor, https_request_maximum);

	if (listen_status == -1) {
		return http_listen_failed_error;
	}
	#undef https_request_maximum

    while (true) {
		struct sockaddr_in client_address = {0};
		socklen_t client_address_size = sizeof(client_address);

		int client_descriptor = accept(
				socket_descriptor, 
				(struct sockaddr *)&client_address, 
				&client_address_size);

		if (client_descriptor == -1) { 
			#if cels_debug
				perror("https_serve.accept failed"); 
			#endif

			continue; 
		}

		pthread_t thread = {0};
		client_param param = {
			.client=client_descriptor, .routes=&router.value};

		pthread_create(&thread, null, https_handle_client_private, &param);
		pthread_detach(thread);
    }

    close(socket_descriptor);
}

void https_send_not_found(
	notused byte_map *request, int client_connection, notused void *param) {

	static const byte_vec not_found_page = byte_vecs_premake(
		"HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n"
		"<html>"
		"<head><title>not found</title></head>"
		"<body><h1>404</h1><h4>your page wasn't found</h4></body>"
		"</html>");

	send(client_connection, not_found_page.data, not_found_page.size - 1, 0);
}

void https_send(
	int client_connection, const byte_vec head, const byte_vec body) {

	send(client_connection, head.data, head.size - 1, 0);
	send(client_connection, body.data, body.size - 1, 0);
}
