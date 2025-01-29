#include "requests.h"

estring requests_connect_securely_private(int socket, const string packet, const allocator *mem) {
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

	SSL_CTX *ctx = SSL_CTX_new(method);
	if(!ctx) {
		err = request_creating_context_error;
		goto cleanup0;
	}

	SSL_CTX_set_options(ctx, 0);
	SSL *ssl = SSL_new(ctx);

	if(!ssl) {
		err = request_creating_context_error;
		goto cleanup1;
	}

	//

	int set_status = SSL_set_fd(ssl, socket);
	if(set_status < 0) {
		err = request_binding_secure_connection_error;
		goto cleanup2;
	}

	//

	int connection_status = SSL_connect(ssl);
	if(connection_status != 1) {
        ERR_print_errors_fp(stderr);
		err = request_opening_secure_connection_error;
		goto cleanup2;
	}

	//
	
	int write_status = SSL_write(ssl,packet.data, packet.size);
	if(write_status <= 0) {
		err = request_sending_error;
		goto cleanup2;
	}

	//

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

	//

	SSL_free(ssl);
	SSL_CTX_free(ctx);

    return (estring){.value=response};

	cleanup3:
	strings_free(&response, mem);

	cleanup2:
	SSL_free(ssl);

	cleanup1:
	SSL_CTX_free(ctx);

	cleanup0:
    return (estring){.error=err};
}

