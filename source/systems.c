#include "systems.h"
#include "strings.h"


/* systems */

error systems_load(const string path, const allocator *mem) {
	#if cels_debug
		errors_abort("path", strings_check_extra(&path));
	#endif

	file *env = fopen(path.data, "r");
	if (!env) { return fail; }

	error error = system_successfull;
	string separator = strings_premake("=");
	byte_vec line = {0};

	while (!files_next(env, &line, mem)) {
		bool is_string = byte_vecs_is_string(&line);
		if (!is_string) {
			error = system_env_file_mal_formed_error; 
			goto cleanup0;
		}

		string *l = (string *)&line;
		size_t pos = strings_find(l, separator, 0);
		if (pos == -1) { continue; }

		if (pos < 1) { 
			error = system_env_file_mal_formed_error; 
			goto cleanup0;
		}

		if (l->size - pos < 1) { 
			error = system_env_file_mal_formed_error; 
			goto cleanup0;
		}

		l->data[pos] = '\0';
		l->data[l->size - 1] = '\0';

		char *key_start = l->data;
		char *value_start = l->data + pos;
		bool is_between_quotes = 
			l->data[pos + 1] == '"' && 
			l->data[l->size - 2] == '"';

		if (is_between_quotes) {
			value_start++;
			*value_start = '\0';
			value_start++;
			l->data[l->size - 2] = '\0';
		}

		setenv(key_start, value_start, 0);
	}

	cleanup0:
	fclose(env);
	vectors_free(&line, null, mem);
	return error;
}
