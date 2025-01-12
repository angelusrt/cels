#include "files.h"

estring files_read(file *self, const allocator *mem) {
	long current_position = ftell(self);
	if (current_position == -1) {
		return (estring){.error=file_telling_position_error};
	}

	int seek_error = fseek(self, 0, SEEK_END);
	if (seek_error == -1) {
		return (estring){.error=file_seeking_position_error};
	}

	long last_position = ftell(self);
	if (last_position == -1) {
		return (estring){.error=file_telling_position_error};
	}

	seek_error = fseek(self, 0, SEEK_SET);
	if (seek_error == -1) {
		return (estring){.error=file_seeking_position_error};
	}

	size_t new_capacity = last_position - current_position;
	string buffer = strings_init(new_capacity, mem);

	long bytes_read = fread(buffer.data, 1, buffer.capacity, self);
	buffer.size = bytes_read;

	int error = 0;
	if (feof(self)) {
		error = file_did_not_end_error;
		goto error;
	}

	if (ferror(self)) {
		error = file_other_error;
		goto error;
	}

	return (estring){.value=buffer};

	error:
	strings_free(&buffer, mem);
	return (estring){.error=error};
}

error files_write(file *self, const string text) {
	long write_error = fwrite(text.data, 1, text.size - 1, self);
	if (write_error < (long)text.size - 1) {
		return file_writing_error;
	}

	return file_successfull;
}

estring_vec files_list(const string path, const allocator *mem) {
	dir *directory = opendir(path.data);
	if (!directory) {
		return (estring_vec){.error=file_directory_not_opened_error};
	}

	//

	string_vec files = string_vecs_init(vector_min, mem);

	//

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

		string file = strings_make(entity->d_name, mem);

		bool push_error = string_vecs_push(&files, file, mem);
		if (push_error) {
			strings_free(&file, mem);
			string_vecs_free(&files, mem);
			return (estring_vec){.error=file_allocation_error};
		}
	}

	return (estring_vec){.value=files};
}

ssize_t files_find(file *self, string seps, ssize_t pos) {
	if (pos >= 0) {
		ssize_t position = fseek(self, pos, SEEK_SET);
		if (position == -1) {
			return -1;
		}
	} else {
		ssize_t position = ftell(self);
		if (position == -1) {
			return -1;
		}
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

bool files_next(file *self, string *line, const allocator *mem) {
	//check self and line

	long current_position = ftell(self);
	if (current_position == -1) {
		return file_telling_position_error;
	}

	if (line->data == null) {
		string buffer = strings_init(string_small_size, mem);
		*line = buffer;

		if (current_position != 0) {
			long pos = fseek(self, 0, SEEK_SET);
			if (pos == -1) {
				return file_seeking_position_error;
			}
		}
	}

	const string line_separator = {
		.size=3,
		.capacity=3,
		.data=(char[3]){'\n', EOF, '\0'}
	};

	ssize_t next_position = files_find(self, line_separator, current_position + 1);
	if (next_position == -1) {
		return true;
	}

	size_t capacity = next_position + 1 - current_position;
	while (capacity > line->capacity) {
		bool upscale_error = char_vecs_upscale(line, mem);
		if (upscale_error) {
			return file_allocation_error;
		}
	}

	long seek_error = fseek(self, current_position, SEEK_SET);
	if (seek_error == -1) {
		return file_seeking_position_error;
	}

	size_t bytes_read = fread(line->data, 1, capacity, self);
	line->size = bytes_read;

	if (feof(self)) {
		return file_did_not_end_error;
	}

	if (ferror(self)) {
		return file_other_error;
	}

	return false;
}

