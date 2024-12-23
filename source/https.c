#include "https.h"

const string line_sep = strings_premake("\r\n");
const string token_sep = strings_premake(" ");
const string attribute_sep = strings_premake(": ");
const string route_sep = strings_premake("/");
const string route_var_sep = strings_premake(":");

const string variable_charset = strings_premake(
	"abcdefghijklmnopqrstuvwxyz0123456789_");
const string regex_charset = strings_premake(
	"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789+[]|()\\-*._");

/* routers */

bool routers_check(const router *self) {
	#if cels_debug
		if (errors_check("routers_check.self.location", strings_check(&self->location))) {
			return true;
		}
	#else
		if (strings_check(&self->location)) {
			return true;
		}
	#endif

	return false;
}

void routers_debug(const router *self) {
	#if cels_debug
		errors_panic("routers_debug.self", routers_check(self));
	#endif

	printf(
		"<router>{.location=%s, .params=%p}\n",
		self->location.data, (void *)self->params);

	/*printf(
		"<router>{.location=%s, .func=%p, .params=%p}",
		self->location.data, self->func, self->params);*/
}

void routers_print(const router *self) {
	#if cels_debug
		errors_panic("routers_print.self", routers_check(self));
	#endif

	printf("%s\n", self->location.data);
}

bool routers_equals(const router *r0, const router *r1) {
	#if cels_debug
		errors_panic("routers_equals.r0", routers_check(r0));
		errors_panic("routers_equals.r1", routers_check(r1));
	#endif

	return strings_equals(&r0->location, &r1->location);
}

bool routers_seems(const router *r0, const router *r1) {
	#if cels_debug
		errors_panic("routers_seems.r0", routers_check(r0));
		errors_panic("routers_seems.r1", routers_check(r1));
	#endif

	return strings_seems(&r0->location, &r1->location);
}

/* router_vecs */

vectors_generate_implementation(
	router, 
	router_vec, 
	routers_check, 
	routers_print,
	routers_equals, 
	routers_seems,
	defaults_free)

bool router_vecs_make_push(router_vec *self, char *location, httpfunc callback, void *params, const allocator *mem) {
	#if cels_debug
		errors_panic("router_vecs_make_push.self", vectors_check((const vector *)self));
		errors_panic("router_vecs_make_push.location", location == null);
		errors_panic("router_vecs_make_push.#location", strlen(location) == 0);
	#endif

	size_t locsize = strlen(location) + 1;

	string loc = {.data=location, .size=locsize, .capacity=locsize};

	router r = {.location=loc, .func=callback, .params=params};
	return router_vecs_push(self, r, mem);
}

/* router_nodes */

bool router_nodes_check(const router_node *self) {
	return routers_check((router *)&self->data);
}

void router_nodes_debug(const router_node *self) {
	#if cels_debug
		errors_panic("router_nodes_debug.self", router_nodes_check(self));
	#endif

	printf(
		"%p<router_node>{.data=<router>{.location=%s, .has_regex=%d}, .next=%p}\n", 
		(void *)self, self->data.location.data, self->data.has_regex, (void *)self->next);
}

void router_nodes_full_debug_private(const router_node *self, size_t stackframe) {
	if (stackframe > nodes_max_recursion) { return; }

	router_nodes_debug(self);
	if (self->next == null) { return; }

	for (size_t i = 0; i < self->next->size; i++) {
		router_nodes_full_debug_private(&self->next->data[i], ++stackframe);
	}
}

void router_nodes_full_debug(const router_node *self) {
	#if cels_debug
		errors_panic("router_nodes_full_debug.self", router_nodes_check(self));
	#endif

	router_nodes_full_debug_private(self, 0);
}

void router_nodes_print(const router_node *self) {
	#if cels_debug
		errors_panic("router_nodes_print.self", router_nodes_check(self));
	#endif

	printf(
		"%s (%zu): %p\n", 
		self->data.location.data, 
		self->data.hash,
		(void *)self->next);
}

bool router_nodes_equals(const router_node *r0, const router_node *r1) {
	#if cels_debug
		errors_panic("router_nodes_equals.r0", router_nodes_check(r0));
		errors_panic("router_nodes_equals.r1", router_nodes_check(r1));
	#endif

	return strings_equals(&r0->data.location, &r1->data.location);
}

