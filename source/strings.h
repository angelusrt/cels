#ifndef cels_strings_h
#define cels_strings_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <math.h>
#include <ctype.h>

#include "errors.h"
#include "vectors.h"
#include "maths.h"


/*
 * The module 'strings' is about manipulating the 
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
 * terminated strings in alot of places - 
 * where it doesn't, a string_view 
 * is pressuposed.
 *
 * TODO:
 * - simplify
 * - better slice
 * - deprecate shift
 */


/* string */

typedef enum string_size {
	string_min_size = 16,
	string_petit_size = 128,
	string_small_size = 1024,
	string_big_size = 4096,
	string_large_size = 65536,
	string_very_large_size = 1048576, 
	string_max_size = SIZE_MAX,
} string_size;

typedef struct string {
  size_t size;
  size_t capacity;
  char *data;
} string;

typedef string string_view;

typedef errors(string) estring;

typedef vectors(string) string_vec;
typedef vectors(string_view) string_view_vec;

typedef errors(string_vec) estring_vec;

typedef vectors(string_vec) string_mat;

#include "bytes.h"

/*
 * Verifies if character is a either 
 * a space, a carriage-return, a 
 * line-feed or a tab.
 *
 * #to-review
 */
cels_warn_unused
bool chars_is_whitespace(char letter);

/*
 * Verifies if character is "A-Z | a-z | 0-9 | _".
 *
 * #to-review
 */
cels_warn_unused
bool chars_is_alphanumeric(char letter);

/*
 * Prints non-printable-characters only, 
 * defaulting to hexadecimal-char-code-printing.
 *
 * #to-review
 */
void chars_print_special(char letter);

/* Prints printable and non-printable 
 * characters.
 *
 * #to-review
 */
void chars_print_normal(char letter);

/*
 * Checks if string literal is invalid.
 * Returns true if it is.
 *
 * #to-review
 */
bool strs_check(const char *self);


/* string_view */

/*
 * Converts a string_view to string.
 *
 * #to-review
 */
string string_views_to_string(const string_view *self, const allocator *mem);

/* 
 * Checks string, regardless of obeying to null-termination
 * and returns true if something illegal was found.
 *
 * #tested
 */
cels_warn_unused
bool string_views_check(const string *self);


/* strings */

/* 
 * Creates a static variable from string literal 
 * with size and capacity already specified.
 *
 * Should'nt be modified (view only)!
 *
 * #view-only #tested
 */
#define s(lit) \
	{.size=sizeof(lit), .capacity=sizeof(lit), .data=lit}

/* 
 * Creates an automatic variable that may be 
 * modified but not expanded.
 *
 * #automatic
 */
#define strings_preinit(length) \
	{.data=((char[length]){0}), .size=0, .capacity=length}

/* 
 * Creates a static variable from string literal 
 * with size and capacity already specified.
 *
 * Should'nt be modified (view only)!
 *
 * #view-only #tested
 */
#define strings_premake(lit) \
	{.size=sizeof(lit), .capacity=sizeof(lit), .data=lit}

/* 
 * Creates a static variable from string literal 
 * with size and capacity already specified.
 *
 * Should'nt be modified (view only)!
 *
 * This function is a convenience over 
 * strings_premake for using in function 
 * arguments.
 *
 * #view-only #to-review #decanonize?
 */
#define strings_do(lit) (string)strings_premake(lit)

/*
 * Convenience macro that receives 
 * a char literal and returns 
 * a size_t hash.
 *
 * #to-review
 */
#define strings_prehash(lit) \
	strings_hash(&(string)strings_premake(lit))

/* 
 * Checks if string was properly initialized 
 * returning true if something illegal happened.
 *
 * #tested
 */
cels_warn_unused
bool strings_check(const string *self);

/* 
 * Checks if string was properly assigned
 * returning true if something illegal happened.
 *
 * #tested
 */
cels_warn_unused
bool strings_check_extra(const string *self);

