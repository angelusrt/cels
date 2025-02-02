#include "https.h"
#include "strings.h"

const string line_sep = strings_premake("\r\n");
const string token_sep = strings_premake(" ");
const string attribute_sep = strings_premake(": ");
const string route_sep = strings_premake("/");
const string route_var_sep = strings_premake(":");

const string variable_charset = strings_premake(
	"abcdefghijklmnopqrstuvwxyz0123456789_");
const string regex_charset = strings_premake(
	"abcdefghijklmnopqrstuvwxyz"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"0123456789+[]|()\\-*._");

/* routers */

bool routers_check(const router *self) {
	#if cels_debug
		errors_return("self.location", strings_check(&self->location))	
	#else
		if (strings_check(&self->location)) return true;
	#endif

	return false;
}

void routers_debug(const router *self) {
	#if cels_debug
		errors_abort("self", routers_check(self));
	#endif

	printf(
		"<router>{.location=%s, .params=%p}\n",
		self->location.data, (void *)self->params);
}

router routers_clone(router *self, const allocator *mem) {
	router other = *self;
	other.location = strings_clone(&self->location, mem);

	return other;
}

void routers_print(const router *self) {
	#if cels_debug
		errors_abort("self", routers_check(self));
	#endif

	printf("%s\n", self->location.data);
}

bool routers_equals(const router *self, const router *other) {
	#if cels_debug
		errors_abort("self", routers_check(self));
		errors_abort("other", routers_check(other));
	#endif

	return strings_equals(&self->location, &other->location);
}

bool routers_seems(const router *self, const router *other) {
	#if cels_debug
		errors_abort("self", routers_check(self));
		errors_abort("other", routers_check(other));
	#endif

	return strings_seems(&self->location, &other->location);
}

/* router_vecs */

vectors_generate_implementation(
	router, 
	router_vec, 
	routers_check, 
	routers_clone,
	routers_print,
	routers_debug,
	routers_equals, 
	routers_seems,
	defaults_free)

bool router_vecs_make_push(
	router_vec *self, 
	char *location, 
	httpfunc callback, 
	void *params, 
	const allocator *mem
) {
	#if cels_debug
		errors_abort("self", vectors_check((const vector *)self));
		errors_abort("location", location == null);
		errors_abort("#location", strlen(location) == 0);
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
		errors_abort("self", router_nodes_check(self));
	#endif

	printf(
		"%p<router_node>{.data=<router>{.location=%s, .has_regex=%d}, .next=%p}\n", 
		(void *)self, self->data.location.data, self->data.has_regex, (void *)self->next);
}

void router_nodes_full_debug_private(const router_node *self, size_t stackframe) {
	if (stackframe > cels_max_recursion) { return; }

	router_nodes_debug(self);
	if (self->next == null) { return; }

	for (size_t i = 0; i < self->next->size; i++) {
		router_nodes_full_debug_private(&self->next->data[i], ++stackframe);
	}
}

void router_nodes_full_debug(const router_node *self) {
	#if cels_debug
		errors_abort("self", router_nodes_check(self));
	#endif

	router_nodes_full_debug_private(self, 0);
}

void router_nodes_print(const router_node *self) {
	#if cels_debug
		errors_abort("self", router_nodes_check(self));
	#endif

	printf(
		"%s (%zu): %p\n", 
		self->data.location.data, 
		self->data.hash,
		(void *)self->next);
}

bool router_nodes_equals(const router_node *self, const router_node *other) {
	#if cels_debug
		errors_abort("self", router_nodes_check(self));
		errors_abort("other", router_nodes_check(other));
	#endif

	return strings_equals(&self->data.location, &other->data.location);
}

bool router_nodes_seems(const router_node *self, const router_node *other) {
	#if cels_debug
		errors_abort("self", router_nodes_check(self));
		errors_abort("other", router_nodes_check(other));
	#endif

	return strings_seems(&self->data.location, &other->data.location);
}

/* router_node_vecs */

vectors_generate_implementation(
	router_node, 
	router_node_vec,
	router_nodes_check,
	defaults_clone, 
	router_nodes_print,
	router_nodes_debug,
	router_nodes_equals,
	router_nodes_seems,
	defaults_free)

