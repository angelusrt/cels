#include "jsons.h"
#include "strings.h"

#define case_number \
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

/* tokens */

typedef enum json_token {
	json_invalid_token = -1,
	json_continue_token = 0,
	json_brackets_start_token = '{',
	json_brackets_end_token = '}',
	json_braces_start_token = '[',
	json_braces_end_token = ']',
	json_quotes_start_token = '"',
	json_quotes_end_token = '\'',
	json_numbers_start_token = '0',
	json_numbers_end_token = '9',
	json_comma_token = ',',
	json_colon_token = ':',
	json_number_token = 'n',
	json_token_token = 't',
	json_list_token = 'l',
	json_object_token = 'o',
	json_item_token = 'i',
} json_token;

/* checks */

bool jsons_check_if_commence_private(json_token token) {
	bool is_terminal = 
		token == json_quotes_start_token || 
		token == json_brackets_start_token || 
		token == json_numbers_start_token || 
		token == json_braces_start_token;

	return is_terminal;
}

bool jsons_check_if_intermediary_private(json_token token) {
	bool is_terminal = 
		token == json_comma_token || 
		token == json_colon_token;

	return is_terminal;
}

bool jsons_check_if_terminal_private(json_token token) {
	bool is_terminal = 
		token == json_number_token || 
		token == json_token_token || 
		token == json_list_token || 
		token == json_object_token ||
		token == json_item_token;

	return is_terminal;
}

bool jsons_check_if_may_in_list_private(json_token token) {
	bool is_terminal = 
		token == json_number_token || 
		token == json_token_token || 
		token == json_list_token || 
		token == json_object_token;

	return is_terminal;
}

/* private */

__attribute_warn_unused_result__
ssize_t jsons_find_last_token_private(const string_vec *token_stack, json_token token) {
	//TODO: checks

	for (ssize_t i = token_stack->size; i >= 0; i--) {
		if (token_stack->data[i].data[0] == token) {
			return i;
		}
	}

	return -1;
}

__attribute_warn_unused_result__
json_token jsons_tokenize_private(const string *json, size_t position, const string_vec *token_stack) {
	//TODO: checks
	
	char current_character = json->data[position];

	char last_character = '\0';
	if (token_stack->size > 0) {
		last_character = token_stack->data[token_stack->size - 1].data[0];
	}

	char next_character = '\0';
	if (position + 1 < json->size - 1) {
		next_character = json->data[position + 1];
	}

	bool was_skipped = false;
	if (position > 0) {
		was_skipped = json->data[position - 1] == '\\';
	}

	bool is_within_quotes = last_character == json_quotes_start_token;
	bool is_position_invalid = position == 0 || position == json->size - 1;

	//
	
	switch (current_character) {
	case '{': 
		if (is_within_quotes) {
			return json_continue_token;
		}

		return json_brackets_start_token;
	break;
	case '}': 
		if (is_within_quotes) {
			return json_continue_token;
		}

		return json_brackets_end_token;
	break;
	case '[': 
		if (is_within_quotes) {
			return json_continue_token;
		}

		return json_braces_start_token;
	break;
	case ']': 
		if (is_within_quotes) {
			return json_continue_token;
		}

		return json_braces_end_token;
	break;
	case '"': 
		if (is_position_invalid) {
			return json_invalid_token;
		}

		bool was_started = last_character == json_quotes_start_token;
		if (was_started) {
			if (was_skipped) {
				return json_continue_token;
			}

			return json_quotes_end_token;
		} else {
			return json_quotes_start_token;
		}
	break;
	case ':':
		if (is_position_invalid) {
			return json_invalid_token;
		}

		if (is_within_quotes) {
			return json_continue_token;
		}

		return json_colon_token;
	case ',':
		if (is_position_invalid) {
			return json_invalid_token;
		}

		if (is_within_quotes) {
			return json_continue_token;
		}

		return json_comma_token;
	case_number:
		if (is_within_quotes) {
			return json_continue_token;
		}

		bool is_next_number = next_character >= '0' && next_character <= '9';
		bool was_number_started = last_character == json_numbers_start_token;

		if (was_number_started && is_next_number) {
			return json_continue_token;
		} else if (was_number_started && !is_next_number) {
			return json_numbers_end_token;
		} else if (!was_number_started && is_next_number) {
			return json_numbers_start_token;
		} else {
			return json_number_token;
		}
	break;
	case ' ':
		return json_continue_token;
	break;
	default:
		if (is_within_quotes) {
			return json_continue_token;
		}

		return json_invalid_token;
	}

	return json_continue_token;
}