/* 
 * Checks if string is properly within charset.
 *
 * #tested
 */
cels_warn_unused
bool strings_check_charset(const string *self, const string charset);

/*
 * Allocates string (aka vectors(char)) with 
 * quantity mapping its capacity.
 *
 * You need to check if allocation was successfull.
 *
 * Should be freed using 'strings-free'.
 *
 * #allocates #depends:stdio.h #posix-reliant 
 * #tested #to-review
 */
string strings_init(size_t quantity, const allocator *mem);

/*
 * Allocates string from char literal. 
 * Capacity snaps to nearest two powered number.
 *
 * #allocates #may-panic #depends:string.h
 * #tested #posix-reliant #to-edit
 */
cels_warn_unused
string strings_make(const char *literal, const allocator *mem);

/*
 * Allocates a new string hard-copying 
 * a portion of 'self'.
 *
 * #allocates #may-panic 
 * #depends:string.h #posix-reliant #to-review
 */
cels_warn_unused
string strings_copy(
	const string *self, size_t start, size_t end, const allocator *mem);

/*
 * Allocates a new string hard-copying 'self'.
 *
 * #allocates #may-panic 
 * #depends:string.h #posix-reliant #to-review
 */
cels_warn_unused
string strings_clone(const string *self, const allocator *mem);

/*
 * Creates a capsule over a string literal 
 * which computes its lenght.
 *
 * #to-review
 */
cels_warn_unused
string strings_encapsulate(const char *literal);

/*
 * Creates a view of 'self' delimited 
 * by start or end.
 *
 * If this function is misused an 
 * empty string is returned.
 *
 * #allocates #may-panic 
 * #depends:string.h #posix-reliant #to-review
 */
cels_warn_unused
string_view strings_view(const string *self, size_t start, size_t end);

/*
 * Pops a character from string.
 * Returns fail if error happens.
 *
 * #allocates #may-fail #depends:stdio.h #posix-reliant #to-review
 */
error strings_pop(string *self, const allocator *mem);

/*
 * Concatenates item into 'self' string.
 * Returns fail if error happens.
 *
 * #allocates #may-fail #depends:stdio.h #posix-reliant #to-review
 */
error strings_push(string *self, string item, const allocator *mem);

/*
 * Concatenates item into 'self' string with 'char *'.
 * Returns fail if error happens.
 *
 * #allocates #may-fail #depends:stdio.h #posix-reliant #to-review
 */
error strings_push_with(string *self, char *item, const allocator *mem);

/*
 * Frees alocated string.
 *
 * Shouldn't be used for non-allocated strings!
 *
 * #may-fail #depends:string.h #posix-reliant 
 * #tested #to-review
 */
void strings_free(string *self, const allocator *mem);

/*
 * Processes escape characters.
 *
 * #to-edit
 */
void strings_normalize(string *self);

/*
 * Prints a debug-friendly message of 
 * string's information.
 *
 * #debug #depends:stdio.h #posix-reliant #to-review
 */
void strings_debug(const string *self);

/*
 * Prints a debug-friendly message of 
 * string's information and line-feeds new-line.
 *
 * #debug #depends:stdio.h #posix-reliant #to-review
 */
void strings_debugln(const string *self);

/*
 * Prints the string to the terminal 
 * respecting it's size.
 *
 * #depends:stdio.h #posix-reliant 
 */
void strings_print(const string *self);

/*
 * Prints the string to the terminal 
 * respecting it's size and terminated 
 * by a line break.
 *
 * #depends:stdio.h #posix-reliant 
 */
void strings_println(const string *self);

/*
 * Prints string to terminal, having it's 
 * non-printable-characters printed symbolically.
 *
 * #to-review
 */
void strings_imprint(const string *self);

/*
 * Compares if 'self' string is bigger than 'other' 
 * alphabetically, thus, returning true if it is 
 * and false if is equal or smaller.
 *
 * #case-insensitive #tested #depends:maths.h
 */
