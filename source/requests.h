#ifndef cels_requests_h
#define cels_requests_h

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

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
} request_error;

typedef enum request_method {
	request_get_method,
	request_post_method,
	request_put_method,
	request_delete_method,
} request_method;

typedef enum request_version {
	request_one_version,
	request_one_dot_one_version,
} request_version;

typedef struct request_option {
	const string head;
	const string body;
	const string port;
	ulong timeout;
	size_t max_retry;
	request_method method;
	request_version version;
} request_option;

typedef struct response {
	string head;
	string body;
} response;

typedef errors(response) eresponse;

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
 * Prints error to screen.
 *
 * #to-review
 */
void request_errors_println(request_error self);

#endif
