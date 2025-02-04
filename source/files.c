#include "files.h"
#include "strings.h"

estring files_read(file *self, const allocator *mem) {
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
	string buffer = strings_init(new_capacity, mem);

	long bytes_read = fread(buffer.data, 1, buffer.capacity - 1, self);
	buffer.size = bytes_read + 1;

	if (feof(self)) {
		error = file_did_not_end_error;
		goto cleanup1;
	}

	if (ferror(self)) {
		error = file_other_error;
		goto cleanup1;
	}

	#if cels_debug
		errors_abort("buffer", strings_check_extra(&buffer));
	#endif

	return (estring){.value=buffer};

	cleanup1:
	strings_free(&buffer, mem);

	cleanup0:
	return (estring){.error=error};
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

	if (read->file.data) {
		read->file = strings_init(string_small_size, mem);
	}

	size_t step = maths_min((read->file.capacity - read->file.size), read->size);
	long bytes_read = fread(read->file.data + read->file.size, 1, step, self);
	read->file.size += bytes_read + 1;

	if (bytes_read == 0) {
		return false;
	}

	if (ferror(self)) {
		read->error = file_other_error;
		goto cleanup1;
	}

	return true;

	cleanup1:
	strings_free(&read->file, mem);

	cleanup0:
	return false;
}

error files_write(file *self, const string text) {
	#if cels_debug
		errors_abort("self", !self);
		errors_abort("text", strings_check_extra(&text));
	#endif

	long write_error = fwrite(text.data, 1, text.size - 1, self);
	if (write_error < (long)text.size - 1) {
		return file_writing_error;
	}

	return file_successfull;
}

bool files_write_async(file *self, file_write *file_write) {
	#if cels_debug
		errors_abort("self", !self);
		errors_abort("file_write", !file_write);
	#endif

	ssize_t rest = file_write->file.size - 1 - file_write->internal.position;
	#if cels_debug
		errors_abort("rest is negative", rest < 0);
	#else
		if (rest < 0) { return false; }
	#endif

	char *offset = file_write->file.data + file_write->internal.position;
	size_t step = maths_min(rest, file_write->size);
	size_t writen = fwrite(offset, 1, step, self);

	if ((long)writen < (long)step) {
		file_write->error = file_writing_error;
		return false;
	}

	if (file_write->consume) {
		char_vecs_shift(&file_write->file, 0, writen, null);
	} else {
		file_write->internal.position += writen;
	}

	if (writen == 0) {
		return false;
	}

	return true;
}

estring_vec files_list(const string path, const allocator *mem) {
	#if cels_debug
		errors_abort("path", strings_check_extra(&path));
	#endif

	dir *directory = opendir(path.data);
	if (!directory) {
		return (estring_vec){.error=file_directory_not_opened_error};
	}

	string_vec files = string_vecs_init(vector_min, mem);
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

		bool push_error = string_vecs_force(&files, file, mem);
		if (push_error) {
			return (estring_vec){.error=file_allocation_error};
		}
	}

	return (estring_vec){.value=files};
}

estring_vec files_list_all(const string path, const allocator *mem) {
	error err = ok;
	string_vec shallow_files = string_vecs_init(vector_min, mem);

	estring_vec files = files_list(path, mem); 
	if (files.error != file_successfull) {
		err = files.error;
		goto cleanup0;
	}

	for (size_t i = 0; i < files.value.size; i++) {
		dir *directory = opendir(files.value.data[i].data);
		if (!directory) { 
			string file = strings_clone(&files.value.data[i], mem);
			error push_error = string_vecs_press(&shallow_files, file, mem);
			if (push_error) {
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

				error push_error = string_vecs_press(&shallow_files, file, mem);
				if (push_error) {
					err = file_allocation_error;
					goto cleanup1;
				}
			} else if (entity->d_type == DT_DIR) {
				string file_capsule = strings_encapsulate(entity->d_name);
				estring_vec files = files_list_all(file_capsule, mem);
				if (files.error != file_successfull) {
					continue;
				}

				error unite_error = string_vecs_unite(&shallow_files, &files.value, mem);
				if (unite_error) {
					err = file_allocation_error;
					goto cleanup1;
				}
			} else {
				continue;
			}
		}
	}

	string_vecs_free(&files.value, mem);
	return (estring_vec){.value=shallow_files};

	cleanup1:
	string_vecs_free(&files.value, mem);

	cleanup0:
	string_vecs_free(&shallow_files, mem);
	return (estring_vec){.error=err};
}