cels_warn_unused
long router_node_vecs_find_hash(const router_node_vec *self, size_t hash) {
	#if cels_debug
		errors_abort("self", vectors_check((const vector *)self));
	#endif

	if (self->size == 0) return -1;

	for (size_t i = 0; i < self->size; i++) {
		#if cels_debug
			errors_abort(
				"self.data[i]", 
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

#define header_size 3
static const string headers[header_size] = {
	strings_premake("Method"), 
	strings_premake("Location"), 
	strings_premake("Protocol") 
};

static const size_vec methods = vectors_premake(size_t, 4);
static const size_vec protocols = vectors_premake(size_t, 2);

void https_initialize_private(void) {
	methods.data[0] = strings_prehash("GET");
	methods.data[1] = strings_prehash("POST");
	methods.data[2] = strings_prehash("PUT");
	methods.data[3] = strings_prehash("DELETE");
	protocols.data[0] = strings_prehash("HTTP/1.0");
	protocols.data[1] = strings_prehash("HTTP/1.1");
}

cels_warn_unused
bool https_head_check(const string_map *head) {
	#if cels_debug
		errors_abort("head", bnodes_check((bnode *)head));
	#endif

	//TODO: improve validation

	size_t length = bnodes_length((bnode *)head);
	errors_abort("head.size != 3", length != header_size);

	bool is_method_valid = false;
	string *method_value = string_maps_get(head, headers[0]);
	if (method_value != null) {
		size_t hash = strings_hasherize(method_value);
		ssize_t pos = size_vecs_find(&methods, hash);

		if (pos > -1) { is_method_valid = true; }
	}

	bool is_location_valid = false;
	string *location_value = string_maps_get(head, headers[1]);
	if (location_value != null && location_value->data[0] == '/') {
		is_location_valid = true;
	}

	bool is_protocol_valid = false;
	string *protocol_value = string_maps_get(head, headers[2]);
	if (protocol_value != null) {
		size_t hash = strings_hasherize(protocol_value);
		ssize_t pos = size_vecs_find(&protocols, hash);

		if (pos > -1) { is_protocol_valid = true; }
	}

	return is_method_valid && is_location_valid && is_protocol_valid;
}

cels_warn_unused
string_map *https_tokenize(string *request, const allocator *mem) {
	if (errors_check("https_tokenize.request", strings_check_extra(request))) {
		goto invalid_request0;
	}

	string_vec attributes = strings_split(request, line_sep, 0, mem);
	if (errors_check("https_tokenize.attributes empty", attributes.size < 2)) {
		goto invalid_request1;
	}

	string_vec header = strings_split(&attributes.data[0], token_sep, 0, mem);
	if (errors_check("https_tokenize.header invalid", header.size != header_size)) {
		goto invalid_request2;
	}

	string_map *request_attributes = null;
	for (size_t i = 0; i < header_size; i++) {
		string key = strings_clone(&headers[i], mem);
		bool push_status = string_maps_push(
			request_attributes, key, header.data[i], mem);

		if (push_status) {
			strings_free(&key, mem);
			strings_free(&header.data[i], mem);
		}
	}

	bool is_head_valid = https_head_check(request_attributes);
	if (errors_check("https_tokenize.head invalid", is_head_valid == false)) {
		goto invalid_request3;
	}

	for (size_t i = 1; i < attributes.size; i++) {
		string_vec attribute = strings_split(&attributes.data[i], attribute_sep, 0, mem);

		if (i == attributes.size - 1 && attribute.size < 2) {
			string body_key = strings_make("Body", mem);
			string value = strings_clone(&attributes.data[i], mem);

			bool push_status = string_maps_push(
				request_attributes, body_key, value, mem);

			if (push_status) {
				strings_free(&body_key, mem);
				strings_free(&value, mem);
			}

			mems_dealloc(mem, attribute.data, attribute.capacity);
		} else if (attribute.size == 2) {
			bool push_status = string_maps_push(
				request_attributes, attribute.data[0], attribute.data[1], mem);

			if (push_status) {
				strings_free(&attribute.data[0], mem);
				strings_free(&attribute.data[1], mem);
			}

			mems_dealloc(mem, attribute.data, attribute.capacity);
		} else {
			string_vecs_free(&attribute, mem);
		}
	}

	//string_vecs_free(&header, mem);
	mems_dealloc(mem, header.data, header.capacity);
	string_vecs_free(&attributes, mem);
	return request_attributes;

	invalid_request3:
		string_maps_free(request_attributes, mem);
		mems_dealloc(mem, header.data, header.capacity);
		string_vecs_free(&attributes, mem);
		return null;

	invalid_request2:
		string_vecs_free(&header, mem);
	invalid_request1:
		string_vecs_free(&attributes, mem);
	invalid_request0:
		return null;
}

cels_warn_unused
router_private *https_find_route(router_node *router, string_map *request, const allocator *mem) {
	//TODO: check params

	string *location_value = string_maps_get(request, headers[1]);
	if (!location_value) { return &router->data; }

	string_vec routes = strings_split(location_value, route_sep, 0, mem);
	if (routes.size == 0) { goto cleanup; }

	if (location_value->data[0] == '/' && location_value->size == 2) {
		string_vecs_free(&routes, mem);
		return &router->data;
	}

	router_node_vec *router_current = router->next;
	for (size_t i = 0; i < routes.size; i++) {
		size_t route_hash = strings_hasherize(&routes.data[i]);

		bool has_matched = false;
		for (size_t j = 0; j < router_current->size; j++) {
			router_node *route = &router_current->data[j];

			if (route->data.has_regex) {
				int regex_status = regexec(
					&route->data.regex,
					routes.data[i].data,
					0, null, 0);

				has_matched = regex_status == 0;

				if (has_matched) {
					string key = strings_clone(&route->data.name, mem);
					string value = strings_clone(&routes.data[i], mem);
					bool push_status = string_maps_push(request, key, value, mem);

					if (push_status) {
						strings_free(&key, mem);
						strings_free(&value, mem);
					}
				}
			} else {
				if (route->data.hash == route_hash) {
					has_matched = true;
				}
			}

			if (has_matched && (i < routes.size - 1)) {
				router_current = route->next;
				break;
			} else if (has_matched && (i == routes.size - 1)) {
				string_vecs_free(&routes, mem);
				return &route->data;
			}
		}

		if (!has_matched) {
			//Verify if '0' exists else default
			string_vecs_free(&routes, mem);
			return &router->next->data[0].data;
		}

		if (i == routes.size - 1) {
			string_vecs_free(&routes, mem);
			return &router_current->data[0].data;
		}
	}

	cleanup:
	string_vecs_free(&routes, mem);

	return &router->data;
}

typedef struct https_handle_client_params {
	int client;
	router_node *routes;
	const allocator *mem;
} https_handle_client_params;

void *https_handle_client(void *args) {
    https_handle_client_params arg = *((https_handle_client_params *)args);
	router_node *routes = arg.routes;
	int client_descriptor = arg.client;
	const allocator *mem = arg.mem;

	#define https_request_size 1024
	string request = strings_init(https_request_size, mem);
    ssize_t request_bytes = recv(
		client_descriptor, 
		request.data, 
		request.capacity, 
		0);

	#if cels_debug
		errors_warn("https_handle_client.recv error'ed", request_bytes < 0);
		if (request_bytes < 0) {
			fprintf(
				stderr, 
				"recv: %s (%d), client_descriptor: %d\n", 
				strerror(errno), 
				errno, 
				client_descriptor);
		} 
	#endif

    if(request_bytes > 0) {
		request.size = request_bytes + 1;

		string_map *request_attributes = https_tokenize(&request, mem);

		if (!request_attributes) { 
			goto cleanup0; 
		}

		if (request_attributes) {
			size_t size = bnodes_length((bnode *)request_attributes);
			if (size == 0) { 
				goto cleanup1; 
			}
		}

		router_private *callback = https_find_route(routes, request_attributes, mem);
		errors_abort("callback.func", !callback->func);
		callback->func(request_attributes, client_descriptor, callback->params);

		cleanup1:
		string_maps_free(request_attributes, mem);
    }

	cleanup0:
	strings_free(&request, mem);
    close(client_descriptor);
    return null;
}

router_private https_find_root_private(router_vec *callbacks) {
	#if cels_debug
		errors_abort(
			"callbacks", 
			vectors_check((vector *)callbacks));
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
	return (router_private) {
		.location=r.location, 
		.params=r.params, 
		.func=r.func 
	};
}

cels_warn_unused
router_node https_create_routes_private(router_vec *callbacks, const allocator *mem) {
	#if cels_debug
		errors_abort(
			"callbacks", 
			vectors_check((const vector *)callbacks));
	#endif

	router_node_vec rnv = router_node_vecs_init(vector_min, mem);
	router_node_vec *rnv_capsule = mems_alloc(mem, sizeof(router_node_vec));

	errors_abort("rnv_capsule", rnv_capsule == null);
	*rnv_capsule = rnv;

	router_node r = { .data=https_find_root_private(callbacks), .next=rnv_capsule };

	for (size_t i = 0; i < callbacks->size; i++) {
		#if cels_debug
			printf("callback: %zu/%zu\n", i, callbacks->size);
		#endif

		string location_normalized = strings_replace(
			&callbacks->data[i].location, 
			strings_do(" "), 
			strings_do(""), 
			0, mem);

		string_vec location_terms = strings_split(&location_normalized, route_sep, 0, mem);
		if (location_terms.size < 1) { goto cleanup0; }

		router_node *router_current = &r;
		for (size_t j = 0; j < location_terms.size; j++) {
			string_vec var_terms = strings_split(&location_terms.data[j], route_var_sep, 0, mem);
			#if cels_debug
				string_vecs_debug(&var_terms);
			#endif

			errors_abort("var_terms.size > 2", var_terms.size > 2);

			bool is_variable_valid = strings_check_charset(&var_terms.data[0], variable_charset);
			errors_abort("!is_variable_valid", !is_variable_valid);

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
						errors_abort("func != null (already exists)", has_func);
					}
				} else if (hash_pos <= -1) {
					router_node_vec rnv = router_node_vecs_init(vector_min, mem);
					router_node_vec *rnv_capsule = mems_alloc(mem, sizeof(router_node_vec));
					errors_abort("rnv_capsule", rnv_capsule == null);
					*rnv_capsule = rnv;

					#if cels_debug
						strings_println(&location_terms.data[j]);
					#endif

					router_node node = {
						.next=rnv_capsule,
						.data={
							.location=strings_clone(&location_terms.data[j], mem), //Why?
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
						errors_abort("error", error);
					#endif

					if (error) {
						strings_free(&node.data.location, mem);
						router_node_vecs_free(rnv_capsule, mem);
						mems_dealloc(mem, rnv_capsule, sizeof(router_node_vec*));
					}
				}

				hash_pos = router_node_vecs_find_hash(router_current->next, hash);
				errors_abort("hash_pos == -1 (1)", hash_pos == -1);

				router_current = &router_current->next->data[hash_pos];
				string_vecs_free(&var_terms, mem);
				continue;
			}

			bool is_regex_valid = strings_check_charset(&var_terms.data[1], regex_charset);
			errors_abort("!is_regex_valid", !is_regex_valid);

			size_t hash = strings_hasherize(&var_terms.data[0]);
			string name = strings_format("vars_%s", mem, var_terms.data[0].data);

			errors_abort("router_current.next", !router_current->next);
			long hash_pos = router_node_vecs_find_hash(router_current->next, hash);
			bool is_empty = router_current->next->size == 0;

			regex_t regex;
			int reg_status = regcomp(&regex, var_terms.data[1].data, REG_EXTENDED);
			errors_abort("regcomp != 0", reg_status != 0);

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
				router_node_vec rnv = router_node_vecs_init(vector_min, mem);
				router_node_vec *rnv_capsule = mems_alloc(mem, sizeof(router_node_vec));
				errors_abort("rnv_capsule", rnv_capsule == null);
				*rnv_capsule = rnv;

				router_node node = {
					.next=rnv_capsule,
					.data={
						.location=strings_clone(&var_terms.data[0], null),
						.has_regex=true,
						.regex=regex,
						.hash=hash,
						.name=name,
					}
				};

				bool error = router_node_vecs_push(router_current->next, node, mem);
				#if cels_debug
					errors_abort("error", error);
				#endif

				if (error) {
					strings_free(&node.data.location, mem);
					router_node_vecs_free(rnv_capsule, mem);
					mems_dealloc(mem, rnv_capsule, sizeof(router_node_vec*));
				}

				errors_abort("router_current.next", !router_current->next);
				hash_pos = router_node_vecs_find_hash(router_current->next, hash);

				errors_abort("hash_pos == -1 (2)", hash_pos == -1);
			} else {
				errors_abort("!is_empty (route colision)", !is_empty);
			}

			if (is_last) {
				router_private *router = &router_current->next->data[hash_pos].data;
				bool has_func_added = router->func != null;
				errors_abort("!func (already added)", has_func_added);

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
		errors_abort("callbacks", vectors_check((void *)callbacks));
	#endif

	https_initialize_private();
	router_node routes = https_create_routes_private(callbacks, mem);

	#if cels_debug
		router_nodes_full_debug(&routes);
		printf("\n");
	#endif
	
    struct sockaddr_in address = {
		.sin_family=AF_INET, 
		.sin_addr={.s_addr=INADDR_ANY},
		.sin_port=htons(port)
	};

    short socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
	errors_abort("socket failed", socket_descriptor == -1);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    short bind_statusus = bind(
		socket_descriptor, 
		(struct sockaddr *)&address, 
		sizeof(address));

	errors_abort("bind failed", bind_statusus == -1);

	#define https_request_maximum 200
    short listen_statusus = listen(socket_descriptor, https_request_maximum);
	errors_abort("listen failed", listen_statusus == -1);

    while (true) {
		struct sockaddr_in client_address;
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

		pthread_t thread;
		https_handle_client_params params = {
			.client=client_descriptor, .routes=&routes};
		pthread_create(&thread, NULL, https_handle_client, &params);
		pthread_detach(thread);
    }

    close(socket_descriptor);
}

void https_default_not_found(notused string_map *request, int client_connection, notused void *params) {
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
