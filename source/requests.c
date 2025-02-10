#include "requests.h"
#include "strings.h"

/* private */

typedef struct request {
	string packet;
	string host;
	string port;
	struct addrinfo info;
	struct timeval timeout;
	size_t max_retry;
	size_t initial_buffer_size;
	bool is_secure;
} request;

typedef errors(request) erequest;

static const string request_port = strings_premake("80");
static const string request_port_secure = strings_premake("443");

void requests_free_private(request *self, const allocator *mem) {
	#if cels_debug
		errors_abort("self", !self);
	#endif

	if (self->packet.data) {
		strings_free(&self->packet, mem);
	}

	if (self->host.data) {
		strings_free(&self->host, mem);
	}

	#if cels_debug
		memset(&self->packet, 0, sizeof(string));
		memset(&self->host, 0, sizeof(string));
	#endif
}

void request_internals_free_private(request_internal *self, const allocator *mem) {
	#if cels_debug
		errors_abort("self", !self);
	#endif

	if (self->response.data) {
		strings_free(&self->response, mem);
	}

	if (self->packet.data) {
		strings_free(&self->packet, mem);
	}

	#if cels_debug
		self->socket = 0;
		memset(&self->response, 0, sizeof(char_vec));
		memset(&self->packet, 0, sizeof(string));
	#endif
}

erequest requests_contruct_private(
	const string *url, const request_option *option, const allocator *mem
) {
	#if cels_debug
		errors_abort("url", strings_check_extra(url));
	#endif

	error err = ok;

	request request = {0};
	request_option empty = {0};
	if (!option) { option = &empty; }

	//

	if (option->head.data) {
		static const string host = strings_premake("Host"); 
		ssize_t pos = strings_find(&option->head, host, 0);
		if (pos != -1) {
			err = request_precomputed_field_set_error;
			goto cleanup0;
		}
	}

	ssize_t protocol_position = strings_find_with(url, "://", 0);
	if (protocol_position >= 0) { 
		err = request_protocol_in_url_error;
		goto cleanup0;
	}

	string method = strings_do("GET"); 
	if (option->method >= 0 && option->method < request_private_method) {
		method = strings_do((void *)request_methods[option->method]);
	}

	string version = strings_do("HTTP/1.0");
	if (option->version >= 0 && option->version < request_private_version) {
		version = strings_do((void *)request_versions[option->method]);
	}

	//

	string_vec paths = strings_split_with(url, "/", 1, mem);
	if (paths.size == 0) { 
		err = request_malformed_url_error;
		goto cleanup1;
	}

	string host = paths.data[0];
	static const string host_charset = 
		strings_premake("abcdefghijklmnopqrstuvwxyz.-1234567890");

	bool is_host_valid = strings_check_charset(&host, host_charset);
	if (!is_host_valid) { 
		err = request_illegal_host_error;
		goto cleanup1;
	}

	string path = paths.size == 1 ? strings_do("") : paths.data[1];
	string head = !option->head.data ? strings_do("") : option->head;
	string body = !option->body.data ? strings_do("") : option->body;

	//

	request.max_retry = option->max_retry == 0 ? 5 : option->max_retry;

	request.initial_buffer_size = 
		option->initial_buffer_size == 0 ? 
		string_small_size : option->initial_buffer_size;

	request.host = host;
	request.packet = strings_format(
		"%s /%s %s\r\nHost:%s\r\n%s\r\n\r\n%s\r\n", 
		mem,
		method.data, 
		path.data,
		version.data,
		host.data,
		head.data, 
		body.data);

	request.port = option->port.data ? option->port : strings_do("80");
	request.info = (struct addrinfo) {
		.ai_family=AF_UNSPEC,
		.ai_socktype=SOCK_STREAM,
	};

	if (strings_equals(&request.port, &request_port)) {
		request.is_secure = false;
	} else if (strings_equals(&request.port, &request_port_secure)) {
		request.is_secure = true;
	} else {
		err = request_port_error;
		goto cleanup1;
	}

	#define request_timeout 5
	size_t timeout = option->timeout == 0 ? request_timeout : option->timeout;
	request.timeout = (struct timeval) {.tv_sec=timeout, .tv_usec=0};
	#undef request_timeout

	mems_dealloc(mem, paths.data, paths.capacity);
	return (erequest){.value=request};

	cleanup1:
	string_vecs_free(&paths, mem);

	cleanup0:
	return (erequest){.error=err};
}

