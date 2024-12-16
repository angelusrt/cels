#ifndef strings_h
#define strings_h
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <math.h>
#include <ctype.h>

#include "errors.h"
#include "vectors.h"

typedef vectors(char *) string;
typedef vectors(string *) string_vec;
typedef errors(string) string_with_error;

typedef enum strings_size {
	strings_min_size = 16,
	strings_petit_size = 128,
	strings_small_size = 1024,
	strings_big_size = 4096,
	strings_large_size = 65536,
	strings_very_large_size = 1048576, 
	strings_max_size = SIZE_MAX,
} strings_size;

/*
 * The module strings is about manipulating the 
 * data structure 'string'.
 *
 * It has the shortcoming of not handling 
 * wide characters nicely.
 *
 * Also, all programs writen with it should 
 * be compiled with '-lm' because it has a 
 * math.h dependency. 
 *
 * And, by a 'string' being the same as a 
 * 'vectors(char)' you should put a reference 
 * for vectors.c anywhere in your program 
 * (as well as strings.c); without it, the 
 * compile will scream at you.
 *
 * 'strings' also pressuposes null 
 * terminated strings.
 *
 * TODO:
 * - trim
 * - join
 * - simplify
 * - revise case-insensitivity
 */

/* 
 * Creates an automatic variable that may be 
 * modified but not expanded.
 *
 * #automatic
 */