typedef enum fuse_state {
	fuse_invalid_state = -1,
	fuse_valid_state,
	fuse_nothing_state,
	fuse_invalid_json_state,
} fuse_state;

fuse_state jsons_fuse_private(string_vec *token_stack, const allocator *mem) {
	//TODO: checks

	if (token_stack->size < 2) {
		return fuse_nothing_state;
	}

	string previous = token_stack->data[token_stack->size - 2];
	char previous_token = previous.data[0];
	string current = token_stack->data[token_stack->size - 1];
	char current_token = current.data[0];
	string new_token = {0};

	if (jsons_check_if_commence_private(current_token)) {
		return fuse_nothing_state;
	}

	if (jsons_check_if_intermediary_private(current_token)) {
		return fuse_nothing_state;
	}

	if (current_token == json_quotes_end_token) {
		bool is_start = previous.data[0] == json_quotes_start_token;
		errors_panic("jsons_fuse_private.previous_token != quotes_start", !is_start);

		new_token = strings_make_format("t%s,%s", mem, previous.data + 1, current.data + 1);
	} else if (current_token == json_numbers_end_token) {
		bool is_start = previous.data[0] == json_numbers_start_token;
		errors_panic("jsons_fuse_private.previous_token != numbers_start", !is_start);

		size_t start_pos = atoi(previous.data + 1);
		size_t end_pos = atoi(current.data + 1);

		new_token = strings_make_format("n{%zu,%zu}", mem, start_pos - 1, end_pos + 1);
	}

	if (new_token.data) {
		strings_free(&previous, mem);
		strings_free(&current, mem);
		token_stack->size -= 2;
		goto push_state;
	}

	//

	if (token_stack->size < 3) {
		return fuse_nothing_state;
	}

	string third = token_stack->data[token_stack->size - 3];
	char third_token = third.data[0];

	if (previous_token == json_colon_token) {
		if (third_token != json_token_token) {
			return fuse_invalid_json_state;
		}

		if (!jsons_check_if_terminal_private(current_token)) {
			return fuse_invalid_json_state;
		}

		if (current_token == json_token_token) {
			new_token = strings_make_format(
				"i{%s,%s}", mem, third.data + 1, current.data + 1);
		} else {
			new_token = strings_make_format(
				"i{%s,%s}", mem, third.data + 1, current.data);
		} 

		if (new_token.data) {
			strings_free(&third, mem);
			strings_free(&previous, mem);
			strings_free(&current, mem);
			token_stack->size -= 3;
			goto push_state;
		}
	}

	//

	char closing_token = json_brackets_start_token;
	char type_token = json_object_token;

	if (current_token == json_braces_end_token) {
		closing_token = json_braces_start_token;
		type_token = json_list_token;
	} else if (current_token == json_brackets_end_token) {
		closing_token = json_brackets_start_token;
		type_token = json_object_token;
	} else {
		goto normal_state;
	}

	ssize_t pos = jsons_find_last_token_private(token_stack, closing_token);
	if (pos == -1) { return fuse_invalid_state; }

	size_t new_capacity = 0;
	const size_t range = token_stack->size - pos;
	if (range == 0) { return fuse_invalid_state; }

	if (current_token == json_brackets_end_token) {
		for (size_t i = 1; i < range - 1; i++) {
			string current = token_stack->data[pos+i];
			char current_token = current.data[0];

			bool is_odd = i % 2 == 0;
			if (is_odd && current_token != json_comma_token) {
				return fuse_invalid_json_state;
			} else if (!is_odd && current_token != json_item_token) {
				return fuse_invalid_json_state;
			} else if (!is_odd) {
				new_capacity += current.size - 1;
			} else {
				new_capacity++;
			}
		}
	} else if (current_token == json_braces_end_token) {
		char list_type = json_token_token;

		for (size_t i = 1; i < range - 1; i++) {
			string current = token_stack->data[pos+i];
			char current_token = current.data[0];

			if (i == 1) {
				list_type = current_token;
			}

			bool is_valid = jsons_check_if_may_in_list_private(current_token);
			bool is_same_type = list_type == current_token;
			bool is_odd = i % 2 == 0;

			if (is_odd && current_token != json_comma_token) {
				return fuse_invalid_json_state;
			} else if (!is_odd && (!is_valid || !is_same_type)) {
				return fuse_invalid_json_state;
			} else if (!is_odd) {
				new_capacity += current.size - 1;
			} else {
				new_capacity++;
			}
		}
	}

	string start = token_stack->data[pos];
	if (range == 1) {
		new_token = strings_make_format(
			"%c{%s,%s,{}}", mem, type_token, start.data + 1, current.data + 1);
		goto push_state;
	} 

	new_capacity += 5 + (previous.size - 2) + (current.size - 2);
	new_token = strings_init(new_capacity + 1, mem);

	string initials = strings_make_format(
		"%c{%s,%s,{", mem, type_token, start.data + 1, current.data + 1);
	bool push_error = strings_push(&new_token, initials, mem);

	strings_free(&initials, mem);
	if (push_error) {
		goto string_push_error_state;
	}

	for (size_t i = 1; i < range - 1; i++) {
		string comma = strings_premake(",");
		string current = token_stack->data[pos+i];

		bool push_error = false;
		bool is_odd = i % 2 == 0;
		if (is_odd) {
			push_error = strings_push(&new_token, comma, mem);
		} else {
			push_error = strings_push(&new_token, current, mem);
		}

		if (push_error) {
			goto string_push_error_state;
		}

		strings_free(&current, mem);
	}

	string endings = strings_premake("}}");
	push_error = strings_push(&new_token, endings, mem);
	if (push_error) {
		goto string_push_error_state;
	}

	token_stack->size -= range;
	goto push_state;


	normal_state:
	return fuse_nothing_state;

	//

	push_state:
	push_error = string_vecs_fpush(token_stack, new_token, mem);
	return push_error ? fuse_invalid_state : fuse_valid_state;

	string_push_error_state:
	strings_free(&new_token, mem);
	string_vecs_free(token_stack, mem);
	return fuse_invalid_state;
}