cels_warn_unused
erequest_internal requests_init_private(const string *url, const request_option *option, const allocator *mem) {
	error err = ok;

	erequest request = requests_contruct_private(url, option, mem);
	if (request.error != request_successfull) {
		err = request.error;
		goto cleanup0;
	}

	struct addrinfo *server;
	int get_status = getaddrinfo(
		request.value.host.data,
		request.value.port.data,
		&request.value.info,
		&server);

	if (get_status < 0 || !server) {
		err = request_dns_not_resolved_error;
		freeaddrinfo(server);
		goto cleanup1;
	}

	//

	#if cels_debug
		void *address = null;
		if (server->ai_family == AF_INET) {
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)server->ai_addr;
			address = &(ipv4->sin_addr);
		} else {
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)server->ai_addr;  
			address = &(ipv6->sin6_addr);
		}

		char ip[INET6_ADDRSTRLEN];
		inet_ntop(server->ai_family, address, ip, sizeof(ip)); 

		string address_formated = strings_format("https_request.address = %s", mem, ip);
		errors_print(errors_success_mode, address_formated.data, null);
	#endif

	//

	int socket_descriptor = socket(
		server->ai_family, 
		server->ai_socktype, 
		server->ai_protocol);

	if (socket_descriptor < 0) {
		err = request_socket_creation_error;
		goto cleanup1;
	}

	//

	int set_status = setsockopt(
		socket_descriptor, 
		SOL_SOCKET, 
		SO_RCVTIMEO, 
		(const char*)&request.value.timeout, 
		sizeof(request.value.timeout));

	if (set_status < 0) {
		err = request_set_socket_option_error;
		goto cleanup2;
	}

	//

	int conn_status = connect(
		socket_descriptor, 
		server->ai_addr, 
		server->ai_addrlen);

	freeaddrinfo(server);

	if (conn_status < 0) { 
		err = request_connection_error;
		goto cleanup2;
	}

	//

	request_internal internal = {
		.socket = socket_descriptor,
		.packet = request.value.packet,
		.port = request.value.port,
		.state = request_send_state,
		.max_retry = request.value.max_retry,
		.initial_buffer_size = request.value.initial_buffer_size,
		.is_secure = request.value.is_secure,
	};

	return (erequest_internal){.value=internal};

	cleanup2:
	close(socket_descriptor);

	cleanup1:
	requests_free_private(&request.value, mem);

	cleanup0:
	return (erequest_internal){.error=err};
}

#if cels_openssl
estring requests_connect_securely_private(
	int socket, const string packet, const allocator *mem
) {
	#if cels_debug
		errors_abort("packet", strings_check_extra(&packet));
	#endif

	error err = ok;

	int crypto_options = 
		OPENSSL_INIT_ADD_ALL_CIPHERS | 
		OPENSSL_INIT_ADD_ALL_DIGESTS | 
		OPENSSL_INIT_LOAD_CRYPTO_STRINGS;

    int init_status = OPENSSL_init_crypto(crypto_options, null);
	if(init_status < 0) {
		err = request_initializing_crypto_error;
		goto cleanup0;
	}

	int ssl_options = 
		OPENSSL_INIT_LOAD_SSL_STRINGS | 
		OPENSSL_INIT_LOAD_CRYPTO_STRINGS;

	init_status = OPENSSL_init_ssl(ssl_options, null);
	if(init_status < 0) {
		err = request_initializing_ssl_error;
		goto cleanup0;
	}

	const SSL_METHOD *method = TLS_client_method();
	if(!method) {
		err = request_creating_context_error;
		goto cleanup0;
	}

	SSL_CTX *ssl_context = SSL_CTX_new(method);
	if(!ssl_context) {
		err = request_creating_context_error;
		goto cleanup0;
	}

	SSL_CTX_set_options(ssl_context, 0);
	SSL *ssl = SSL_new(ssl_context);

	if(!ssl) {
		err = request_creating_context_error;
		goto cleanup1;
	}

	int set_status = SSL_set_fd(ssl, socket);
	if(set_status < 0) {
		err = request_binding_secure_connection_error;
		goto cleanup2;
	}

	int connection_status = SSL_connect(ssl);
	if(connection_status != 1) {
        ERR_print_errors_fp(stderr);
		err = request_opening_secure_connection_error;
		goto cleanup2;
	}

	int write_status = SSL_write(ssl,packet.data, packet.size);
	if(write_status <= 0) {
		err = request_sending_error;
		goto cleanup2;
	}

	string response = strings_init(string_small_size, mem);
    while(response.size < response.capacity) {
        long bytes = SSL_read(
			ssl,
			response.data + response.size, 
			response.capacity - response.size);

        if (bytes < 0) {
			err = request_receiving_error;
			goto cleanup3;
		} 

        response.size += bytes;

		if (bytes == 0) {
            break;
		} else if (response.size >= response.capacity) {
			bool upscale_status = char_vecs_upscale(&response, mem);

			if (upscale_status) {
				err = request_upscaling_error;
				goto cleanup3;
			}
		}
    }

	bool push_error = char_vecs_push(&response, '\0', mem);
	if (push_error) {
		err = request_upscaling_error;
		goto cleanup3;
	}

	SSL_free(ssl);
	SSL_CTX_free(ssl_context);

    return (estring){.value=response};

	cleanup3:
	strings_free(&response, mem);

	cleanup2:
	SSL_free(ssl);

	cleanup1:
	SSL_CTX_free(ssl_context);

	cleanup0:
    return (estring){.error=err};
}

