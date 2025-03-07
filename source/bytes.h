#ifndef cels_bytes_h
#define cels_bytes_h

#include <sys/socket.h>

#include "mems.h"
#include "nodes.h"
#include "vectors.h"
#include "types.h"


/*
 * The 'bytes' module defines the 
 * byte data-structure, which is a 
 * respresentation over raw-data.
 *
 * The most usefull definition is 
 * byte_vec, which is a vector of 
 * null-terminated sized raw-data.
 */


/* byte */

typedef unsigned char byte;


/* byte_vec */

typedef vectors(byte) byte_vec;
typedef errors(byte_vec) ebyte_vec;

typedef vectors(byte_vec) byte_mat;

#include "strings.h"

maps(byte_map, string, byte_vec)
typedef errors(byte_map) ebyte_map;

/* 
 * Creates a static variable from string literal 
 * with size and capacity already specified.
 *
 * Should'nt be modified (view only)!
 *
 * #view-only #to-review
 */
#define byte_vecs_premake(lit) { \
	.size=sizeof(lit), \
	.capacity=sizeof(lit), \
	.data=(byte *)lit, \
	.type_size=sizeof(byte) \
}

/*
 * Checks if 'self' is invalid.
 *
 * #to-review
 */
bool byte_vecs_check(const byte_vec *self);

/* 
 * Prints byte_vecs.
 *
 * #to-review
 */
void byte_vecs_print(const byte_vec *self);

/*
 * Frees byte_vec.
 *
 * #to-review
 */
void byte_vecs_free(byte_vec *self, const allocator *mem);

/*
 * Checks if byte_vecs is string.
 *
 * #to-review
 */
cels_warn_unused
bool byte_vecs_is_string(own byte_vec *self);

/*
 * Receives response from socket_descriptor - 
 * wraps recv syscall.
 *
 * If an error happens 'fail' is set.
 *
 * If 'max_size' is bigger than zero, 
 * it limits byte_vec lenght.
 *
 * #to-review
 */
cels_warn_unused
ebyte_vec byte_vecs_receive(
	int socket_descriptor, int socket_flags, size_t max_size, const allocator *mem);

/*
 * Finds all occurrences of 'substring' 
 * within 'self' limited by 'n' and returns a list 
 * of its positions.
 *
 * #to-review
 */
size_vec byte_vecs_find_all(
	const byte_vec *self, 
	const byte_vec substring, 
	size_t n, 
	const allocator *mem);

/*
 * Splits bytes according to separator 'sep' 
 * limited by 'n'.
 *
 * #to-review
 */
byte_mat byte_vecs_split(
	const byte_vec *self, 
	const byte_vec sep, 
	size_t n, 
	const allocator *mem);

#endif
