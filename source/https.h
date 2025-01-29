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

/*
 * The module 'https' provides conveniences 
 * to comunicate via http - be it a web-server 
 * or a network-request.
 *
 * If you use it, you need to compile with 
 * "-lm -lpthread -lssl -lcrypto"
 */

/* routers */

typedef void (*httpfunc) (string_map *, int, void *);

typedef struct router {
	string location;
	httpfunc func;
	void *params;
} router;

typedef struct router_private {
	string location;
	httpfunc func;
	void *params;
	string name;
	size_t hash;
	bool has_regex;
	regex_t regex;
} router_private;

/*
 * Checks router if it's invalid returning true if it is.
 *
 * #to-review
 */
bool routers_check(const router *self);

/*
 * Prints debug-friendly message of router structure.
 *
 * #to-review
 */
void routers_debug(const router *self);

/*
 * Clones 'self'.
 *
 * #to-review
 */
router routers_clone(router *self, const allocator *mem);

/*
 * Prints router.
 *
 * #to-review
 */
void routers_print(const router *self);

/*
 * Compares routers location, if they are 
 * equal, it returns true.
 *
 * #case-sensitive #to-review
 */
bool routers_equals(const router *r0, const router *r1);

/*
 * Compares routers location, if they seem, 
 * it returns true.
 *
 * #case-insensitive #to-review
 */
bool routers_seems(const router *r0, const router *r1);

/* router_vecs */

vectors_generate_definition(router, router_vec)

bool router_vecs_make_push(
	router_vec *self, 
	char *location, 
	httpfunc callback, 
	void *params, 
	const allocator *mem);

/* router_nodes and router_node_vecs */

typedef struct router_node router_node;
vectors_generate_definition(router_node, router_node_vec)
nodes(router_node, router_node_vec, router_private);

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

/*
 * Debugs recursively all tree.
 *
 * #to-review
 */
void router_nodes_full_debug(const router_node *self);

/*
 * Prints router_node to the terminal.
 *
 * #depends:stdio.h #to-review
 */
void router_nodes_print(const router_node *self);

/*
 * Compares router_node returning true if 
 * router_node.data.location are equal.
 *
 * #case-sensitive #to-review
 */
cels_warn_unused
bool router_nodes_equals(const router_node *r0, const router_node *r1);

/*
 * Compares router_node returning true if 
 * router_node.data.location seem alike.
 *
 * #case-insensitive #to-review
 */
cels_warn_unused
bool router_nodes_seems(const router_node *r0, const router_node *r1);

/* https */

static const string https_default_head = 
	strings_premake("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");

/*
 * Serves a web-server to port 
 * with list of router callbacks 
 * provided.
 *
 * #allocates
 */
void https_serve(short port, router_vec *callbacks, const allocator *mem);

/*
 * Provides standard not_found response.
 */
void https_default_not_found(string_map *request, int client_connection, void *params);

/*
 * Sends body and head to client. 
 */
void https_send(int client_connection, const string *body, const string *head);

#endif