bool router_nodes_seems(const router_node *r0, const router_node *r1) {
	#if cels_debug
		errors_panic("router_nodes_seems.r0", router_nodes_check(r0));
		errors_panic("router_nodes_seems.r1", router_nodes_check(r1));
	#endif

	return strings_seems(&r0->data.location, &r1->data.location);
}

/* router_node_vecs */

vectors_generate_implementation(
	router_node, 
	router_node_vec,
	router_nodes_check,
	router_nodes_print,
	router_nodes_equals,
	router_nodes_seems,
	defaults_free)

__attribute_warn_unused_result__
long router_node_vecs_find_hash(const router_node_vec *self, size_t hash) {
	#if cels_debug
		errors_panic("router_node_vecs_find_hash.self", vectors_check((const vector *)self));
	#endif

	if (self->size == 0) return -1;

	for (size_t i = 0; i < self->size; i++) {
		#if cels_debug
			errors_panic(
				"router_node_vecs_find_hash.self.data[i]", 
				router_nodes_check(&self->data[i]));
			router_nodes_debug(&self->data[i]);
		#endif

		if (self->data[i].data.hash == hash) return i;
	}

	#if cels_debug
		printf("\n");
	#endif

	return -1;
}

/* https */

#define https_header_size 3
static const string https_header_attributes[https_header_size] = {
	strings_premake("Method"), strings_premake("Location"), strings_premake("Protocol") };

/*GET, POST, PUT, DELETE*/
static const size_vec https_possible_methods = vectors_premake(
	size_t, 4, 45458, 324184, 46305, 14117355,);

/*HTTP/1.1*/
static const size_vec https_possible_protocols = vectors_premake(
	size_t, 1, 327228069);

__attribute_warn_unused_result__
bool https_head_check(string_map *head) {
	#if cels_debug
		errors_panic("https_head_check.head", bnodes_check((bnode *)head));
	#endif

	//TODO: improve validation

	size_t length = bnodes_length((bnode *)head);
	errors_panic("https_head_check.head.size != 3", length != 3);

	bool is_method_valid = false;
	string *method_value = string_maps_get(head, https_header_attributes[0]);
	if (method_value != null) {
		size_t hash = strings_hasherize(method_value);
		ssize_t pos = size_vecs_find(&https_possible_methods, hash);

		if (pos > -1) { is_method_valid = true; }
	}

	bool is_location_valid = false;
	string *location_value = string_maps_get(head, https_header_attributes[1]);
	if (location_value != null && location_value->data[0] == '/') {
		is_location_valid = true;
	}

	bool is_protocol_valid = false;
	string *protocol_value = string_maps_get(head, https_header_attributes[2]);
	if (protocol_value != null) {
		size_t hash = strings_hasherize(protocol_value);
		ssize_t pos = size_vecs_find(&https_possible_protocols, hash);

		if (pos > -1) { is_protocol_valid = true; }
	}

	return is_method_valid && is_location_valid && is_protocol_valid;
}

__attribute_warn_unused_result__
string_map *https_tokenize(string *request) {
	if (errors_check("https_tokenize.request", strings_check_extra(request))) {
		goto invalid_request0;
	}

	string_vec attributes = strings_make_split(request, &line_sep, 0, null);
	if (errors_check("https_tokenize.attributes empty", attributes.size < 2)) {
		goto invalid_request1;
	}

	string_vec header = strings_make_split(&attributes.data[0], &token_sep, 0, null);
	if (errors_check("https_tokenize.header invalid", header.size != https_header_size)) {
		goto invalid_request2;
	}

	string_map *request_attributes = null;
	for (size_t i = 0; i < https_header_size; i++) {
		string_maps_push(&request_attributes, https_header_attributes[i], header.data[i], null);
	}

	bool is_head_valid = https_head_check(request_attributes);
	if (errors_check("https_tokenize.head invalid", is_head_valid == false)) {
		goto invalid_request3;
	}

	for (size_t i = 1; i < attributes.size; i++) {
		string_vec attribute = strings_make_split(&attributes.data[i], &attribute_sep, 0, null);

		if (i == attributes.size - 1 && attribute.size < 2) {
			string body_key = strings_premake("Body");
			string_maps_push(&request_attributes, body_key, attributes.data[i], null);
		} else if (attribute.size == 2) {
			string_maps_push(&request_attributes, attribute.data[0], attribute.data[1], null);
		}

		string_vecs_free(&attribute, null);
	}

	string_vecs_free(&header, null);
	string_vecs_free(&attributes, null);
	return request_attributes;

	invalid_request3:
		string_maps_free(request_attributes, null);
	invalid_request2:
		string_vecs_free(&header, null);
	invalid_request1:
		string_vecs_free(&attributes, null);
	invalid_request0:
		return null;
}

