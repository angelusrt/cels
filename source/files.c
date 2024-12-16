#include "files.h"

string_with_error files_read(file *f) {
	long current_pos = ftell(f);
	if (current_pos == -1) return (string_with_error){.error=-1};

	int seek_error = fseek(f, 0, SEEK_END);
	if (seek_error == -1) return (string_with_error){.error=-2};

	long last_pos = ftell(f);
	if (last_pos == -1) return (string_with_error){.error=-1};

	seek_error = fseek(f, 0, SEEK_SET);
	if (seek_error == -1) return (string_with_error){.error=-2};

	size_t new_capacity = maths_nearest_two_power(last_pos - current_pos);
	string buffer = strings_init(new_capacity, null);

	long bytes_read = fread(buffer.data, 1, buffer.capacity, f);
	buffer.size = bytes_read + 1;

	bool did_file_not_end = feof(f) == 0;
	int file_error = ferror(f);
	long error = did_file_not_end+file_error;

	if (error != 0) {
		strings_free(&buffer, null);
		return (string_with_error){.error=error};
	}

	return (string_with_error){.value=buffer};
}
