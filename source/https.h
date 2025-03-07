#ifndef https_h
#define https_h

#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <regex.h>        
#include <netdb.h>
#include <sys/cdefs.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h> 
#include <arpa/inet.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

#include "errors.h"
#include "strings.h"
#include "vectors.h"
#include "nodes.h"
#include "utils.h"
#include "bytes.h"


/*
 * The module 'https' provides conveniences 
 * to comunicate via http - be it a web-server 
 * or a network-request.
 *
 * If you use it, you need to compile with 
 * "-lm -lpthread -lssl -lcrypto"
 */


/* routers */

typedef void (*httpfunc) (byte_map *, int, void *);

typedef enum http_error {
	http_successfull,
	http_generic_error,
	http_request_invalid_error,
	http_request_mal_formed_error,
	http_not_found_error,
	http_head_size_error,
	http_method_invalid_error,
	http_location_invalid_error,
	http_protocol_invalid_error,
	http_property_size_invalid_error,
	http_property_mal_formed_error,
	http_property_probably_duplicated_error,
	http_route_name_mal_formed_error,
	http_route_name_invalid_error,
	http_route_collision_error,
	http_route_regex_invalid_error,
	http_socket_failed_error,
	http_bind_failed_error,
	http_listen_failed_error,
} http_error;

typedef struct router {
	string location;
	httpfunc func;
	void *param;
} router;

typedef struct router_private {
	string location;
	httpfunc func;
	void *param;
	string name;
	size_t hash;
	bool has_regex;
	regex_t regex;
} router_private;

typedef errors(router_private) erouter_private;

/*
 * Checks router if it's invalid returning true if it is.
 *
 * #to-review
 */
bool routers_check(const router *self);


/* router_vecs */

typedef vectors(router) router_vec;

error router_vecs_push_with(
	router_vec *self, 
	char *location, 
	httpfunc callback, 
	void *param, 
	const allocator *mem);


/* router_nodes and router_node_vecs */

typedef struct router_node router_node;
typedef munodes(router_node, router_private) router_node;
typedef mutrees(router_node) router_tree;
typedef errors(router_tree) erouter_tree;
typedef mutree_iterators(router_node) router_tree_iterator;


/* router_nodes */

/*
 * Checks if router_node is invalid, 
 * returning true if it is.
 *
 * #to-review
 */
cels_warn_unused
bool router_nodes_check(const router_node *self);

/*
 * Prints debug-friendly message 
 * of router_node structure.
 *
 * #to-review
 */
void router_nodes_debug(const router_node *self);


/* https */

static const byte_vec https_default_head = 
	byte_vecs_premake("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");

/*
 * Serves a web-server to port 
 * with list of router callbacks 
 * provided.
 *
 * #allocates #to-review
 */
http_error https_serve(short port, router_vec *callbacks, const allocator *mem);

/*
 * Provides standard not_found response.
 *
 * #to-review
 */
void https_send_not_found(
	byte_map *request, int client_connection, void *param);

/*
 * Sends body and head to client. 
 *
 * #to-review
 */
void https_send(
	int client_connection, const byte_vec head, const byte_vec body);

#endif