#define strings_preinit(buffer, length) \
	char buf_##buffer[length] = ""; \
	buffer = (string) {.data=buf_##buffer, .size=length, .capacity=length}

/* 
 * Creates a static variable from string literal 
 * with size and capacity already specified.
 *
 * Should'nt be modified (view only)!
 *
 * #view-only #tested
 */
#define strings_premake(lit) \
	{.data=lit, .size=sizeof(lit), .capacity=sizeof(lit)}

/*
 * Allocates string (aka vectors(char)) with 
 * quantity mapping its capacity.
 *
 * You need to check if allocation was successfull.
 *
 * Should be freed using 'strings_free'.
 *
 * #allocates #depends:stdio.h #posix-reliant #tested #to-edit
 */
#define strings_init(quantity, mem) vectors_init(char, quantity, mem)

/*
 * Prints a debug-friendly message of 
 * string's information.
 *
 * #debug #depends:stdio.h #posix-reliant
 */
#define strings_debug(s) \
	printf(#s"<string>{.size: %zu, .capacity: %zu, .data: %p}\n", s.size, s.capacity, s.data);

/*
 * Pushes char into s - s being an allocated variable.
 * The err variable is a pointer to a boolean 
 * initialized by false, in which, if an error 
 * happens, it will be put to true. 
 * It may be null to be silently ignored.
 *
 * #allocates #may-fail #depends:stdio.h #posix-reliant #test #to-edit
 */
#define strings_push(s, item, mem, err) \
	if (s.size == 0) { \
		vectors_push(s, item, mem, err); \
	} else { \
		s.data[s.size - 1] = item; \
	} \
	vectors_push(s, '\0', mem, err); \

/* 
 * Checks if string was properly initialized 
 * returning true if something illegal happened.
 *
 * #tested
 */
__attribute_warn_unused_result__
bool strings_check(const string *s);

/* 
 * Checks if string was properly assigned
 * returning true if something illegal happened.
 *
 * #tested
 */
__attribute_warn_unused_result__
bool strings_check_extra(const string *s);

/* 
 * Checks if string is properly within charset.
 *
 * #tested
 */
__attribute_warn_unused_result__
bool strings_check_charset(const string *s, const string *charset);

/*
 * Allocates string from char literal. 
 * Capacity snaps to nearest two powered number.
 *
 * #allocates #may-panic #depends:string.h
 * #tested #posix-reliant #to-edit
 */
__attribute_warn_unused_result__
string strings_make(const char *lit, const allocator *mem);

/*
 * Allocates string from another string.
 *
 * #allocates #may-panic 
 * #depends:string.h #posix-reliant #to-edit
 */
__attribute_warn_unused_result__
string strings_make_copy(const string *s, const allocator *mem);

/*
 * Frees alocated string.
 *
 * Shouldn't be used for non-allocated strings!
 *
 * #may-fail #depends:string.h #posix-reliant 
 * #tested #to-edit
 */
void strings_free(string *s, const allocator *mem);

/*
 * Prints the string to the terminal respecting it's size.
 *
 * #depends:stdio.h #posix-reliant 
 */
void strings_print(const string *s);

/*
 * Prints the string to the terminal respecting it's size 
 * and terminated by a line break.
 *
 * #depends:stdio.h #posix-reliant 
 */
void strings_println(const string *s);

/*
 * Compares if first string is bigger than the second 
 * alphabetically, thus, returning true if it is and false 
 * if is equal or smaller.
 *
 * #case-insensitive #tested #depends:maths.h
 */
__attribute_warn_unused_result__
bool strings_compare(const string *first, const string *second);

/*
 * Compares if first string equals the second completely 
 * returning true in that case.
 *
 * #case-sensitive #tested
 */
__attribute_warn_unused_result__
bool strings_equals(const string *first, const string *second);

/*
 * Compares case-insensitivitely if first string 
 * equals the second completely returning true 
 * in that case.
 *
 * #case-insensitive #tested
 */
__attribute_warn_unused_result__
bool strings_seems(const string *first, const string *second);


/*
 * Finds first ocurrence of sep string inside s from pos and returns 
 * the position, where -1 indicates misuse or that sep wasn't found 
 * within s.
 *
 * #case-insensitive #tested
 */
__attribute_warn_unused_result__
ssize_t strings_find(const string *s, const string *sep, size_t pos);

/*
 * Finds at most n ocurrences of sep string inside s and returns 
 * an allocated vector with all positions where sep was found in.
 *
 * If n is 0 the search is unrestricted.
 *
 * #case-insensitive #may-panic #may-fail #allocates #tested #to-edit
 */
__attribute_warn_unused_result__
size_vec strings_make_find(const string *s, const string *sep, size_t n, const allocator *mem);

/*
 * Replaces at most n ocurrences of any character in seps in s to rep.
 *
 * If n is 0, then the replacement is unrestricted.
 * If rep is less than 0, then the characters matched are deleted 
 * and the whole string is shifted accordingly.
 *
 * #case-sensitive #tested
 */
void strings_replace(string *s, const string *seps, const char rep, size_t n);

/*
 * Creates a new string where the ocurrences of text whithin s 
 * are replaced by rep at most n times.
 *
 * If n is 0, then the replacement is unrestricted.
 * If rep is null, then the text found are simply 
 * not copy'd over to the new string.
 *
 * If no ocurrence of text is found in s, s is 
 * copy'd completely to the new string.
 * 
 * #case-insensitive #allocates #may-fail #tested #to-edit
 */
__attribute_warn_unused_result__
string strings_make_replace(const string *s, const string *text, const string *rep, size_t n, const allocator *mem);

/*
 * Creates a string_vec (aka vectors(string)) containing the 
 * split of s by separator sep at most n times.
 *
 * If n is 0, then the split is unrestricted.
 * If sep is not found, a string_vec with a copy of s 
 * is returned.
 *
 * #case-insensitive #allocates #may-fail #may-panic 
 * #depends:string.h #posix-reliant #tested #to-edit
 */
__attribute_warn_unused_result__
string_vec strings_make_split(const string *s, const string *sep, size_t n, const allocator *mem);

/*
 * Formats the string in form with arguments and 
 * returns it as a string. It obeys to printf format style.
 *
 * #allocates #may-panic #depends:stdio.h #posix-reliant #tested
 *
 * example:
 * string json = strings_make_format("{\"age\": %d}", 10); //{"age": 10}
 */
__attribute_warn_unused_result__
__attribute__ ((__format__ (printf, 1, 3)))
string strings_make_format(const char *const form, const allocator *mem, ...);

/*
 * Hashes string. 
 *
 * #case-insensitive #depends:math.h,ctype.h #tested
 */
__attribute_warn_unused_result__
size_t strings_hasherize(const string *s);

/*
 * Converts characters in s to lowercase.
 *
 * #tested
 */
void strings_lower(string *s);

/*
 * Converts characters in s to uppercase.
 *
 * #tested
 */
void strings_upper(string *s);

/*
 * Iterates through s where each token is separated 
 * by sep and indicates when the iteration has ended 
 * return true. 
 *
 * Each substring is 'assigned' to next which is a view.
 * The next string should be an automatic variable initialy empty. 
 * Also, it isn't null terminated necessarly, so, it shouldn't be 
 * relied uppon.
 *
 * #case-insensitive #iterator #tested
 */
bool strings_next(const string *s, const string *sep, string *next);

/*
 * Verifies if both vectors are equal.
 *
 * #case-sensitive
 */
__attribute_warn_unused_result__
bool string_vecs_equals(const string_vec *v0, const string_vec *v1);

/*
 * Verifies if both vectors are equal case-insensitively.
 *
 * #case-insensitive
 */
__attribute_warn_unused_result__
bool string_vecs_seems(const string_vec *v0, const string_vec *v1);

/*
 * Frees a string_vec deeply - sv.data[i].data must be allocated. 
 *
 * #to-edit
 */
void string_vecs_free(string_vec *sv, const allocator *mem);

//extras #to-edit

#include "nodes.h"

void string_bnode_sets_free(bnode *n, const allocator *mem);

typedef struct string_set string_set;
sets(string);

#define string_sets_push(s, item, mem, err) sets_push(s, item, strings_hasherize, mem, strings_free, err)

#define string_sets_free(s, mem) sets_free(s, mem, string_bnode_sets_free)

void string_bnode_maps_free(bnode *n, const allocator *mem);

typedef key_pairs(string, string) string_key_pair;

typedef struct string_key_pair_map string_key_pair_map;
typedef maps(string_key_pair) string_map;

#define string_maps_push(m, k, v, mem, err) \
	maps_push(m, k, v, strings_hasherize, mem, string_map_items_free, err)

#define string_maps_free(m, mem) \
	maps_free(m, mem, string_bnode_maps_free)

#define string_maps_get(m, lit) \
	string_maps_get_value(m, strings_hasherize(&(string)strings_premake(lit)))

#define string_maps_get_frequency(m, lit) \
	bnodes_get_frequency((bnode *)m, strings_hasherize(&(string)strings_premake(lit)));

/*
 * #to-edit
 */
void string_map_items_free(string_key_pair *ss, const allocator *mem);

string *string_maps_get_value(string_map *s, size_t hash);

#endif
