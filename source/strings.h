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

vectors_generate_definition(char, char_vec)

typedef char_vec string;
typedef vectors(string *) string_vec;                                                \
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
 * Allocates string (aka vectors(char)) with 
 * quantity mapping its capacity.
 *
 * You need to check if allocation was successfull.
 *
 * Should be freed using 'strings_free'.
 *
 * #allocates #depends:stdio.h #posix-reliant #tested #to-review
 */
string strings_init(size_t quantity, const allocator *mem);

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
 * Pushes char into s - s being an allocated variable.
 * The err variable is a pointer to a boolean 
 * initialized by false, in which, if an error 
 * happens, it will be put to true. 
 * It may be null to be silently ignored.
 *
 * #allocates #may-fail #depends:stdio.h #posix-reliant #test #to-review
 */
bool strings_push(string *self, char item, const allocator *mem);

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
 * Prints a debug-friendly message of 
 * string's information.
 *
 * #debug #depends:stdio.h #posix-reliant #to-review
 */
void strings_debug(const string *self);

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

/**string_vecs**/

__attribute_warn_unused_result__
string_vec string_vecs_init(size_t len, const allocator *mem);

/*
 * Frees a string_vec deeply - sv.data[i].data must be allocated. 
 *
 * #to-edit
 */
void string_vecs_free(string_vec *self, const allocator *mem);

bool string_vecs_push(string_vec *self, string item, const allocator *mem);

void string_vecs_sort(string_vec *self, compfunc compare);

void string_vecs_debug(const string_vec *self);

void string_vecs_print(const string_vec *self);

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

/* extras */

#include "nodes.h"

/* sets */

sets_generate_definition(string, string_set)

/* maps */
maps_generate_definition(string, string, string_key_pair, string_map)

bool string_maps_make_push(string_map **self, const char *key, const char *value, const allocator *mem);

#endif
