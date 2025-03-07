#include "files.h"


/* files */

ebyte_vec files_read(file *self, const allocator *mem) {
	#if cels_debug
		errors_abort("self", !self);
	#endif

	error error = ok;
	long current_position = ftell(self);
	if (current_position == -1) {
		error = file_telling_position_error;
		goto cleanup0;
	}

	int seek_error = fseek(self, 0, SEEK_END);
	if (seek_error == -1) {
		error = file_seeking_position_error;
		goto cleanup0;
	}

	long last_position = ftell(self);
	if (last_position == -1) {
		error = file_telling_position_error;
		goto cleanup0;
	}

	seek_error = fseek(self, 0, SEEK_SET);
	if (seek_error == -1) {
		error = file_seeking_position_error;
		goto cleanup0;
	}

	size_t new_capacity = (last_position + 1) - current_position;

	byte_vec buffer = {0};
	vectors_init(&buffer, sizeof(byte), new_capacity, mem);

	ulong bytes_read = fread(buffer.data, 1, buffer.capacity - 1, self);
	buffer.size = bytes_read + 1;

	if (feof(self)) {
		error = file_did_not_end_error;
		goto cleanup1;
	}

	if (ferror(self)) {
		error = file_default_error;
		goto cleanup1;
	}

	#if cels_debug
		errors_abort("buffer", byte_vecs_check(&buffer));
	#endif

	return (ebyte_vec){.value=buffer};

	cleanup1:
	vectors_free(&buffer, null, mem);

	cleanup0:
	return (ebyte_vec){.error=error};
}

bool files_read_async(file *self, file_read *read, const allocator *mem) {
	#if cels_debug
		errors_abort("self", !self);
		errors_abort("read", !read);
		//errors_abort("read.file", strings_check(&read->file));
	#endif

	long current_position = ftell(self);
	if (current_position == -1) {
		read->error = file_telling_position_error;
		goto cleanup0;
	}

	byte_vec *f = &read->file;
	if (f->data) {
		vectors_init(f, sizeof(byte), string_small_size, mem);
	}

	size_t rest = (f->capacity - f->size);
	size_t step = maths_min(rest, read->size);

	long bytes_read = fread(f->data + f->size, 1, step, self);
	f->size += bytes_read + 1;

	if (bytes_read == 0) {
		return false;
	}

	if (ferror(self)) {
		read->error = file_default_error;
		goto cleanup1;
	}

	#if cels_debug
		errors_abort("f", byte_vecs_check(f));
	#endif

	return true;

	cleanup1:
	vectors_free(&read->file, null, mem);

	cleanup0:
	return false;
}

error files_write(file *self, const byte_vec text) {
	#if cels_debug
		errors_abort("self", !self);
		errors_abort("text", byte_vecs_check(&text));
	#endif

	long write_error = fwrite(text.data, 1, text.size - 1, self);
	if (write_error < (long)text.size) {
		return file_writing_error;
	}

	return file_successfull;
}

bool files_write_async(file *self, file_write *file_write) {
	#if cels_debug
		errors_abort("self", !self);
		errors_abort("file_write", !file_write);
	#endif

	byte_vec *f = &file_write->file;
	ssize_t rest = f->size - 1 - file_write->internal.position;
	#if cels_debug
		errors_abort("rest is negative", rest < 0);
	#else
		if (rest < 0) { return false; }
	#endif

	byte *offset = f->data + file_write->internal.position;
	size_t step = maths_min(rest, (ssize_t)file_write->size);
	size_t writen = fwrite(offset, 1, step, self);

	if ((long)writen < (long)step) {
		file_write->error = file_writing_error;
		return false;
	}

	if (file_write->consume) {
		vectors_shift(f, 0, writen, null, null);

		#if cels_debug
			errors_abort("f", byte_vecs_check(f));
		#endif
	} else {
		file_write->internal.position += writen;
	}

	if (writen == 0) {
		return false;
	}

	return true;
}