bool requests_connect_securely_async_private(request_async *request, const allocator *mem) {
	#if cels_debug
		errors_abort("packet", strings_check_extra(&request->internal.packet));
	#endif

	switch (request->internal.state) {
		case request_init_state: return false;
		case request_send_state: goto send;
		case request_receive_state: goto receive;
		case request_finished_state: return false;
	}

	send: {
		int crypto_options = 
			OPENSSL_INIT_ADD_ALL_CIPHERS | 
			OPENSSL_INIT_ADD_ALL_DIGESTS | 
			OPENSSL_INIT_LOAD_CRYPTO_STRINGS;

		int init_status = OPENSSL_init_crypto(crypto_options, null);
		if(init_status < 0) {
			request->response.error = request_initializing_crypto_error;
			goto send_cleanup0;
		}

		int ssl_options = 
			OPENSSL_INIT_LOAD_SSL_STRINGS | 
			OPENSSL_INIT_LOAD_CRYPTO_STRINGS;

		init_status = OPENSSL_init_ssl(ssl_options, null);
		if(init_status < 0) {
			request->response.error = request_initializing_ssl_error;
			goto send_cleanup0;
		}

		const SSL_METHOD *method = TLS_client_method();
		if(!method) {
			request->response.error = request_creating_context_error;
			goto send_cleanup0;
		}

		SSL_CTX *ssl_context = SSL_CTX_new(method);
		if(!ssl_context) {
			request->response.error = request_creating_context_error;
			goto send_cleanup0;
		}

		SSL_CTX_set_options(ssl_context, 0);
		SSL *ssl = SSL_new(ssl_context);

		if(!ssl) {
			request->response.error = request_creating_context_error;
			goto send_cleanup1;
		}

		int set_status = SSL_set_fd(ssl, request->internal.socket);
		if(set_status < 0) {
			request->response.error = request_binding_secure_connection_error;
			goto send_cleanup2;
		}

		int connection_status = SSL_connect(ssl);
		if(connection_status != 1) {
			ERR_print_errors_fp(stderr);
			request->response.error = request_opening_secure_connection_error;
			goto send_cleanup2;
		}

		int write_status = SSL_write(
			ssl, 
			request->internal.packet.data, 
			request->internal.packet.size);
		if(write_status <= 0) {
			request->response.error = request_sending_error;
			goto send_cleanup2;
		}

		request->internal.state = request_receive_state;
		request->internal.ssl = ssl;
		request->internal.context = ssl_context;
		request->internal.response = strings_init(request->internal.initial_buffer_size, mem);
		return true;

		send_cleanup2:
		SSL_free(ssl);

		send_cleanup1:
		SSL_CTX_free(ssl_context);

		send_cleanup0:
		return false;
	}

	receive: {
		char_vec *response = &request->internal.response;

		#if cels_debug
			errors_abort("request.internal.response", vectors_check((const vector *)response));
		#endif

		if (response->size > 0) {
			response->size--;
		}

		size_t size = response->capacity - response->size;
		if (size > 0) { size--; }

		long bytes = SSL_read(
			request->internal.ssl,
			response->data + response->size, 
			size);

		if (bytes < 0 && request->internal.max_retry >= request->internal.retried) {
			request->response.error = request_receiving_error;
			goto receive_cleanup0;
		} else if (bytes < 0) {
			request->internal.retried++;
			return true;
		}

		request->internal.retried = 0;
		response->size += bytes + 1;
		response->data[response->size - 1] = '\0';

		if (bytes > 0) {
			if (response->size >= response->capacity) {
				bool upscale_status = char_vecs_upscale(response, mem);

				if (upscale_status) {
					request->response.error = request_upscaling_error;
					goto receive_cleanup1;
				}
			}

			#if cels_debug
				errors_abort("request.internal.response", vectors_check((const vector *)response));
			#endif

			return true;
		}

		SSL_free(request->internal.ssl);
		SSL_CTX_free(request->internal.context);
		return false;

		receive_cleanup1:
		strings_free(&request->internal.response, mem);

		receive_cleanup0:
		SSL_free(request->internal.ssl);
		SSL_CTX_free(request->internal.context);

		return false;
	}
}
#endif