estring jsons_view_inside_private(const string *json, size_t pos) {
	//TODO: checks
	
	string object_open = strings_premake("{");
	string object_close = strings_premake("}");

	ssize_t open_pos = strings_find(json, &object_open, pos);
	if (open_pos == -1) {
		return (estring) {.error=json_invalid_error};
	}

	ssize_t close_pos = strings_find_closing_tag(json, object_open, object_close, open_pos);
	if (close_pos == -1) {
		return (estring) {.error=json_invalid_error};
	}

	string view = {
		.data=json->data + open_pos + 1,
		.size=(close_pos-open_pos),
		.capacity=(close_pos-open_pos) + 1,
	};

	return (estring){.value=view};
}

/* public */

estring jsons_describe(const string *json, const allocator *mem) {
	//TODO: checks

	string_vec token_stack = string_vecs_init(vectors_min, mem);

	for (size_t i = 0; i < json->size - 1; i++) {
		json_token token = jsons_tokenize_private(json, i, &token_stack);
		string token_formated = {0};

		if (token == json_invalid_token) {
			string_vecs_free(&token_stack, mem);
			return (estring){.error=json_invalid_error};
		} else if (token == json_continue_token) {
			continue;
		} else if (token == json_number_token) {
			token_formated = strings_make_format("%c{%zu,%zu}", mem, token, i - 1, i + 1);
		} else {
			token_formated = strings_make_format("%c%zu", mem, token, i);
		}

		bool push_error = string_vecs_fpush(&token_stack, token_formated, mem);
		if (push_error) {
			return (estring){.error=json_invalid_state_error};
		}

		while (true) {
			fuse_state fuse_status = jsons_fuse_private(&token_stack, mem);

			if (fuse_status == fuse_invalid_state) {
				string_vecs_free(&token_stack, mem);
				return (estring){.error=json_invalid_state_error};
			} else if (fuse_status == fuse_invalid_json_state) {
				string_vecs_free(&token_stack, mem);
				return (estring){.error=json_invalid_error};
			} else if (fuse_status == fuse_nothing_state) {
				break;
			}
		}
	}

	string description = token_stack.data[0];
	mems_dealloc(mem, token_stack.data, token_stack.capacity);

	return (estring){.value=description};
}

estring jsons_resume(const string *description, const allocator *mem) {
	//TODO: checks

	estring description_view = jsons_view_inside_private(description, 3);
	if (description_view.error != json_successfull) {
		return (estring){.error=json_invalid_error};
	} 

	string desc = strings_make_copy(&description_view.value, mem);
	size_t i = 0;

	while(true) {
		string object_open = strings_premake("{");
		string object_close = strings_premake("}");

		ssize_t open_pos = strings_find(&desc, &object_open, i);
		if (open_pos == -1) {
			strings_free(&desc, mem);
			return (estring) {.error=json_invalid_error};
		}

		ssize_t close_pos = strings_find_closing_tag(&desc, object_open, object_close, open_pos);
		if (close_pos == -1) {
			strings_free(&desc, mem);
			return (estring) {.error=json_invalid_error};
		}

		if (desc.data[close_pos + 1] == ',') {
			desc.data[close_pos + 1] = ';';
		}

		i = close_pos + 1;

		if (i >= desc.size - 1) {
			break;
		}
	} 

	string chars_to_remove = strings_premake("ionl{}");
	strings_replace(&desc, &chars_to_remove, -1, 0);

	return (estring){.value=desc};
}