ssize_t files_find(file *self, const byte_vec substring, ssize_t pos) {
	#if cels_debug
		errors_abort("self", !self);
		errors_abort("substring", byte_vecs_check(&substring));
	#endif

	if (pos >= 0) {
		error seek_error = fseek(self, pos, SEEK_SET);
		if (seek_error == -1) { return -2; }
	} else {
		pos = ftell(self);
		if (pos == -1) { return -2; }
	}

	char letter = '\0';
	size_t i = 0;
	size_t j = pos;
	do {
		letter = tolower(fgetc(self));

		if (tolower(substring.data[i]) == letter) {
			i++;
		} else if (tolower(substring.data[0]) == letter) {
			i = 1;
		} else {
			i = 0;
		}

		j++;
		if (i == substring.size - 1) { return j - i; }
	} while(letter != EOF);

	return -1;
}

ssize_t files_find_from(file *self, const byte_vec seps, ssize_t pos) {
	#if cels_debug
		errors_abort("self", !self);
		errors_abort("seps", byte_vecs_check(&seps));
	#endif

	if (pos >= 0) {
		ssize_t position = fseek(self, pos, SEEK_SET);
		if (position == -1) { return -2; }
	} else {
		ssize_t position = ftell(self);
		if (position == -1) { return -2; }
		pos = position;
	}

	char current_character = '\0';
	do {
		current_character = fgetc(self);

		for (size_t i = 0; i < seps.size - 1; i++) {
			if (current_character == seps.data[i]) {
				return pos;
			}
		}

		pos++;
	} while (current_character != EOF);

	return -1;
}

bool files_next(file *self, byte_vec *line, const allocator *mem) {
	#if cels_debug
		errors_abort("self", !self);
	#endif

	long current_position = ftell(self);
	if (current_position == -1) {
		return file_telling_position_error;
	}

	if (!line->data) {
		byte_vec buffer = {0};
		vectors_init(&buffer, sizeof(byte), string_small_size, mem);
		*line = buffer;

		if (current_position != 0) {
			long pos = fseek(self, 0, SEEK_SET);
			if (pos == -1) { return file_seeking_position_error; }
		}
	}

	#if cels_debug
		errors_abort("line", byte_vecs_check(line));
	#endif

	const byte_vec line_sep = vectors_premake(byte, '\n', EOF, '\0');

	ssize_t next_position = files_find_from(
		self, line_sep, current_position + 1);

	if (next_position == -1) { return true; }

	size_t capacity = next_position + 1 - current_position;
	while (capacity > line->capacity) {
		error upscale_error = vectors_upscale(line, mem);
		if (upscale_error) { return file_allocation_error; }
	}

	long seek_error = fseek(self, current_position, SEEK_SET);
	if (seek_error == -1) { return file_seeking_position_error; }

	size_t bytes_read = fread(line->data, 1, capacity, self);
	if (bytes_read == 0) { return file_reading_error; }

	line->size = bytes_read;
	line->data[line->size - 1] = '\0';

	if (ferror(self)) { return file_default_error; }

	return false;
}


/* paths */

estring paths_make(const string *filepath, const allocator *mem) {
	#if cels_debug
		errors_abort("filepath", strings_check_extra(filepath));
	#endif

	if (filepath->data[0] == '/') {
		return (estring){.value=strings_clone(filepath, mem)};
	} else if (filepath->data[0] != '.') {
		//currently the default for other filesystems
		return (estring){.value=strings_clone(filepath, mem)};
	}

	string working_directory = strings_preinit(PATH_MAX);
	char *is_successfull = getcwd(
		working_directory.data, working_directory.capacity);

	if (!is_successfull) {
		return (estring){.error=file_current_directory_not_read_error};
	}

	working_directory.size = strlen(working_directory.data) + 1;
	working_directory.data[working_directory.size - 1] = '\0';

	string path = strings_format(
		"%s" file_sep "%s", 
		mem, 
		working_directory.data, 
		filepath->data);

	estring path_normalized = paths_normalize(&path, mem);
	strings_free(&path, mem);

	return path_normalized;
}