__attribute_warn_unused_result__
router_private *https_find_route(router_node *router, string_map *request) {
	//TODO: check params

	string *location_value = string_maps_get(request, https_header_attributes[1]);
	string_vec routes = strings_make_split(location_value, &route_sep, 0, null);
	
	if (routes.size == 0) { goto cleanup; }

	if (location_value->data[0] == '/' && location_value->size == 2) {
		string_vecs_free(&routes, null);
		return &router->data;
	}

	router_node_vec *router_current = router->next;
	for (size_t i = 0; i < routes.size; i++) {
		size_t route_hash = strings_hasherize(&routes.data[i]);

		bool has_matched = false;
		for (size_t j = 0; j < router_current->size; j++) {
			router_node *route = &router_current->data[j];
			if (route->data.has_regex) {
				int regex_stat = regexec(
					&router_current->data[j].data.regex,
					routes.data[i].data,
					0, NULL, 0);

				has_matched = regex_stat == 0;

				if (has_matched) {
					strings_println(&routes.data[i]);
					string_maps_push(&request, route->data.name, routes.data[i], null);
				}
			} else {
				if (router->data.hash == route_hash) {
					has_matched = true;
				}
			}

			if (has_matched && (i < routes.size - 1 || j < router_current->size - 1)) {
				router_current = router_current->data[j].next;
				break;
			}
		}

		if (has_matched && i == routes.size - 1) {
			string_vecs_free(&routes, null);
			return &router_current->data->data;
		} else if (has_matched && i <= routes.size - 1 ) {
			router_current = router_current->data->next;
		}

		if (!has_matched) {
			//Verify if '0' exists else default
			string_vecs_free(&routes, null);
			return &router->next->data->data;
		}

		if (i == routes.size - 1) {
			string_vecs_free(&routes, null);
			return &router_current->data->data;
		}
	}

	cleanup:
	string_vecs_free(&routes, null);

	return &router->data;
}

typedef struct https_handle_client_params {
	int client;
	router_node *routes;
} https_handle_client_params;

void *https_handle_client(void *args) {
    https_handle_client_params arg = *((https_handle_client_params *)args);
	router_node *routes = arg.routes;
	int client_fd = arg.client;

	string request = strings_init(https_request_size, null);
    ssize_t request_bytes = recv(client_fd, request.data, request.capacity, 0);
	#if cels_debug
		errors_warn("https_handle_client.recv error'ed", request_bytes < 0);
		if (request_bytes < 0) {
			fprintf(stderr, "recv: %s (%d), client_fd: %d\n", strerror(errno), errno, client_fd);
		} 
	#endif

    if(request_bytes > 0) {
		request.size = request_bytes + 1;

		string_map *request_attributes = https_tokenize(&request);
		if (request_attributes) {
			size_t size = bnodes_length((bnode *)request_attributes);
			if (size == 0) { goto cleanup; }
		}

		router_private *callback = https_find_route(routes, request_attributes);
		errors_panic("https_handle_client.callback.func", !callback->func);

		callback->func(request_attributes, client_fd, callback->params);

		cleanup:
		string_maps_free(request_attributes, null);
    }

	strings_free(&request, null);
    close(client_fd);
    return NULL;
}

router_private https_find_root(router_vec *callbacks) {
	#if cels_debug
		errors_panic("https_find_root callbacks", vectors_check((vector *)callbacks));
	#endif

	for (size_t i = 0; i < callbacks->size; i++) {
		string *l = &callbacks->data[i].location;
		if (l->size == 2 && l->data[0] == '/') {
			router r = callbacks->data[i];

			return (router_private) {
				.location=r.location,
				.params=r.params,
				.func=r.func
			};
		}
	}

	router r = callbacks->data[0];
	return (router_private) { .location=r.location, .params=r.params, .func=r.func };
}