estring_map jsons_unmake(const string *json, const allocator *mem) {
	//TODO: checks

	estring json_description = jsons_describe(json, mem);
	if (json_description.error != json_successfull) {
		return (estring_map){.error=json_invalid_error};
	} 

	//

	estring json_resume = jsons_resume(&json_description.value, mem);
	if (json_resume.error != json_successfull) {
		return (estring_map){.error=json_resume.error};
	} 

	//

	string_map *json_map = null;
	const string item_separator = strings_premake(";");
	const string pos_separator = strings_premake(",");
	string_vec items = strings_make_split(&json_resume.value, &item_separator, 0, mem);
	//check

	for (size_t i = 0; i < items.size; i++) {
		string_vec positions = strings_make_split(&items.data[i], &pos_separator, 4, mem);
		if (positions.size < 4) {
			return (estring_map){.error=json_invalid_error};
		}

		//

		size_t key_init = atoi(positions.data[0].data) + 1;
		size_t key_end = atoi(positions.data[1].data);
		size_t value_init = atoi(positions.data[2].data);
		size_t value_end = atoi(positions.data[3].data) + 1;

		if (positions.size == 4) {
			value_init++;
			value_end--;
		}

		//

		string key_view = {
			.data=json->data+key_init,
			.size=key_end-key_init,
			.capacity=(key_end-key_init)+1,
		};

		string key = strings_make_copy(&key_view, mem);
		key.size++;
		key.data[key.capacity] = '\0';

		string value_view = {
			.data=json->data+value_init,
			.size=value_end-value_init,
			.capacity=(value_end-value_init)+1,
		};

		string value = strings_make_copy(&value_view, mem);
		value.size++;
		value.data[value.capacity] = '\0';

		//

		bool push_error = string_maps_push(&json_map, key, value, mem);
		if (push_error) {
			string_maps_free(json_map, mem);
			return (estring_map){.error=json_invalid_state_error};
		}
	}

	//

	return (estring_map){.value=json_map};
}

typedef struct jsons_make_private_params {
	string *json;
	const allocator *mem;
	bool *error;
} jsons_make_private_params;

void *jsons_make_private(string_map *self, void *args) {
	jsons_make_private_params *params = args;

	const string quote = strings_premake("\"");
	bool push_error = strings_push(params->json, quote, params->mem);
	if (push_error) { goto cleanup; }

	push_error = strings_push(params->json, self->data.key, params->mem);
	if (push_error) { goto cleanup; }

	push_error = strings_push(params->json, quote, params->mem);
	if (push_error) { goto cleanup; }


	const string colon = strings_premake(":");
	push_error = strings_push(params->json, colon, params->mem);
	if (push_error) { goto cleanup; }

	char value_start_char = self->data.value.data[0];
	bool is_value_text = value_start_char != '[' && value_start_char != '{';

	if (is_value_text) {
		push_error = strings_push(params->json, quote, params->mem);
		if (push_error) { goto cleanup; }

		push_error = strings_push(params->json, self->data.value, params->mem);
		if (push_error) { goto cleanup; }

		push_error = strings_push(params->json, quote, params->mem);
		if (push_error) { goto cleanup; }
	} else {
		push_error = strings_push(params->json, self->data.value, params->mem);
		if (push_error) { goto cleanup; }
	}

	const string comma = strings_premake(",");
	push_error = strings_push(params->json, comma, params->mem);
	if (push_error) { goto cleanup; }

	return null;

	cleanup:
	*params->error = true;
	return null;
}

estring jsons_make(const string_map *self, const allocator *mem) {
	string json = strings_make("{", mem);

	//
	
	bool private_error = false;

	jsons_make_private_params params = {
		.error=&private_error, .json=&json, .mem=mem};

	enfunctor func = {
		.func=(selffunc)jsons_make_private, .params=&params};

	bnodes_iterate((bnode *)self, func);

	if (private_error) { goto cleanup; }

	//

	bool pop_error = strings_pop(&json, mem);
	if (pop_error) { goto cleanup; }

	const string close_bracket = strings_premake("}");
	bool push_error = strings_push(&json, close_bracket, mem);
	if (push_error) { goto cleanup; }

	return (estring){.value=json};

	cleanup:
	strings_free(&json, mem);
	return (estring){.error=json_invalid_token};
}