estring requests_connect_insecurely_private(
	int socket, const string packet, const allocator *mem
) {
	#if cels_debug
		errors_abort("packet", strings_check_extra(&packet));
	#endif

	int send_status = send(socket, packet.data, packet.size, 0);
	if (send_status < 0) {
		return (estring){.error=request_sending_error};
	}

	error err = ok;
	string response = strings_init(string_small_size, mem);

    while(response.size < response.capacity) {
        long bytes = recv(
			socket, 
			response.data + response.size, 
			response.capacity - response.size, 
			0);

		if (bytes < 0) {
			err = request_receiving_error;
			goto cleanup;
		} 

        response.size += bytes;

        if (bytes == 0) {
            break;
		} else if (response.size >= response.capacity) {
			error upscale_error = char_vecs_upscale(&response, mem);
			if (upscale_error) {
				err = request_upscaling_error;
				goto cleanup;
			}
		}
    }

	bool push_error = char_vecs_push(&response, '\0', mem);
	if (push_error) {
		err = request_upscaling_error;
		goto cleanup;
	}

    return (estring){.value=response};

	cleanup:
	strings_free(&response, mem);
	return (estring){.error=err};
}

bool requests_connect_insecurely_async_private(request_async *request, const allocator *mem) {
	#if cels_debug
		errors_abort("packet", strings_check_extra(&request->internal.packet));
	#endif

	switch (request->internal.state) {
		case request_init_state: return false;
		case request_send_state: goto send;
		case request_receive_state: goto receive;
		case request_finished_state: return false;
	}

	send: {
		int send_status = send(
			request->internal.socket, 
			request->internal.packet.data, 
			request->internal.packet.size, 
			0);

		if (send_status < 0) {
			request->response.error = request_sending_error;
			return false;
		}

		request->internal.response = strings_init(request->internal.initial_buffer_size, mem);
		request->internal.state = request_receive_state;
		return true;
	}

	receive: {
		char_vec *response = &request->internal.response; 

		#if cels_debug
			errors_abort("request.internal.response", vectors_check((const vector *)response));
		#endif

		if (response->size > 0) {
			response->size--;
		}

		size_t size = response->capacity - response->size;
		if (size > 0) { size--; }

		long bytes = recv(
			request->internal.socket, 
			response->data + response->size, 
			size, 0);

		if (bytes < 0 && request->internal.retried >= request->internal.max_retry) {
			request->response.error = request_receiving_error;
			goto receive_cleanup0;
		} else if (bytes < 0) {
			request->internal.retried++;
			return true;
		}

		request->internal.retried = 0;
		response->size += bytes + 1;
		response->data[response->size - 1] = '\0';

		if (bytes > 0) {
			if (response->size >= response->capacity) {
				error upscale_error = char_vecs_upscale(response, mem);

				if (upscale_error) {
					request->response.error = request_upscaling_error;
					goto receive_cleanup0;
				}
			}

			#if cels_debug
				errors_abort("request.internal.response", vectors_check((const vector *)response));
			#endif

			return true;
		}

		return false;

		receive_cleanup0:
		strings_free(&request->internal.response, mem);
		return false;
	}
}

