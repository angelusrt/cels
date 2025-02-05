#include "jsons.h"
#include "strings.h"

#define case_whitespace \
	case ' ': \
	case '\r': \
	case '\n': \
	case '\t'

#define case_number \
	case '-': \
	case '0': \
	case '1': \
	case '2': \
	case '3': \
	case '4': \
	case '5': \
	case '6': \
	case '7': \
	case '8': \
	case '9' 

string jsons_parse_text_private(const string *json, size_t *position, const allocator *mem) {
	#if cels_debug
		errors_abort("json", strings_check_extra(json));
		errors_abort("position", !position);
	#endif

	size_t pos = *position;
	size_t start = 0;
	size_t end = 0;

	for (size_t i = pos; i < json->size - 1; i++) {
		if (json->data[i] == '"') {
			if (start == 0) {
				start = i + 1;
			} else {
				bool was_escaped = json->data[i - 1] == '\\';
				if (was_escaped) {
					continue;
				}

				end = i;
				break;
			}
		}
	}

	if ((start > end) || (start == end)) {
		goto error;
	}

	string text = strings_copy(json, start, end, mem);
	strings_normalize(&text);

	*position += (end - start) + 1;
	return text;

	error:
	return (string){0};
}

string jsons_parse_struct_private(
	const string *json, size_t *position, bool is_object, const allocator *mem
) {
	#if cels_debug
		errors_abort("json", strings_check_extra(json));
		errors_abort("position", !position);
	#endif

	size_t pos = *position;
	size_t start = 0;
	size_t end = 0;

	char opening = is_object ? '{' : '[';
	char closing = is_object ? '}' : ']';

	bool is_inside_quotes = false;
	bool has_startet = false;
	ssize_t count = 0;
	for (size_t i = pos; i < json->size - 1; i++) {
		if (json->data[i] == opening && !is_inside_quotes) {
			if (!has_startet) { start = i; }

			count++;
			has_startet = true;
		} 

		if (has_startet) {
			if (json->data[i] == '"') {
				if (is_inside_quotes) {
					if (json->data[i - 1] == '\\') {
						continue;
					}

					is_inside_quotes = false;
				} else {
					is_inside_quotes = true;
				}
				continue;
			}

			if (!is_inside_quotes) {
				if (json->data[i] == closing) {
					count--;
				} 

				if (count == 0) {
					end = i + 1;
					break;
				}
			}
		}
	}

	if ((start > end) || (start == end)) {
		goto error;
	}

	string text = strings_copy(json, start, end, mem);
	*position += (end - 1) - start;
	return text;

	error:
	return (string){0};
}

string jsons_parse_naked_private(const string *json, size_t *position, const allocator *mem) {
	#if cels_debug
		errors_abort("json", strings_check_extra(json));
		errors_abort("position", !position);
	#endif

	size_t start = 0;
	size_t end = 0;
	size_t pos = *position;

	switch (json->data[pos]) {
	case 't':
		if (json->size - 2 < pos + 3) {
			goto error;
		}

		bool is_valid_true = 
			json->data[pos + 1] == 'r' && 
			json->data[pos + 2] == 'u' &&
			json->data[pos + 3] == 'e';

		if (is_valid_true) {
			start = pos;
			end = pos + 4;
		} else {
			goto error;
		}
	break;
	case 'f':
		if (json->size - 2 < pos + 4) {
			goto error;
		}

		bool is_valid_false = 
			json->data[pos + 1] == 'a' && 
			json->data[pos + 2] == 'l' &&
			json->data[pos + 3] == 's' &&
			json->data[pos + 4] == 'e';

		if (is_valid_false) {
			start = pos;
			end = pos + 5;
		} else {
			goto error;
		}
	break;
	case 'n':
		if (json->size - 2 < pos + 3) {
			goto error;
		}

		bool is_valid_null = 
			json->data[pos + 1] == 'u' && 
			json->data[pos + 2] == 'l' &&
			json->data[pos + 3] == 'l';

		if (is_valid_null) {
			start = pos;
			end = pos + 4;
		} else {
			goto error;
		}
	break;
	case_number:
		for(size_t i = pos; i < json->size - 1; i++) {
			bool is_end = false;

			switch(json->data[i]) {
			case_whitespace:
			case ',':
			case ']':
			case '}':
				start = pos;
				end = i;
				is_end = true;
				break;
			}

			if (is_end) {
				break;
			}
		}
		//TODO: check numbers
	}

	string text = strings_copy(json, start, end, mem);
	strings_normalize(&text);

	*position += (end - 1) - start;
	return text;

	error:
	return (string){0};
}