cels_warn_unused
bool strings_compare(const string *self, const string *other);

/*
 * Compares if 'self' string equals 'other' completely,
 * returning true in that case.
 *
 * #case-sensitive #tested
 */
cels_warn_unused
bool strings_equals(const string *self, const string *other);

/*
 * Compares case-insensitivitely if 'self' string 
 * equals 'other' completely, returning true 
 * in that case.
 *
 * #case-insensitive #tested
 */
cels_warn_unused
bool strings_seems(const string *self, const string *other);

/*
 * Finds self ocurrence of substring inside 
 * 'self' from pos and returns the position 
 * of where matching began. 
 *
 * It returns -1 for misuse or if substring 
 * wasn't found.
 *
 * #case-insensitive #to-review
 */
cels_warn_unused
ssize_t strings_find(const string *self, const string substring, size_t pos);

/*
 * Convenience over 'strings_find' which lets 
 * you use string literal for the substring.
 *
 * #case-insensitive #to-review
 */
cels_warn_unused
ssize_t strings_find_with(
	const string *self, const char *substring, size_t pos);

/*
 * Finds any character from seps within 'self' 
 * beginning from position 'pos' and returns 
 * the position of where matching began. 
 *
 * It returns -1 for misuse or if substring 
 * wasn't found.
 *
 * #case-insensitive #to-review
 */
cels_warn_unused
ssize_t strings_find_from(const string *self, const string seps, size_t pos);

/*
 * Finds at most n ocurrences of substring 
 * inside self and returns an allocated vector 
 * with all positions where sep was found in.
 *
 * If n is 0 the search is unrestricted.
 *
 * #case-insensitive #may-panic #may-fail 
 * #allocates #tested #to-review
 */
cels_warn_unused
size_vec strings_find_all(
	const string *self, 
	const string substring, 
	size_t n, 
	const allocator *mem);

/*
 * Finds the respective closing-tag of the provided 
 * opening-tag by counting their occurrences and 
 * returning the found position.
 *
 * If pos is given, the search happens by this mark.
 *
 * #case-sensitive #to-review
 */
cels_warn_unused
ssize_t strings_find_matching(
	const string *self, 
	const string open_tag, 
	const string close_tag, 
	size_t pos);

/*
 * Replaces at most n ocurrences of any character 
 * in seps in self to rep.
 *
 * If n is 0, then the replacement is unrestricted.
 * If rep is less than 0, then the characters matched 
 * are deleted and the whole string is shifted 
 * accordingly.
 *
 * #case-sensitive #tested
 */
void strings_replace_from(
	string *self, const string seps, const char rep, size_t n);

/*
 * Creates a new string where the ocurrences of substring 
 * whithin self are replaced by 'replace' at most n times.
 *
 * If n is 0, then the replacement is unrestricted.
 * If no ocurrence of text is found in self, self is 
 * copy'd completely to the new string.
 * 
 * #case-insensitive #allocates #may-fail #tested #to-edit
 */
 cels_warn_unused
 string strings_replace(
	const string *self, 
	const string substring, 
	const string replace, 
	size_t n, 
	const allocator *mem);

/*
 * Convenience over strings_replace where 'rep' is a 'char *'.
 *
 * #case-insensitive #allocates #may-fail #tested #to-edit
 */
string strings_replace_with(
	const string *self, 
	const string substring, 
	const char *rep, 
	size_t n, 
	const allocator *mem);

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
cels_warn_unused
string_vec strings_split(
	const string *self, const string sep, size_t n, const allocator *mem);

/*
 * A convenience over 'strings_split' which uses 'char *'.
 *
 * #to-review
 */
cels_warn_unused
string_vec strings_split_with(
	const string *self, const char *sep, size_t n, const allocator *mem);

/*
 * Formats the string in form with arguments and 
 * returns it as a string. It obeys to printf format style.
 *
 * #allocates #may-panic #depends:stdio.h 
 * #posix-reliant #tested
 *
 * example:
 * string json = strings_make_format("{\"age\": %d}", 10); //{"age": 10}
 */