ssize_t files_find(file *self, const string substring, ssize_t pos) {
	#if cels_debug
		errors_abort("self", !self);
		errors_abort("substring", strings_check_extra(&substring));
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

ssize_t files_find_from(file *self, const string seps, ssize_t pos) {
	#if cels_debug
		errors_abort("self", !self);
		errors_abort("seps", strings_check_extra(&seps));
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

bool files_next(file *self, string *line_view, const allocator *mem) {
	#if cels_debug
		errors_abort("self", !self);
	#endif

	long current_position = ftell(self);
	if (current_position == -1) {
		return file_telling_position_error;
	}

	if (!line_view->data) {
		string buffer = strings_init(string_small_size, mem);
		*line_view = buffer;

		if (current_position != 0) {
			long pos = fseek(self, 0, SEEK_SET);
			if (pos == -1) { return file_seeking_position_error; }
		}
	}

	#if cels_debug
		errors_abort("line_view", strings_check_view(line_view));
	#endif

	const string line_separator = {
		.size=3,
		.capacity=3,
		.data=(char[3]){'\n', EOF, '\0'}
	};

	ssize_t next_position = files_find_from(self, line_separator, current_position + 1);
	if (next_position == -1) { return true; }

	size_t capacity = next_position + 1 - current_position;
	while (capacity > line_view->capacity) {
		error upscale_error = char_vecs_upscale(line_view, mem);
		if (upscale_error) { return file_allocation_error; }
	}

	long seek_error = fseek(self, current_position, SEEK_SET);
	if (seek_error == -1) { return file_seeking_position_error; }

	size_t bytes_read = fread(line_view->data, 1, capacity, self);
	if (bytes_read == 0) { return file_reading_error; }

	line_view->size = bytes_read;
	line_view->data[line_view->size - 1] = '\0';

	if (ferror(self)) { return file_other_error; }

	return false;
}

estring files_normalize(const string *filepath, const allocator *mem) {
	#if cels_debug
		errors_abort("filepath", strings_check_extra(filepath));
	#endif

	static const string file_sep = strings_premake("/");
	static const string one_dots = strings_premake(".");
	static const string two_dots = strings_premake("..");

	string path_normalized = strings_init(vector_min, mem);
	string_vec file_nodes = strings_split(filepath, file_sep, 0, mem);

	size_t i = 0;
	while(true) {
		if (strings_seems(&file_nodes.data[i], &two_dots)) {
			if (i == 0) { goto cleanup; }

			string_vecs_shift(&file_nodes, i - 1, 2, mem);
			i--;
		} else if (strings_seems(&file_nodes.data[i], &one_dots)) {
			string_vecs_shift(&file_nodes, i, 1, mem);
		} else {
			i++;
		}

		if (i == file_nodes.size - 1) {
			break;
		}
	}

	for (size_t i = 0; i < file_nodes.size; i++) {
		error push_error = strings_push(&path_normalized, file_sep, mem);
		if (push_error) { goto cleanup; }

		push_error = strings_push(&path_normalized, file_nodes.data[i], mem);
		if (push_error) { goto cleanup; }
	}

	string_vecs_free(&file_nodes, mem);
	return (estring){.value=path_normalized};

	cleanup:
	strings_free(&path_normalized, mem);
	string_vecs_free(&file_nodes, mem);
	return (estring){.error=file_mal_formed_error};
}

estring files_path(const string *filepath, const allocator *mem) {
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
	getcwd(working_directory.data, working_directory.capacity);
	working_directory.size = strlen(working_directory.data) + 1;
	working_directory.data[working_directory.size - 1] = '\0';

	string path = strings_format(
		"%s/%s", mem, working_directory.data, filepath->data);

	estring path_normalized = files_normalize(&path, mem);
	strings_free(&path, mem);

	return path_normalized;
}

error files_make_directory(const char *path, notused __mode_t mode) {
	#if cels_debug
		errors_abort("path", strs_check(path));
	#endif

	error error = ok;

	#ifdef cels_windows
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
