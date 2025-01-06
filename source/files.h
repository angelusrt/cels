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

typedef enum file_error {
	file_successfull,
	file_telling_position_error,
	file_seeking_position_error,
	file_did_not_end_error,
	file_other_error,
	file_directory_not_opened_error,
	file_allocation_error,
} file_error;

/*
 */
__attribute_warn_unused_result__
estring files_read(file *f, const allocator *mem);

/*
 * #posix-only
 */
__attribute_warn_unused_result__
estring_vec files_list(const string path, const allocator *mem);

#endif
