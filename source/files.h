#ifndef cels_files_h
#define cels_files_h

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#include "utils.h"
#include "errors.h"
#include "strings.h"
#include "vectors.h"
#include "mems.h"

#if _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <sys/stat.h>
#endif


/*
 * The module 'files' deals with 
 * manipulations over the file-system,
 * like file and folder manipulations.
 */


/* files */

typedef FILE file;

typedef enum file_error {
	file_successfull,
	file_default_error,
	file_telling_position_error,
	file_seeking_position_error,
	file_writing_error,
	file_reading_error,
	file_did_not_end_error,
	file_directory_already_exists_error,
	file_directory_not_created_error,
	file_directory_not_opened_error,
	file_allocation_error,
	file_mal_formed_error,
	file_current_directory_not_read_error,
} file_error;

typedef struct file_read {
	byte_vec file;
	size_t size;
	error error;
} file_read;

typedef struct file_write_internal {
	size_t position;
} file_write_internal;

typedef struct file_write {
	byte_vec file;
	size_t size;
	error error;
	bool consume;
	file_write_internal internal;
} file_write;

/*
 * Read files content to string 
 * and if any error happens a 
 * file_error is returned.
 *
 * #to-review
 */
cels_warn_unused
ebyte_vec files_read(file *self, const allocator *mem);

/*
 * Read files without blocking.
 *
 * #to-review
 */
cels_warn_unused
bool files_read_async(file *self, file_read *read, const allocator *mem);

/*
 * Write text to file and reports 
 * a file_error if any happened.
 *
 * #to-review
 */
error files_write(file *self, const byte_vec text);

/*
 * Write text to file non-blocking.
 *
 * #to-review
 */
bool files_write_async(file *self, file_write *file_write);

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
 * or -2 if a seek/tell fails.
 *
 * #to-review
 */
cels_warn_unused
ssize_t files_find(file *self, const byte_vec substring, ssize_t pos);

/*
 * Finds first of any character in seps within 
 * file starting from position pos and returns 
 * the position where it was found.
 *
 * If pos is negative, the search begins where 
 * the file position is set.
 *
 * If nothing is found a -1 is returned or a 
 * -2 if a seek/tell fails.
 *
 * #to-review
 */
cels_warn_unused
ssize_t files_find_from(file *self, byte_vec seps, ssize_t pos);

/*
 * Gets next line and puts it in buffer 'line'.
 *
 * If string is not allocated or a new-line 
 * is too big, mem is used to allocate the 
 * sufficient space.
 *
 * It returns false when it ends.
 *
 * byte_vec may be converted to string, if the 
 * file isn't bynary.
 *
 * #to-review
 */
cels_warn_unused
bool files_next(file *self, byte_vec *line, const allocator *mem);

/*
 * Normalizes filepath, eliminating ".." and ".".
 *
 * If filepath is mal-formed or an allocation 
 * error happens, file_error is returned.
 *
 */
cels_warn_unused
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
cels_warn_unused
estring files_path(const string *filepath, const allocator *mem);


/* dirs */

typedef DIR dir;

typedef struct dir_iterator_internal {
	dir *directory;
	#ifdef __linux__
	struct dirent *entity;
	#endif
} dir_iterator_internal;

typedef struct dir_iterator {
	string data; /* view-only */
	int type;
	error error;
	dir_iterator_internal internal;
} dir_iterator;

/*
 * Creates directory.
 *
 * If you're targeting windows leave mode untoched.
 *
 * #to-review
 */
error dirs_make(const char *path, notused __mode_t mode);

/*
 * Lists all files and folders of 
 * a directory shallowly. 
 *
 * If any error happens a file_error 
 * is returned.
 *
 * #posix-only #to-review
 */
cels_warn_unused
estring_vec dirs_list(const string path, const allocator *mem);

/*
 * Lists files and directories deeply.
 *
 * #posix-only #to-review
 */
cels_warn_unused
estring_vec dirs_list_all(const string path, const allocator *mem);

/*
 * Iterates through directory entities.
 *
 * #allocates #implicitly-allocates #to-review 
 */
bool dirs_next(const char *path, dir_iterator *iterator, const allocator *mem);

/*
 * Frees and closes the internal state of 'dirs_next' 
 * if it was short-circuit'ed.
 *
 * #to-review
 */
void dir_iterators_free(dir_iterator *iterator, const allocator *mem);

#endif