__attribute_warn_unused_result__
router_node https_create_routes_private(router_vec *callbacks, const allocator *mem) {
	#if cels_debug
		errors_panic(
			"https_create_routes_private.callbacks", 
			vectors_check((const vector *)callbacks));
	#endif

	router_node_vec rnv = router_node_vecs_init(vectors_min, mem);
	router_node_vec *rnv_capsule = mems_alloc(mem, sizeof(router_node_vec));

	errors_panic("https_create_routes_private.rnv_capsule", rnv_capsule == null);
	*rnv_capsule = rnv;

	router_node r = { .data=https_find_root(callbacks), .next=rnv_capsule };

	for (size_t i = 0; i < callbacks->size; i++) {
		#if cels_debug
			printf("callback: %zu/%zu\n", i, callbacks->size);
		#endif

		string location_normalized = strings_make_replace(
			&callbacks->data[i].location, 
			&(string)strings_premake(" "), 
			null, 0, mem);

		string_vec location_terms = strings_make_split(&location_normalized, &route_sep, 0, mem);
		if (location_terms.size < 1) { goto cleanup0; }

		router_node *router_current = &r;
		for (size_t j = 0; j < location_terms.size; j++) {
			string_vec var_terms = strings_make_split(&location_terms.data[j], &route_var_sep, 0, mem);
			#if cels_debug
				string_vecs_debug(&var_terms);
			#endif

			errors_panic("https_create_routes_private.(var_terms.size > 2)", var_terms.size > 2);

			bool is_variable_valid = strings_check_charset(&var_terms.data[0], &variable_charset);
			errors_panic("https_create_routes_private.!is_variable_valid", !is_variable_valid);

			bool is_last = j == location_terms.size - 1;
			if (var_terms.size < 2) {
				size_t hash = strings_hasherize(&location_terms.data[j]);
				long hash_pos = router_node_vecs_find_hash(router_current->next, hash);

				if (hash_pos > -1) {
					router_private *r = &router_current->next->data[hash_pos].data;
					bool has_func = r->func != null;

					if (!has_func && is_last) {
						r->params = callbacks->data[i].params;
						r->func = callbacks->data[i].func;
					} else if (has_func && is_last){
						errors_panic("https_create_routes_private.(func != null) (already exists)", has_func);
					}
				} else if (hash_pos <= -1) {
					router_node_vec rnv = router_node_vecs_init(vectors_min, mem);
					router_node_vec *rnv_capsule = mems_alloc(mem, sizeof(router_node_vec));
					errors_panic("https_create_routes_private.rnv_capsule", rnv_capsule == null);
					*rnv_capsule = rnv;

					#if cels_debug
						strings_println(&location_terms.data[j]);
					#endif

					router_node node = {
						.next=rnv_capsule,
						.data={
							.location=strings_make_copy(&location_terms.data[j], mem), //Why?
							.has_regex=false,
							.hash=hash,
						}
					};

					if (is_last) {
						node.data.params = callbacks->data[i].params;
						node.data.func = callbacks->data[i].func;
					} 

					bool error = router_node_vecs_push(router_current->next, node, mem);
					#if cels_debug
						errors_panic("https_create_routes_private.error", error);
					#endif

					if (error) {
						strings_free(&node.data.location, mem);
						router_node_vecs_free(rnv_capsule, mem);
						mems_dealloc(mem, rnv_capsule, sizeof(router_node_vec*));
					}
				}

				hash_pos = router_node_vecs_find_hash(router_current->next, hash);
				errors_panic("https_create_routes_private.(hash_pos == -1) (1)", hash_pos == -1);

				router_current = &router_current->next->data[hash_pos];
				string_vecs_free(&var_terms, mem);
				continue;
			}

			bool is_regex_valid = strings_check_charset(&var_terms.data[1], &regex_charset);
			errors_panic("https_create_routes_private.!is_regex_valid", !is_regex_valid);

			size_t hash = strings_hasherize(&var_terms.data[0]);
			string name = strings_make_format("vars_%s", mem, var_terms.data[0].data);

			errors_panic("https_create_routes_private.router_current.next", !router_current->next);
			long hash_pos = router_node_vecs_find_hash(router_current->next, hash);
			bool is_empty = router_current->next->size == 0;

			regex_t regex;
			int reg_stat = regcomp(&regex, var_terms.data[1].data, REG_EXTENDED);
			errors_panic("https_create_routes_private.(regcomp != 0)", reg_stat != 0);

			if (hash_pos > -1) {
				router_private *router = &router_current->next->data[hash_pos].data;
				bool has_regex = router->has_regex;

				if (!has_regex && is_last) {
					router->has_regex = true;
					router->regex = regex;
					router->name = name;
				} else {
					regfree(&regex);
					strings_free(&name, mem);
				}
			} else if (hash_pos <= -1 && is_empty){
				router_node_vec rnv = router_node_vecs_init(vectors_min, mem);
				router_node_vec *rnv_capsule = mems_alloc(mem, sizeof(router_node_vec));
				errors_panic("https_create_routes_private.rnv_capsule", rnv_capsule == null);
				*rnv_capsule = rnv;

				router_node node = {
					.next=rnv_capsule,
					.data={
						.location=strings_make_copy(&var_terms.data[0], null),
						.has_regex=true,
						.regex=regex,
						.hash=hash,
						.name=name,
					}
				};

				bool error = router_node_vecs_push(router_current->next, node, mem);
				#if cels_debug
					errors_panic("https_create_routes_private.error", error);
				#endif

				if (error) {
					strings_free(&node.data.location, mem);
					router_node_vecs_free(rnv_capsule, mem);
					mems_dealloc(mem, rnv_capsule, sizeof(router_node_vec*));
				}

				errors_panic("https_create_routes_private.router_current.next", !router_current->next);
				hash_pos = router_node_vecs_find_hash(router_current->next, hash);

				errors_panic("https_create_routes_private.hash_pos == -1 (2)", hash_pos == -1);
			} else {
				errors_panic("https_create_routes_private.!is_empty (route colision)", !is_empty);
			}

			if (is_last) {
				router_private *router = &router_current->next->data[hash_pos].data;
				bool has_func_added = router->func != null;
				errors_panic("https_create_routes_private.!func (already added)", has_func_added);

				router->params = callbacks->data[i].params;
				router->func = callbacks->data[i].func;
			} 

			router_current = &router_current->next->data[hash_pos];
			string_vecs_free(&var_terms, mem);
		}

		cleanup0:
		string_vecs_free(&location_terms, mem);
		strings_free(&location_normalized, mem);
	}

	return r;
}

