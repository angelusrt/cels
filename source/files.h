#ifndef files_h
#define files_h
#pragma once

#include <sys/cdefs.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#include "errors.h"
#include "strings.h"
#include "vectors.h"
#include "mems.h"

typedef FILE file;
typedef DIR dir;

typedef enum file_error {
	file_successfull,
	file_telling_position_error,
	file_seeking_position_error,
	file_writing_error,
	file_did_not_end_error,
	file_other_error,
	file_directory_not_opened_error,
	file_allocation_error,
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
ssize_t files_find(file *self, string seps, ssize_t pos);

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
bool files_next(file *self, string *line, const allocator *mem);

#endif