/* public */

void request_errors_println(request_error self) {
	#if cels_debug
		errors_abort("self", !self);
		errors_abort("#self", self < 0 || self >= request_private_error);
	#endif
	
	if (self < 0 || self >= request_private_error) {
		return;
	}

	printf("%s\n", request_error_messages[self]);
}

eresponse requests_make(const string *url, const request_option *option, const allocator *mem) {
	#if cels_debug
		errors_abort("url", strings_check_extra(url));
	#endif

	error err = ok;
	erequest_internal internal = requests_init_private(url, option, mem);
	if (internal.error != request_successfull) {
		err = internal.error;
		goto cleanup0;
	}

	estring response_raw = {0};
	if (!internal.value.is_secure) {
		response_raw = requests_connect_insecurely_private(
			internal.value.socket, internal.value.packet, mem);
	} 
	#if cels_openssl
	else {
		response_raw = requests_connect_securely_private(
			internal.value.socket, internal.value.packet, mem);
	}
	#else
	else {
		err = request_secure_not_implemented_error;
		goto cleanup0;
	}
	#endif

	if (response_raw.error != request_successfull) {
		err = response_raw.error;
		goto cleanup1;
	}

	//
	
	response response = {0};
	string_vec packets = strings_split_with(&response_raw.value, "\r\n\r\n", 1, mem);
	errors_abort("#packets", packets.size == 0);

	if (packets.size == 1) {
		response.head = packets.data[0];
		response.body = strings_do("");
	} else {
		response.head = packets.data[0];
		response.body = packets.data[1];
	}

	mems_dealloc(mem, packets.data, sizeof(string) * packets.capacity);
	strings_free(&response_raw.value, mem);
	request_internals_free_private(&internal.value, mem);
	close(internal.value.socket);

	return (eresponse){.value=response};

	cleanup1:
	close(internal.value.socket);
	request_internals_free_private(&internal.value, mem);

	cleanup0:
	return (eresponse){.error=err};
}

bool requests_make_async(const string *url, request_async *request, const allocator *mem) {
	#if cels_debug
		errors_abort("url", strings_check_extra(url));
	#endif

	switch (request->internal.state) {
		case request_init_state: goto init;
		case request_send_state: goto connect;
		case request_receive_state: goto connect;
		case request_finished_state: return false;
	}

	init: {
		erequest_internal internal = requests_init_private(url, &request->option, mem);
		if (internal.error != request_successfull) {
			request->internal = internal.value;
			request->response.error = internal.error;
			return false;
		}

		request->internal = internal.value;
		return true;
	}

	connect: {
		bool shall_continue = false;
		if (!request->internal.is_secure) {
			shall_continue = requests_connect_insecurely_async_private(request, mem);
		} 
		#if cels_openssl
		else {
			shall_continue = requests_connect_securely_async_private(request, mem);
		}
		#else
		else {
			request->response.error = request_secure_not_implemented_error;
			goto connect_cleanup0;
		}
		#endif

		if (shall_continue) {
			return true;
		} else if (request->response.error != request_successfull) {
			goto connect_cleanup0;
		}

		//
		
		bool is_raw = (request->option.flags & request_async_raw_mode_flag) == 1;
		if (!is_raw) {
			response response = {0};
			string_vec packets = strings_split_with(
				&request->internal.response, "\r\n\r\n", 1, mem);
			errors_abort("#packets", packets.size == 0);

			if (packets.size == 1) {
				response.head = packets.data[0];
				response.body = strings_do("");
			} else {
				response.head = packets.data[0];
				response.body = packets.data[1];
			}

			request->response.value = response;
			mems_dealloc(mem, packets.data, sizeof(string) * packets.capacity);
		}

		close(request->internal.socket);
		request_internals_free_private(&request->internal, mem);

		return false;

		connect_cleanup0:
		close(request->internal.socket);
		request_internals_free_private(&request->internal, mem);
		return false;
	}
}