estring requests_connect_insecurely_private(int socket, const string packet, const allocator *mem) {
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

typedef struct request {
	string packet;
	string host;
	string port;
	struct addrinfo info;
	struct timeval timeout;
} request;

typedef errors(request) erequest;

erequest requests_contruct_private(const string *url, const request_option *opts, const allocator *mem) {
	#if cels_debug
		errors_abort("url", strings_check_extra(url));
	#endif

	error err = ok;

	request_option empty = {0};
	if (!opts) { opts = &empty; }

	request request = {0};

	//
	
	if (opts->head.data) {
		const string host = strings_premake("Host"); 
		ssize_t pos = strings_find(&opts->head, host, 0);
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


	//
	
	string method = {0}; 
	switch (opts->method) {
	case request_get_method:
		method = strings_do("GET");
	break;
	case request_post_method:
		method = strings_do("POST");
	break;
	case request_put_method:
		method = strings_do("PUT");
	break;
	case request_delete_method:
		method = strings_do("DELETE");
	break;
	default:
		method = strings_do("GET");
	break;
	}

	string version = {0};
	switch (opts->version) {
	case request_one_version:
		version = strings_do("HTTP/1.0");
	break;
	case request_one_dot_one_version:
		version = strings_do("HTTP/1.1");
	break;
	default:
		version = strings_do("HTTP/1.0");
	break;
	}

	//

	string_vec paths = strings_split_with(url, "/", 1, mem);
	if (paths.size == 0) { 
		err = request_malformed_url_error;
		goto cleanup1;
	}

	string host = paths.data[0];
	strings_println(&host);

	const string host_charset = strings_premake("abcdefghijklmnopqrstuvwxyz.-1234567890");
	bool is_host_valid = strings_check_charset(&host, host_charset);
	if (!is_host_valid) { 
		err = request_illegal_host_error;
		goto cleanup1;
	}

	string path = paths.size == 1 ? strings_do("") : paths.data[1];
	string head = !opts->head.data ? strings_do("") : opts->head;
	string body = !opts->body.data ? strings_do("") : opts->body;

	//

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

	request.port = opts->port.data ? opts->port : strings_do("80");

	request.info = (struct addrinfo) {
		.ai_family=AF_UNSPEC,
		.ai_socktype=SOCK_STREAM,
	};

	#define request_timeout 5
	size_t timeout = opts->timeout == 0 ? request_timeout : opts->timeout;
	request.timeout = (struct timeval) {.tv_sec=timeout, .tv_usec=0};
	#undef request_timeout

	string_vecs_free(&paths, mem);
	return (erequest){.value=request};

	cleanup1:
	string_vecs_free(&paths, mem);

	cleanup0:
	return (erequest){.error=err};
}

cels_warn_unused
eresponse requests_make(const string *url, const request_option *opts, const allocator *mem) {
	#if cels_debug
		errors_abort("url", strings_check_extra(url));
	#endif

	error err = ok;
	erequest request = requests_contruct_private(url, opts, mem);
	if (request.error != request_successfull) {
		err = request.error;
		goto cleanup0;
	}

	printf("packet: \n");
	strings_println(&request.value.packet);

	struct addrinfo *server;
	int get_status = getaddrinfo(
		request.value.host.data,
		request.value.port.data,
		&request.value.info,
		&server);

	if (get_status < 0) {
		err = request_dns_not_resolved_error;
		goto cleanup1;
	}

	if (!server) {
		err = request_dns_not_resolved_error;
		goto cleanup1;
	}

	/*
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

	string address_formated = strings_format(
		"https_request.address = %s", mem, ip);

	errors_print(errors_success_mode, address_formated.data, null);
	*/

	//

	int socket_d = socket(
		server->ai_family, 
		server->ai_socktype, 
		server->ai_protocol);

	if (socket_d < 0) {
		err = request_socket_creation_error;
		goto cleanup2;
	}

	//

	int set_status = setsockopt(
		socket_d, 
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
		socket_d, 
		server->ai_addr, 
		server->ai_addrlen);

	if (conn_status < 0) { 
		err = request_connection_error;
		goto cleanup2;
	}

	//

	const string https_default_port = strings_premake("80");
	const string https_default_secure_port = strings_premake("443");

	estring response_raw = {0};
	if (strings_equals(&request.value.port, &https_default_port)) {
		response_raw = requests_connect_insecurely_private(
			socket_d, request.value.packet, mem);
	} else if (strings_equals(&request.value.port, &https_default_secure_port)) {
		response_raw = requests_connect_securely_private(
			socket_d, request.value.packet, mem);
	} else {
		err = request_port_error;
		goto cleanup2;
	}

	if (response_raw.error != request_successfull) {
		err = response_raw.error;
		goto cleanup2;
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

	close(socket_d);
	freeaddrinfo(server);

	mems_dealloc(mem, packets.data, sizeof(string) * packets.capacity);
	strings_free(&response_raw.value, mem);
	strings_free(&request.value.packet, mem);
	strings_free(&request.value.host, mem);

	return (eresponse){.value=response};

	cleanup2:
	close(socket_d);
	freeaddrinfo(server);

	cleanup1:
	strings_free(&request.value.packet, mem);
	strings_free(&request.value.host, mem);

	cleanup0:
	return (eresponse){.error=err};
}

void request_errors_println(request_error self) {
	#if cels_debug
		errors_abort("self", !self);
	#endif

    switch (self) {
	case request_successfull:
		printf("request was successful.\n");
	break;
	case request_default_error:
		printf("error: unknown.\n");
	break;
	case request_protocol_in_url_error:
		printf("error: url has protocol.\n");
	break;
	case request_malformed_url_error:
		printf("error: url malformed.\n");
	break;
	case request_illegal_host_error:
		printf("error: illegal host.\n");
	break;
	case request_precomputed_field_set_error:
		printf("error: precomputed field set error.\n");
	break;
	case request_dns_not_resolved_error:
		printf("error: dns not resolved.\n");
	break;
	case request_socket_creation_error:
		printf("error: socket creation failed.\n");
	break;
	case request_set_socket_option_error:
		printf("error: failed to set socket options.\n");
	break;
	case request_connection_error:
		printf("error: connection failed.\n");
	break;
	case request_sending_error:
		printf("error: failed to send data.\n");
	break;
	case request_receiving_error:
		printf("error: failed to receive data.\n");
	break;
	case request_upscaling_error:
		printf("error: upscaling failed.\n");
	break;
	case request_initializing_crypto_error:
		printf("error: failed to initialize crypto.\n");
	break;
	case request_initializing_ssl_error:
		printf("error: ssl initialization failed.\n");
	break;
	case request_initializing_bio_error:
		printf("error: failed to initialize bio.\n");
	break;
	case request_initializing_library_error:
		printf("error: failed to initialize library.\n");
	break;
	case request_creating_context_error:
		printf("error: failed to create context.\n");
	break;
	case request_opening_secure_connection_error:
		printf("error: failed to open secure connection.\n");
	break;
	case request_binding_secure_connection_error:
		printf("error: failed to bind secure connection.\n");
	break;
	case request_certification_error:
		printf("error: certification failed.\n");
	break;
	case request_port_error:
		printf("error: invalid port.\n");
	break;
    }
}
