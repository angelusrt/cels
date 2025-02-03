#ifndef cels_requests_h
#define cels_requests_h

#ifndef cels_openssl
#define cels_openssl 1
#endif

#if cels_openssl
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#endif

#include <netdb.h>
#include <arpa/inet.h>

#include "errors.h"
#include "strings.h"
#include "vectors.h"
#include "mems.h"

typedef enum request_error {
	request_successfull,
	request_default_error,
	request_protocol_in_url_error,
	request_malformed_url_error,
	request_illegal_host_error,
	request_precomputed_field_set_error,
	request_dns_not_resolved_error,
	request_socket_creation_error,
	request_set_socket_option_error,
	request_connection_error,
	request_sending_error,
	request_receiving_error,
	request_upscaling_error,
	request_initializing_crypto_error,
	request_initializing_ssl_error,
	request_initializing_bio_error,
	request_initializing_library_error,
	request_creating_context_error,
	request_opening_secure_connection_error,
	request_binding_secure_connection_error,
	request_certification_error,
	request_port_error,
	request_secure_not_implemented_error,
	request_private_error
} request_error;

typedef enum request_method {
	request_get_method,
	request_post_method,
	request_put_method,
	request_delete_method,
	request_patch_method,
	request_private_method,
} request_method;

typedef enum request_version {
	request_one_version,
	request_one_dot_one_version,
	request_private_version,
} request_version;

typedef enum request_state {
	request_init_state,
	request_send_state,
	request_receive_state,
	request_finished_state
} request_state;

typedef enum request_flag {
	/* 
	 * In async_raw_mode, response is not filled, 
	 * instead the user should rely on 
	 * request->internal.response
	 */
	request_async_raw_mode_flag = 1 << 0,
} request_flag;

typedef struct request_option {
	const string head;
	const string body;
	const string port;
	ulong timeout;
	size_t max_retry;
	size_t initial_buffer_size;
	request_method method;
	request_version version;
	request_flag flags;
} request_option;

typedef struct response {
	string head;
	string body;
} response;

typedef errors(response) eresponse;

typedef struct request_internal {
	request_state state;
	string packet;
	char_vec response;
	string port;
	int socket;

	#if cels_openssl
	SSL *ssl;
	SSL_CTX *context;
	#endif

	size_t max_retry;
	size_t retried;
	size_t initial_buffer_size;
	bool is_secure;
} request_internal;

typedef errors(request_internal) erequest_internal;

typedef struct request_async {
	request_internal internal;
	request_option option;
	eresponse response;
} request_async;

static const char *request_error_messages[] = {
    [request_successfull] = "request was successful.",
    [request_default_error] = "error: unknown.",
    [request_protocol_in_url_error] = "error: url has protocol.",
    [request_malformed_url_error] = "error: url malformed.",
    [request_illegal_host_error] = "error: illegal host.",
    [request_precomputed_field_set_error] = "error: precomputed field set error.",
    [request_dns_not_resolved_error] = "error: dns not resolved.",
    [request_socket_creation_error] = "error: socket creation failed.",
    [request_set_socket_option_error] = "error: failed to set socket options.",
    [request_connection_error] = "error: connection failed.",
    [request_sending_error] = "error: failed to send data.",
    [request_receiving_error] = "error: failed to receive data.",
    [request_upscaling_error] = "error: upscaling failed.",
    [request_initializing_crypto_error] = "error: failed to initialize crypto.",
    [request_initializing_ssl_error] = "error: ssl initialization failed.",
    [request_initializing_bio_error] = "error: failed to initialize bio.",
    [request_initializing_library_error] = "error: failed to initialize library.",
    [request_creating_context_error] = "error: failed to create context.",
    [request_opening_secure_connection_error] = "error: failed to open secure connection.",
    [request_binding_secure_connection_error] = "error: failed to bind secure connection.",
    [request_certification_error] = "error: certification failed.",
    [request_secure_not_implemented_error] = "error: secure not implemented.",
    [request_port_error] = "error: invalid port."
};

static const char *request_methods[] = {
    [request_get_method] = "GET",
    [request_post_method] = "POST",
    [request_put_method] = "PUT",
    [request_delete_method] = "DELETE",
    [request_patch_method] = "PATCH",
};

static const char *request_versions[] = {
    [request_one_version] = "HTTP/1.0",
    [request_one_dot_one_version] = "HTTP/1.1"
};

/*
 * Prints error to screen.
 *
 * #to-review
 */
void request_errors_println(request_error self);

/*
 * Requests a site and returns 
 * response.
 *
 * #implicitly-allocates #allocates
 * #to-review
 */
cels_warn_unused
eresponse requests_make(const string *url, const request_option *opts, const allocator *mem);

/*
 * Requests a site asynchronously and 
 * returns response.
 *
 * #implicitly-allocates #allocates
 * #to-review
 */
cels_warn_unused
bool requests_make_async(const string *url, request_async *request, const allocator *mem);

#endif