cels_warn_unused
__attribute__ ((__format__ (printf, 1, 3)))
string strings_format(const char *const form, const allocator *mem, ...);

/*
 * Hashes string. 
 *
 * #case-insensitive #depends:math.h,ctype.h #tested
 */
cels_warn_unused
size_t strings_hash(const string *self);

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
bool strings_next(const string *self, const string sep, string_view *next);

/*
 * Slices string shifting start and cutting end.
 *
 * #to-review
 */
void strings_slice(string *self, size_t start, size_t end);

/*
 * Shifts whole string eliminating a character.
 * Does nothing if position is invalid.
 *
 * #to-review
 */
void strings_shift(string *self, size_t position, size_t amount);

/*
 * Trims leading white-spaces on both 
 * ends of string.
 *
 * #to-review
 */
void strings_trim(string *self);

/*
 * Cuts leading white-spaces on both 
 * ends of string, returning a view.
 *
 * #to-review #view
 */
string strings_cut(const string *self);

/*
 * Checks if 'self' has suffix (naturaly, at 
 * the end), returning true if it has.
 *
 * #to-review
 */
cels_warn_unused
bool strings_has_suffix(const string *self, const string suffix);

/*
 * Checks if 'self' has prefix (naturaly, at 
 * the beginning), returning true if it has.
 *
 * #to-review
 */
cels_warn_unused
bool strings_has_prefix(const string *self, const string prefix);

/* 
 * Converts string to byte-vec.
 *
 * #tested
 */
cels_warn_unused
byte_vec strings_to_byte_vec(own string *self);


/* string_vecs */

/*
 * Makes string_vecs from char* list.
 *
 * #allocates #tested
 */
#define string_vecs_make(mem, ...) \
  string_vecs_make_helper( \
	  (char*[]){__VA_ARGS__}, \
	  sizeof((char*[]){__VA_ARGS__})/sizeof(char*), \
	  mem)

/*
 * Concatenates strings inside 'self' 
 * with 'sep' in between.
 *
 * #to-review
 */
cels_warn_unused
string string_vecs_join(string_vec *self, string sep, const allocator *mem);

/*
 * Use string_vecs_make instead.
 *
 * #not-to-use #to-review
 */
cels_warn_unused
string_vec string_vecs_make_helper(
	char *args[], size_t argn, const allocator *mem);


/* extras */

#include "nodes.h"


/* sets */

sets(string_set, string)

/*
 * Initiates string_sets.
 *
 * #to-review
 */
string_set string_sets_init(void);

/*
 * Iterates through set.
 *
 * #to-review
 */
bool string_sets_next(const string_set *self, string_set_iterator *it);

/*
 * Gets item in set.
 *
 * #to-review
 */
string *string_sets_get(const string_set *self, string item);

/*
 * Pushes item to set.
 *
 * #to-review
 */
error string_sets_push(string_set *self, string item, const allocator *mem);


/* maps */

maps(string_map, string, string)
typedef errors(string_map) estring_map;

/*
 * Initiates string_maps.
 *
 * #to-review
 */
string_map string_maps_init(void);

/*
 * Iterates through string_map.
 *
 * #to-review
 */
bool string_maps_next(const string_map *self, string_map_iterator *it);

/*
 * Gets value of pair else null.
 *
 * #to-review
 */
string *string_maps_get(const string_map *self, string key);

/*
 * Pushes key and value to map.
 *
 * #to-review
 */
error string_maps_push(
	string_map *self, string key, string value, const allocator *mem);

/*
 * Push key and value over string_map allocating 
 * string's with mem. This function is a 
 * convenience over string_maps_push.
 * 
 * #to-review
 */
error string_maps_push_with(
	string_map *self, const char *key, const char *value, const allocator *mem);


/* lists */

pools(string_pool, string)

#endif