estring_map jsons_unmake_object_private(const string *json, const allocator *mem) {
	#if cels_debug
		errors_abort("json", strings_check_extra(json));
	#endif

	int error = 0;
	string_map map = string_maps_init();

	bool is_key_valid = true;
	string key = {0};
	bool is_value_valid = false;
	string value = {0};

	for (size_t position = 1; position < json->size - 2; position++) {
		switch (json->data[position]) {
		case_whitespace:
			continue;
		case ':':
			if (!is_value_valid && is_key_valid && key.size != 0) {
				is_value_valid = true;
			} else {
				error = json_misplaced_colon_error;
				goto error;
			}

			continue;
		case ',':
			if (!is_key_valid) {
				is_key_valid = true;
			} else {
				error = json_misplaced_comma_error;
				goto error;
			}

			continue;
		case '"':
			if (is_key_valid && key.size == 0 && !is_value_valid) {
				key = jsons_parse_text_private(json, &position, mem);
				if (key.size == 0) {
					error = json_key_size_error;
					goto error;
				}
			} else if (is_key_valid && key.size != 0 && !is_value_valid) {
				error = json_missing_colon_error;
				goto error;
			} else if (is_key_valid && key.size != 0 && is_value_valid && value.size == 0) {
				value = jsons_parse_text_private(json, &position, mem);
				if (value.size == 0) {
					error = json_value_size_error;
					goto error;
				}
			} else if (is_key_valid && key.size != 0 && is_value_valid && value.size != 0) {
				error = json_missing_comma_error;
				goto error;
			} else {
				error = json_impossible_state_error;
				goto error;
			}

			break;
		case '{':
		case '[': ; //empty statement
			bool is_object = json->data[position] == '{';

			if (!is_key_valid || key.size == 0 || !is_value_valid || value.size != 0) {
				error = json_missing_colon_error;
				goto error;
			}

			value = jsons_parse_struct_private(json, &position, is_object, mem);
			if (value.size == 0) {
				error = json_value_size_error;
				goto error;
			}

			break;
		default:
			if (!is_key_valid || key.size == 0 || !is_value_valid || value.size != 0) {
				error = json_missing_colon_error;
				goto error;
			}

			value = jsons_parse_naked_private(json, &position, mem);
			if (value.size == 0) {
				error = json_invalid_naked_value_error;
				goto error;
			}

			break;
		}

		if (key.data && value.data) {
			bool push_error = string_maps_push(&map, key, value, mem);
			if (push_error) {
				error = json_invalid_state_error;
				goto error;
			}

			strings_erase(&key);
			strings_erase(&value);
			is_key_valid = false;
			is_value_valid = false;
		}
	}

	return (estring_map) {.value=map};

	error:
	if (map.data) {
		string_maps_free(&map, mem);
	}

	if (key.size != 0) {
		strings_free(&key, mem);
	}

	if (value.size != 0) {
		strings_free(&value, mem);
	}

	return (estring_map) {.error=error};
}

