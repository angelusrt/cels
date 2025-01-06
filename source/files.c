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

estring_vec files_list(const string path, const allocator *mem) {
	DIR *directory = opendir(path.data);
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
