#include "systems.h"

error systems_load(const string path, const allocator *mem) {
	#if cels_debug
		errors_abort("path", strings_check_extra(&path));
	#endif

	file *env = fopen(path.data, "r");
	if (!env) { return fail; }

	error error = system_successfull;
	string separator = strings_premake("=");
	string line_view = {0};

	while (!files_next(env, &line_view, mem)) {
		size_t pos = strings_find(&line_view, separator, 0);
		if (pos == -1) { continue; }

		if (pos < 1) { 
			error = system_env_file_mal_formed_error; 
			goto cleanup;
		}

		if (line_view.size - pos < 1) { 
			error = system_env_file_mal_formed_error; 
			goto cleanup;
		}

		line_view.data[pos] = '\0';
		line_view.data[line_view.size - 1] = '\0';

		char *key_start = line_view.data;
		char *value_start = line_view.data + pos;
		bool is_between_quotes = 
			line_view.data[pos + 1] == '"' && 
			line_view.data[line_view.size - 2] == '"';

		if (is_between_quotes) {
			value_start++;
			*value_start = '\0';
			value_start++;
			line_view.data[line_view.size - 2] = '\0';
		}

		setenv(key_start, value_start, 0);
	}

	cleanup:
	fclose(env);
	strings_free(&line_view, mem);
	return error;
}