estring_map jsons_unmake_list_private(const string *json, const allocator *mem) {
	#if cels_debug
		errors_abort("json", strings_check_extra(json));
	#endif

	int error = 0;
	string_map map = string_maps_init();

	size_t count = 0;
	bool is_value_valid = true;
	string value = {0};

	for (size_t position = 1; position < json->size - 2; position++) {
		switch (json->data[position]) {
		case_whitespace:
			continue;
		case ',':
			if (!is_value_valid) {
				is_value_valid = true;
			} else {
				error = json_misplaced_comma_error;
				goto error;
			}

			continue;
		case '"':
			if (is_value_valid && value.size == 0) {
				value = jsons_parse_text_private(json, &position, mem);
				if (value.size == 0) {
					error = json_value_size_error;
					goto error;
				}
			} else if (!is_value_valid || value.size != 0) {
				error = json_missing_comma_error;
				goto error;
			} else {
				error = json_impossible_state_error;
				goto error;
			}

			break;
		case '{':
		case '[': ; //empty statement
			bool is_object = json->data[position] == '{';

			if (!is_value_valid || value.size != 0) {
				error = json_missing_comma_error;
				goto error;
			}

			value = jsons_parse_struct_private(json, &position, is_object, mem);
			if (value.size == 0) {
				error = json_value_size_error;
				goto error;
			}

			break;
		default:
			if (!is_value_valid || value.size != 0) {
				error = json_missing_comma_error;
				goto error;
			}

			value = jsons_parse_naked_private(json, &position, mem);
			if (value.size == 0) {
				error = json_invalid_naked_value_error;
				goto error;
			}

			break;
		}

		if (value.size != 0) {
			string key = strings_format("%zu", mem, count);
			bool push_error = string_maps_push(&map, key, value, mem);
			if (push_error) {
				error = json_invalid_state_error;
				goto error;
			}

			strings_empty(&value);
			is_value_valid = false;
			count++;
		}
	}

	return (estring_map) {.value=map};

	error:
	if (map.data) {
		string_maps_free(&map, mem);
	}

	if (value.size != 0) {
		strings_free(&value, mem);
	}

	return (estring_map) {.error=error};
}

estring_map jsons_unmake(const string *json, const allocator *mem) {
	#if cels_debug
		errors_abort("json", strings_check_extra(json));
	#endif

	//json should be trimmed
	bool is_object = json->data[0] == '{' && json->data[json->size - 2] == '}';
	if (is_object) {
		return jsons_unmake_object_private(json, mem);
	}

	bool is_list = json->data[0] == '[' && json->data[json->size - 2] == ']';
	if (is_list) {
		return jsons_unmake_list_private(json, mem);
	}

	return (estring_map){.error=json_invalid_error};
}

typedef struct jsons_make_private_params {
	string *json;
	const allocator *mem;
	bool *error;
} jsons_make_private_params;

void *jsons_make_private(const string_map_bynary_node *self, jsons_make_private_params *params) {
	#if cels_debug
		errors_abort("self", bynary_nodes_check((const bynary_node *)self));
		errors_abort("params", !params);
		errors_abort("params.json", strings_check_extra(params->json));
		errors_abort("params.error", !params->error);
	#endif

	bool push_error = strings_push_with(params->json, "\"", params->mem);
	if (push_error) { goto cleanup; }

	push_error = strings_push(params->json, self->data.key, params->mem);
	if (push_error) { goto cleanup; }

	push_error = strings_push_with(params->json, "\"", params->mem);
	if (push_error) { goto cleanup; }

	push_error = strings_push_with(params->json, ":", params->mem);
	if (push_error) { goto cleanup; }

	char value_start_char = self->data.value.data[0];
	bool is_value_text = value_start_char != '[' && value_start_char != '{';

	if (is_value_text) {
		push_error = strings_push_with(params->json, "\"", params->mem);
		if (push_error) { goto cleanup; }

		push_error = strings_push(params->json, self->data.value, params->mem);
		if (push_error) { goto cleanup; }

		push_error = strings_push_with(params->json, "\"", params->mem);
		if (push_error) { goto cleanup; }
	} else {
		push_error = strings_push(params->json, self->data.value, params->mem);
		if (push_error) { goto cleanup; }
	}

	push_error = strings_push_with(params->json, ",", params->mem);
	if (push_error) { goto cleanup; }

	return null;

	cleanup:
	*params->error = true;
	return null;
}

estring jsons_make(const string_map *self, const allocator *mem) {
	#if cels_debug
		errors_abort("self", bynary_nodes_check((const void *)self));
	#endif

	string json = strings_make("{", mem);

	bool private_error = false;
	jsons_make_private_params params = {
		.error=&private_error, .json=&json, .mem=mem};

	bynary_node_iterator it = {0};
	while (bynary_nodes_next((bynary_node *)self->data, &it)) {
		jsons_make_private((string_map_bynary_node *)it.data, &params);
	}

	if (private_error) { goto cleanup; }

	bool pop_error = strings_pop(&json, mem);
	if (pop_error) { goto cleanup; }

	bool push_error = strings_push_with(&json, "}", mem);
	if (push_error) { goto cleanup; }

	return (estring){.value=json};

	cleanup:
	strings_free(&json, mem);
	return (estring){.error=json_invalid_error};
}