estring paths_normalize(const string *filepath, const allocator *mem) {
	#if cels_debug
		errors_abort("filepath", strings_check_extra(filepath));
	#endif

	static const string file_sep2 = strings_premake(file_sep);
	static const string one_dots = strings_premake(".");
	static const string two_dots = strings_premake("..");

	string path_normalized = strings_init(vector_min, mem);
	string_vec file_nodes = strings_split(filepath, file_sep2, 0, mem);

	size_t i = 0;
	while(true) {
		if (strings_seems(&file_nodes.data[i], &two_dots)) {
			if (i == 0) { goto cleanup; }

			vectors_shift(&file_nodes, i - 1, 2, (freefunc)strings_free, mem);
			i--;
		} else if (strings_seems(&file_nodes.data[i], &one_dots)) {
			vectors_shift(&file_nodes, i, 1, (freefunc)strings_free, mem);
		} else {
			i++;
		}

		if (i == file_nodes.size - 1) {
			break;
		}
	}

	for (size_t i = 0; i < file_nodes.size; i++) {
		error push_error = strings_push(&path_normalized, file_sep2, mem);
		if (push_error) { goto cleanup; }

		push_error = strings_push(&path_normalized, file_nodes.data[i], mem);
		if (push_error) { goto cleanup; }
	}

	vectors_free(&file_nodes, (freefunc)strings_free, mem);
	return (estring){.value=path_normalized};

	cleanup:
	strings_free(&path_normalized, mem);
	vectors_free(&file_nodes, (freefunc)strings_free, mem);
	return (estring){.error=file_mal_formed_error};
}


/* dirs */

error dirs_make(const char *path, notused __mode_t mode) {
	#if cels_debug
		errors_abort("path", strs_check(path));
	#endif

	error error = ok;

	#ifdef _WIN32
		struct stat st = {0};
		if (_stat(path, &st) != -1) {
			return file_directory_already_exists_error;
		}

		error = _mkdir(name);
	#else
		struct stat st = {0};
		if (stat(path, &st) != -1) {
			return file_directory_already_exists_error;
		}

		error = mkdir(path, mode); 
	#endif

	return error == fail ? file_directory_not_created_error : ok;
}

estring_vec dirs_list(const string path, const allocator *mem) {
	#if cels_debug
		errors_abort("path", strings_check_extra(&path));
	#endif

	dir *directory = opendir(path.data);
	if (!directory) {
		return (estring_vec){.error=file_directory_not_opened_error};
	}

	string_vec files = {0};
	vectors_init(&files, sizeof(string), vector_min, mem);

	struct dirent *entity;

	while ((entity = readdir(directory))) {
		if (entity->d_name[0] == '.') {
			if (entity->d_name[1] == '\0') {
				continue;
			} else if (entity->d_name[1] == '.') {
				if (entity->d_name[2] == '\0') {
					continue;
				}
			}
		}

		if (entity->d_type != DT_REG || entity->d_type != DT_DIR) {
			continue;
		}

		string file = strings_make(entity->d_name, mem);

		bool push_error = vectors_push(&files, &file, mem);
		if (push_error) {
			vectors_free(&files, (freefunc)strings_free, mem);
			strings_free(&file, mem);

			return (estring_vec){.error=file_allocation_error};
		}
	}

	closedir(directory);
	return (estring_vec){.value=files};
}

