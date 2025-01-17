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

/* char_vecs */

vectors_generate_definition(char, char_vec)
typedef char_vec string;

/*
 * Verifies if character is a either 
 * a space, a carriage-return, a 
 * line-feed or a tab.
 *
 * #to-review
 */
__attribute_warn_unused_result__
bool chars_is_whitespace(char letter);

/*
 * Prints non-printable-characters only, 
 * defaulting to hexadecimal-char-code-printing.
 *
 * #to-review
 */
void chars_print_special(char letter);

/* string_extras */

typedef errors(string) estring;

typedef enum string_size {
	string_min_size = 16,
	string_petit_size = 128,
	string_small_size = 1024,
	string_big_size = 4096,
	string_large_size = 65536,
	string_very_large_size = 1048576, 
	string_max_size = SIZE_MAX,
} string_size;

/* string_vecs */

vectors_generate_definition(string, string_vec)
typedef errors(string_vec) estring_vec;

/*
 * Makes string_vecs from char* list.
 *
 * #allocates #to-review
 */
/*#define string_vecs_make(mem, args...) ({ \
  char *_args[] = {args}; \
  string_vecs_make_helper(_args, sizeof(_args)/sizeof(char*), mem); \
})*/
#define string_vecs_make(mem, ...) \
  string_vecs_make_helper((char*[]){__VA_ARGS__}, sizeof((char*[]){__VA_ARGS__})/sizeof(char*), mem)

/*
 * Concatenates strings inside 'self' 
 * with 'sep' in between.
 *
 * #to-review
 */
__attribute_warn_unused_result__
string string_vecs_join(string_vec *self, string sep, const allocator *mem);

/*
 * Use string_vecs_make instead.
 *
 * #not-to-use #to-review
 */
__attribute_warn_unused_result__
string_vec string_vecs_make_helper(char *args[], size_t argn, const allocator *mem);


/* strings */

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
 * Convenience macro that receives 
 * a char literal and returns 
 * a size_t hash.
 *
 * #to-review
 */
#define strings_prehash(lit) \
	strings_hasherize(&(string)strings_premake(lit))

//#define self(lit) strings_premake(lit)

/* 
 * Checks if string was properly initialized 
 * returning true if something illegal happened.
 *
 * #tested
 */
__attribute_warn_unused_result__
bool strings_check(const string *self);

/* 
 * Checks if string was properly assigned
 * returning true if something illegal happened.
 *
 * #tested
 */
__attribute_warn_unused_result__
bool strings_check_extra(const string *self);

/* 
 * Checks if string is properly within charset.
 *
 * #tested
 */
__attribute_warn_unused_result__
bool strings_check_charset(const string *self, const string *charset);

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
 * Allocates a new string hard-copying 'self'.
 *
 * #allocates #may-panic 
 * #depends:string.h #posix-reliant #to-review
 */
__attribute_warn_unused_result__
string strings_clone(const string *self, const allocator *mem);

/*
 * Makes a null-terminated string copy 
 * of 'self' - being a string_view.
 *
 * #to-review
 */
__attribute_warn_unused_result__
string strings_unview(const string *self, const allocator *mem);

/*
 * Pops a character from string.
 * Returns true if any error happend.
 *
 * #allocates #may-fail #depends:stdio.h #posix-reliant #to-review
 */
bool strings_pop(string *self, const allocator *mem);

/*
 * Concatenates item into 'self' string.
 * Returns true if any error happend.
 *
 * #allocates #may-fail #depends:stdio.h #posix-reliant #to-review
 */
bool strings_push(string *self, string item, const allocator *mem);

/*
 * Frees alocated string.
 *
 * Shouldn't be used for non-allocated strings!
 *
 * #may-fail #depends:string.h #posix-reliant 
 * #tested #to-edit
 */
void strings_free(string *self, const allocator *mem);

/*
 * Prints a debug-friendly message of 
 * string'self information.
 *
 * #debug #depends:stdio.h #posix-reliant #to-review
 */
void strings_debug(const string *self);

/*
 * Prints the string to the terminal respecting it'self size.
 *
 * #depends:stdio.h #posix-reliant 
 */
void strings_print(const string *self);

/*
 * Prints the string to the terminal respecting it'self size 
 * and terminated by a line break.
 *
 * #depends:stdio.h #posix-reliant 
 */
void strings_println(const string *self);

/*
 * Prints string to terminal, having it'self 
 * non-printable-characters printed symbolically.
 *
 * #to-review
 */
void strings_print_clean(const string *self);

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
 * Compares if first string equals the second completely,
 * returning true in that case.
 *
 * #case-sensitive #tested
 */
__attribute_warn_unused_result__
bool strings_equals(const string *first, const string *second);

/*
 * Compares case-insensitivitely if first string 
 * equals the second completely, returning true 
 * in that case.
 *
 * #case-insensitive #tested
 */