void https_serve(short port, router_vec *callbacks, const allocator *mem) {
	#if cels_debug
		errors_panic("https_serve.callbacks", vectors_check((const vector *)callbacks));
	#endif

	router_node routes = https_create_routes_private(callbacks, mem);

	//debug-init
	router_nodes_full_debug(&routes);
	printf("\n");
	//mem->debug(mem->storage);
	printf("\n");
	//debug-end
	
    struct sockaddr_in server_addr = {
		.sin_family=AF_INET, 
		.sin_addr={.s_addr=INADDR_ANY},
		.sin_port=htons(port)
	};

    short server_fd = socket(AF_INET, SOCK_STREAM, 0);
	errors_panic("https_serve.socket failed", server_fd == -1);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    short bind_status = bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	errors_panic("https_serve.bind failed", bind_status == -1);

    short listen_status = listen(server_fd, https_requests_maximum);
	errors_panic("https_serve.listen failed", listen_status == -1);

    while (1) {
		struct sockaddr_in client_addr;
		socklen_t client_addr_len = sizeof(client_addr);

		int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
		if (client_fd == -1) { 
			#if cels_debug
				perror("https_serve.accept failed"); 
			#endif

			continue; 
		}

		pthread_t thread_id;
		https_handle_client_params params = {.client=client_fd, .routes=&routes};
		pthread_create(&thread_id, NULL, https_handle_client, (void *)&params);
		pthread_detach(thread_id);
    }

    close(server_fd);
}

void https_default_not_found(unused string_map *request, int client_connection, unused void *params) {
	string not_found_page = strings_premake(
		"HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n"
		"<html>"
		"<head><title>not found</title></head>"
		"<body><h1>404</h1><h4>your page wasn't found</h4></body>"
		"</html>");

	send(client_connection, not_found_page.data, not_found_page.size, 0);
}

void https_send(int client_connection, const string *body, const string *head) {
	//TODO: validate params

	send(client_connection, head->data, head->size, 0);
	send(client_connection, body->data, body->size, 0);
}
