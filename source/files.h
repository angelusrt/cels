#ifndef files_h
#define files_h
#pragma once

#include <sys/cdefs.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#include "utils.h"
#include "errors.h"
#include "strings.h"
#include "vectors.h"
#include "mems.h"

#if cels_windows
#include <direct.h>
#else
#include <sys/stat.h>
#endif

typedef FILE file;
typedef DIR dir;

typedef enum file_error {
	file_successfull,
	file_telling_position_error,
	file_seeking_position_error,
	file_writing_error,
	file_reading_error,
	file_did_not_end_error,
	file_other_error,
	file_directory_already_exists_error,
	file_directory_not_created_error,
	file_directory_not_opened_error,
	file_allocation_error,
	file_mal_formed_error,
} file_error;

/*
 * TODO: add checks.
 */

/*
 * Read files content to string 
 * and if any error happens a 
 * file_error is returned.
 *
 * #to-review
 */
__attribute_warn_unused_result__
estring files_read(file *self, const allocator *mem);

/*
 * Write text to file and reports 
 * a file_error if any happened.
 *
 * #to-review
 */
error files_write(file *self, const string text);

/*
 * Lists all files and folders of 
 * a directory shallowly. 
 *
 * If any error happens a file_error 
 * is returned.
 *
 * #posix-only #to-review
 */
__attribute_warn_unused_result__
estring_vec files_list(const string path, const allocator *mem);

/*
 * Finds first substring within file 
 * starting with position 'pos' 
 * and returns the position of where the 
 * matching began. 
 *
 * If pos is negative, the search begins where 
 * the file position is set.
 *
 * If no substring is found -1 is returned 
 * or -2 if a seek fails.
 *
 * #to-review
 */
__attribute_warn_unused_result__
ssize_t files_find(file *self, string substring, ssize_t pos);

/*
 * Finds first of any character in seps within 
 * file starting from position pos and returns 
 * the position where it was found.
 *
 * If pos is negative, the search begins where 
 * the file position is set.
 *
 * If nothing is found or this function is 
 * mal-used, it returns -1.
 *
 * #to-review
 */
__attribute_warn_unused_result__
ssize_t files_find_from(file *self, string seps, ssize_t pos);

/*
 * Gets next line and puts it in buffer 'line'.
 *
 * If string is not allocated or a new-line 
 * is too big, mem is used to allocate the 
 * sufficient space.
 *
 * It returns false when it ends.
 *
 * #to-review
 */
__attribute_warn_unused_result__
bool files_next(file *self, string *line, const allocator *mem);

/*
 * Normalizes filepath, eliminating ".." and ".".
 *
 * If filepath is mal-formed or an allocation 
 * error happens, file_error is returned.
 *
 */
__attribute_warn_unused_result__
estring files_normalize(const string *filepath, const allocator *mem);

/*
 * Normalizes path and concatenates to 
 * working-directory path to make it absolute.
 *
 * If filepath is mal-formed or an allocation 
 * error happens, file_error is returned.
 *
 * #to-review
 */
__attribute_warn_unused_result__
estring files_path(const string *filepath, const allocator *mem);

/*
 * Creates directory.
 *
 * If you're targeting windows leave mode untoched.
 *
 * #to-review
 */
error files_make_directory(const char *path, notused __mode_t mode);

#endif