estring_vec dirs_list_all(const string path, const allocator *mem) {
	error err = ok;

	string_vec shallow_files = {0};
	vectors_init(&shallow_files, sizeof(string), vector_min, mem);

	estring_vec files = dirs_list(path, mem); 
	if (files.error != file_successfull) {
		err = files.error;
		goto cleanup0;
	}

	for (size_t i = 0; i < files.value.size; i++) {
		dir *directory = opendir(files.value.data[i].data);
		if (!directory) { 
			string file = strings_clone(&files.value.data[i], mem);
			error push_error = vectors_push(&shallow_files, &file, mem);
			if (push_error) {
				strings_free(&file, mem);
				err = file_allocation_error;
				goto cleanup1;
			}

			continue; 
		}

		struct dirent *entity;
		while ((entity = readdir(directory))) {
			if (entity->d_name[0] == '.') {
				if (entity->d_name[1] == '\0') {
					continue;
				} else if (entity->d_name[1] == '.') {
					if (entity->d_name[2] == '\0') {
						continue;
					}
				}
			}

			if (entity->d_type == DT_REG) {
				string file = strings_make(entity->d_name, mem);

				error push_error = vectors_push(&shallow_files, &file, mem);
				if (push_error) {
					strings_free(&file, mem);
					err = file_allocation_error;
					closedir(directory);
					goto cleanup1;
				}
			} else if (entity->d_type == DT_DIR) {
				string file_capsule = strings_encapsulate(entity->d_name);
				estring_vec files = dirs_list_all(file_capsule, mem);
				if (files.error != file_successfull) {
					continue;
				}

				error unite_error = vectors_unite(
					&shallow_files, &files.value, mem);

				if (unite_error) {
					err = file_allocation_error;
					closedir(directory);
					goto cleanup1;
				}
			} else {
				continue;
			}
		}

		closedir(directory);
	}

	vectors_free(&files.value, (freefunc)strings_free, mem);
	return (estring_vec){.value=shallow_files};

	cleanup1:
	vectors_free(&files.value, (freefunc)strings_free, mem);

	cleanup0:
	vectors_free(&shallow_files, (freefunc)strings_free, mem);
	return (estring_vec){.error=err};
}

bool dirs_next(
	const char *path, dir_iterator *iterator, notused const allocator *mem) {

	#if cels_debug
		errors_abort("path", strs_check(path));
	#endif

	#if _WIN32 
		WIN32_FIND_DATA descriptor;

		if (!iterator->internal.directory) {
			char path_normalized[MAX_PATH] = {0};
			strcat(path_normalized, path);
			strcat(path_normalized, "\\*.*");

			HANDLE handle = FindFirstFile(path_normalized, &descriptor);
			if (handle == INVALID_HANDLE_VALUE) {
				iterator->error = file_directory_not_opened_error;
				return false;
			}

			iterator->internal.directory = mems_alloc(mem, sizeof(dir));
			if (!iterator->internal.directory) {
				FindClose(handle);      
				iterator->error = file_allocation_error;
				return false;
			} 

			iterator->internal.directory = handle;

			if (descriptor.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				iterator->type = DT_DIR;
			} else {
				iterator->type = DT_REG;
			}

			iterator->data = descriptor.cFileName;
			return true;
		}
		
		dir *directory = iterator->internal.directory;
		while (FindNextFile(directory->handle, &descriptor)) {
			if (descriptor.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				iterator->type = DT_DIR;
			} else {
				iterator->type = DT_REG;
			}

			iterator->data = descriptor.cFileName;
			return true;
		}

		FindClose(iterator->internal.directory);	
		mems_dealloc(mem, iterator->internal.directory, sizeof(dir));
	#else
		if (!iterator->internal.directory) {
			iterator->internal.directory = opendir(path);
			if (!iterator->internal.directory) {
				iterator->error = file_directory_not_opened_error;
				return false;
			}
		}

		struct dirent **ent = &iterator->internal.entity; 
		dir *directory = iterator->internal.directory;

		while ((*ent = readdir(directory))) {
			if ((*ent)->d_name[0] == '.') {
				if ((*ent)->d_name[1] == '\0') {
					continue;
				} else if ((*ent)->d_name[1] == '.') {
					if ((*ent)->d_name[2] == '\0') {
						continue;
					}
				}
			}

			iterator->type = (*ent)->d_type;
			iterator->data = strings_encapsulate((*ent)->d_name);
			return true;
		}

		closedir(directory);
	#endif

	return false;
}

void dir_iterators_free(dir_iterator *iterator, notused const allocator *mem) {
	#if _WIN32
		FindClose(iterator->internal.directory);
		mems_dealloc(mem, iterator->internal.directory, sizeof(dir));
	#else
		closedir(iterator->internal.directory);
	#endif
}