__attribute_warn_unused_result__
bool strings_seems(const string *first, const string *second);


/*
 * Finds first ocurrence of sep string inside self from pos and returns 
 * the position, where -1 indicates misuse or that sep wasn't found 
 * within self.
 *
 * #case-insensitive #tested
 */
__attribute_warn_unused_result__
ssize_t strings_find_from(const string *self, const string *seps, size_t pos);

/*
 * Finds at most n ocurrences of sep string inside self and returns 
 * an allocated vector with all positions where sep was found in.
 *
 * If n is 0 the search is unrestricted.
 *
 * #case-insensitive #may-panic #may-fail #allocates #tested #to-review
 */
__attribute_warn_unused_result__
size_vec strings_find(const string *self, const string *substring, size_t n, const allocator *mem);

/*
 * Finds the respective closing-tag of the provided 
 * opening-tag by counting their occurrences and 
 * returning the found position.
 *
 * If pos is given, the search happens by this mark.
 *
 * #case-sensitive #to-review
 */
__attribute_warn_unused_result__
ssize_t strings_find_closing_tag(const string *self, const string open_tag, const string close_tag, size_t pos);

/*
 * Replaces at most n ocurrences of any character in seps in self to rep.
 *
 * If n is 0, then the replacement is unrestricted.
 * If rep is less than 0, then the characters matched are deleted 
 * and the whole string is shifted accordingly.
 *
 * #case-sensitive #tested
 */
void strings_replace_from(string *self, const string *seps, const char rep, size_t n);

/*
 * Creates a new string where the ocurrences of text whithin self 
 * are replaced by rep at most n times.
 *
 * If n is 0, then the replacement is unrestricted.
 * If rep is null, then the text found are simply 
 * not copy'd over to the new string.
 *
 * If no ocurrence of text is found in self, self is 
 * copy'd completely to the new string.
 * 
 * #case-insensitive #allocates #may-fail #tested #to-edit
 */
__attribute_warn_unused_result__
string strings_replace(const string *self, const string *text, const string *rep, size_t n, const allocator *mem);

/*
 * Creates a string_vec (aka vectors(string)) containing the 
 * split of self by separator sep at most n times.
 *
 * If n is 0, then the split is unrestricted.
 * If sep is not found, a string_vec with a copy of self 
 * is returned.
 *
 * #case-insensitive #allocates #may-fail #may-panic 
 * #depends:string.h #posix-reliant #tested #to-edit
 */
__attribute_warn_unused_result__
string_vec strings_split(const string *self, const string *sep, size_t n, const allocator *mem);

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
string strings_format(const char *const form, const allocator *mem, ...);

/*
 * Hashes string. 
 *
 * #case-insensitive #depends:math.h,ctype.h #tested
 */
__attribute_warn_unused_result__
size_t strings_hasherize(const string *self);

/*
 * Converts characters in self to lowercase.
 *
 * #tested
 */
void strings_lower(string *self);

/*
 * Converts characters in self to uppercase.
 *
 * #tested
 */
void strings_upper(string *self);

/*
 * Iterates through self where each token is separated 
 * by sep and indicates when the iteration has ended 
 * returning true. 
 *
 * Each substring is 'assigned' to next which is a view.
 * The next string should be an automatic variable initialy empty. 
 * Also, it isn't null terminated necessarly, so, it shouldn't be 
 * relied uppon.
 *
 * #case-insensitive #iterator #tested
 */
bool strings_next(const string *self, const string *sep, string *next);

/*
 * Shifts whole string eliminating a character.
 * Does nothing if position is invalid.
 *
 * #to-review
 */
void strings_shift(string *self, size_t position);

/*
 * Trims leading white-spaces on both 
 * ends of string.
 *
 * #to-review
 */
void strings_trim(string *self);

/*
 * Checks if 'self' has suffix (naturaly, at 
 * the end), returning true if it has.
 *
 * #to-review
 */
__attribute_warn_unused_result__
bool strings_has_suffix(const string *self, const string suffix);


/*
 * Checks if 'self' has prefix (naturaly, at 
 * the beginning), returning true if it has.
 *
 * #to-review
 */
__attribute_warn_unused_result__
bool strings_has_prefix(const string *self, const string prefix);

/* string_bivecs */

vectors_generate_definition(string_vec, string_bivec)

/* extras */

#include "nodes.h"

/* sets */

sets_generate_definition(string, string_set)

/* maps */

maps_generate_definition(string, string, string_key_pair, string_map)
typedef errors(string_map *) estring_map;

/*
 * Push key and value over string_map allocating string'self with mem.
 * This function is a convenience over string_maps_push.
 * 
 * #to-review
 */
bool string_maps_make_push(
	string_map **self, const char *key, const char *value, const allocator *mem
);

#endif
