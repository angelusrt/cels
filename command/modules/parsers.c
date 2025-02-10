#include "parsers.h"

typedef enum parser_entity_type {
	parser_alias_entity_type,
	parser_struct_entity_type,
	parser_enum_entity_type,
	parser_union_entity_type,
	parser_function_definition_entity_type,
	parser_function_implementation_entity_type,
} parser_entity_type;

typedef enum parser_error {
	parser_successfull,
	parser_generic_error,
	parser_buffer_overflow_error,
	parser_mal_formed_entity_error
} parser_error;

typedef struct parser_entity {
	parser_entity_type type;
	size_t range[2];
	string name;
	union {
		/* alias */
		string alias;
		/* struct */
		string struct_argument;
		/* enum */
		string enum_argument;
		/* union */
		string union_argument;
		/* function_definition */
		struct {
			string function_definition_argument;
			string function_definition_return;
		};
		/* function_implementation */
		struct {
			string function_argument;
			string function_return;
			string function_body;
		};
	};
} parser_entity;

typedef errors(parser_entity) eparser_entity;

static const char letter = '.';
static const string typedef_word = strings_premake("typedef");
static const string struct_word = strings_premake("struct");
static const string enum_word = strings_premake("enum");
static const string union_word = strings_premake("union");

void parser_entitys_print(parser_entity *entity) {
	printf("<parser_entity>{");

	printf(".name=");
    strings_print(&entity->name);
	printf(",");

    switch (entity->type) {
        case parser_alias_entity_type:
            printf(".type=alias,.alias=");
            strings_imprint(&entity->alias);
            break;
        case parser_struct_entity_type:
            printf(".type=struct,.argument=");
            strings_imprint(&entity->struct_argument);
            break;
        case parser_enum_entity_type:
            printf(".type=enum,.argument=");
            strings_imprint(&entity->enum_argument);
            break;
        case parser_union_entity_type:
            printf(".type=union,.argument=");
            strings_imprint(&entity->union_argument);
            break;
        case parser_function_definition_entity_type:
            printf(".type=function_definition,.argument=");
            strings_imprint(&entity->function_definition_argument);
            printf(".return=");
            strings_imprint(&entity->function_definition_return);
            break;
        case parser_function_implementation_entity_type:
            printf(".type=function_implementation,.argument=");
            strings_imprint(&entity->function_argument);
            printf(".return=");
            strings_imprint(&entity->function_return);
            printf(".body=");
            strings_imprint(&entity->function_body);
            break;
    }

    printf(",.range={%zu, %zu}}\n", entity->range[0], entity->range[1]);
}

void parsers_normalize(string *file) {
	bool is_within_quotes = false;
	for (size_t i = 0; i < file->size - 1; i++) {
		if (i > 0 && file->data[i] == '"' && file->data[i - 1] != '\\') {
			is_within_quotes = !is_within_quotes;
		} else if (i == 0 && file->data[i] == '"') {
			is_within_quotes = !is_within_quotes;
		}

		if (is_within_quotes) {
			continue;
		}

		if (i > 0 && file->data[i - 1] == '/' && file->data[i] == '*') {
			for (size_t j = i; j < file->size - 1; j++) {
				if (j > 0 && file->data[j - 1] == '*' && file->data[j] == '/') {
					memset(file->data + (i - 1), letter, j - (i - 1) + 1);
					i = j;
					break;
				}
			}
		} else if (i > 0 && file->data[i - 1] == '/' && file->data[i] == '/') {
			for (size_t j = i; j < file->size - 1; j++) {
				if (file->data[j - 1] == '\n') {
					memset(file->data + (i - 1), letter, (j - 1) - (i - 1) + 1);
					i = j - 1;
					break;
				}
			}
		} else if (i > 0 && file->data[i - 1] == '#') {
			for (size_t j = i; j < file->size - 1; j++) {
				if (file->data[j - 1] != '\\' && file->data[j] == '\n') {
					memset(file->data + (i - 1), letter, j - (i - 1) + 1);
					i = j;
					break;
				}
			}
		} else if (i > 0 && chars_is_whitespace(file->data[i - 1])) {
			file->data[i - 1] = letter;
		}
	}
}

bool parsers_is_typedef_private(string *file, size_t cursor) {
	if (file->size > cursor + typedef_word.size) {
		for (size_t i = 0; i < typedef_word.size - 1; i++) {
			if (file->data[cursor + i] != typedef_word.data[i]) {
				return false;
			}
		}

		return true;
	}

	return false;
}

bool parsers_is_struct_private(string *file, size_t cursor) {
	if (file->size > cursor + struct_word.size) {
		for (size_t i = 0; i < struct_word.size - 1; i++) {
			if (file->data[cursor + i] != struct_word.data[i]) {
				return false;
			}
		}

		return true;
	}

	return false;
}

bool parsers_is_enum_private(string *file, size_t cursor) {
	if (file->size > cursor + enum_word.size) {
		for (size_t i = 0; i < enum_word.size - 1; i++) {
			if (file->data[cursor + i] != enum_word.data[i]) {
				return false;
			}
		}

		return true;
	}

	return false;
}

bool parsers_is_union_private(string *file, size_t cursor) {
	if (file->size > cursor + union_word.size) {
		for (size_t i = 0; i < union_word.size - 1; i++) {
			if (file->data[cursor + i] != union_word.data[i]) {
				return false;
			}
		}

		return true;
	}

	return false;
}

bool parsers_is_function_private(string *file, size_t cursor, size_t *end) {
	bool first_term_found = false;
	bool second_term_found = false;
	bool third_term_found = false;

	for (size_t i = cursor; i < file->size; i++) {
		if (file->data[i] == ';') {
			*end = i;
			break;
		}

		if (third_term_found) {
			continue;
		} else if (second_term_found) {
			if (file->data[i] == letter) {
				continue;
			} else if (file->data[i] == '(') {
				third_term_found = true;
			} else if (!chars_is_alphanumeric(file->data[i])) {
				return false;
			} else {
				continue;
			}
		} else if (first_term_found) {
			if (file->data[i] == letter) {
				second_term_found = true;
			} else if (!chars_is_alphanumeric(file->data[i])) {
				return false;
			} else {
				continue;
			}
		} else {
			if (chars_is_alphanumeric(file->data[i])) {
				first_term_found = true;
			} else {
				return false;
			}
		}
	}

	return third_term_found;
}

ssize_t parsers_find_next_character_private(string *file, size_t cursor) {
	for (size_t j = cursor; j < file->size - 1; j++) {
		if (file->data[j] != letter) {
			return j;
		}
	}

	return -1;
}

ssize_t parsers_find_next_space_private(string *file, size_t cursor) {
	for (size_t j = cursor; j < file->size - 1; j++) {
		if (file->data[j] == letter) {
			return j;
		}
	}

	return -1;
}

ssize_t parsers_find_next_end_private(string *file, size_t cursor) {
	for (size_t j = cursor; j < file->size - 1; j++) {
		if (file->data[j] == letter || file->data[j] == ';') {
			return j;
		}
	}

	return -1;
}

eparser_entity parsers_parse_typedef_private(string *file, size_t *cursor, const allocator *mem) {
	parser_entity entity = {0};

	size_t initial_pos = *cursor + typedef_word.size - 1;
	ssize_t end_pos = -1;

	for (size_t i = initial_pos; i < file->size; i++) {
		if (file->data[i] == ';') {
			end_pos = i;
			break;
		}
	}

	entity.range[0] = *cursor;
	entity.range[1] = end_pos;

	if (end_pos == -1) {
		return (eparser_entity){.error=parser_mal_formed_entity_error};
	}

	bool has_third_term_started = false;
	ssize_t third_term_end_pos = -1;
	ssize_t third_term_pos = -1;
	for (size_t i = end_pos; i > initial_pos; i--) {
		if (!has_third_term_started && file->data[i] != letter) {
			third_term_end_pos = i;
			has_third_term_started = true;
		} else if (has_third_term_started && file->data[i] == letter) {
			third_term_pos = i + 1;
			break;
		}
	}

	if (third_term_pos == -1 || third_term_end_pos == -1) {
		return (eparser_entity){.error=parser_mal_formed_entity_error};
	}

	ssize_t second_term_pos = parsers_find_next_character_private(file, initial_pos);
	if (second_term_pos < 0) {
		return (eparser_entity){.error=parser_mal_formed_entity_error};
	}

	if (parsers_is_struct_private(file, second_term_pos)) {
		entity.type = parser_struct_entity_type;
	} else if (parsers_is_enum_private(file, second_term_pos)) {
		entity.type = parser_enum_entity_type;
	} else if (parsers_is_union_private(file, second_term_pos)) {
		entity.type = parser_union_entity_type;
	} else {
		entity.type = parser_alias_entity_type;
	}

	ssize_t second_term_end_pos = -1;
	for (size_t i = (size_t)third_term_pos; i > (size_t)second_term_pos; i--) {
		if (file->data[i] != letter) {
			second_term_end_pos = i - 1;
			break;
		} 
	}

	if (second_term_end_pos < 0) {
		return (eparser_entity){.error=parser_mal_formed_entity_error};
	}

	if (entity.type == parser_alias_entity_type) {
		string alias = strings_copy(file, second_term_pos, second_term_end_pos, mem);
		entity.alias = alias;

		string name = strings_copy(file, third_term_pos, third_term_end_pos, mem);
		entity.name = name;

		return (eparser_entity){.value=entity};
	} 

	//other types
	
	ssize_t argument_pos = -1;
	for (size_t i = (size_t)second_term_end_pos; i > (size_t)second_term_pos; i--) {
		if (file->data[i] == '{') {
			argument_pos = i;
			break;
		} 
	}
	
	if (second_term_end_pos < 0) {
		return (eparser_entity){.error=parser_mal_formed_entity_error};
	}

	string alias = strings_copy(file, argument_pos, second_term_end_pos, mem);
	entity.alias = alias;

	string name = strings_copy(file, third_term_pos, third_term_end_pos, mem);
	entity.name = name;

	return (eparser_entity){.value=entity};


	return (eparser_entity){.error=parser_mal_formed_entity_error};
}

error parsers_get_entities_private(string *file, const allocator *mem) {
	string buffer = strings_preinit(512);

	for (size_t i = 0; i < file->size; i++) {
		size_t function_end = 0;

		if (file->data[i] == letter) {
			ssize_t pos = parsers_find_next_character_private(file, i);
			if (pos > 0) {
				i = (size_t)(pos - 1);
				continue;
			} 

			break;
		}

		memset(buffer.data, 0, buffer.capacity);

		if (parsers_is_typedef_private(file, i)) {
			for (size_t j = i + typedef_word.size - 1; j < file->size; j++) {
				if (file->data[j] == ';') {
					if ((j - i) > buffer.capacity - buffer.size) {
						return parser_buffer_overflow_error;
					}

					memcpy(buffer.data + buffer.size, file->data + i, j - i);
					printf("t(%s)\n", buffer.data);

					eparser_entity entity = parsers_parse_typedef_private(file, &i, mem);
					if (entity.error != parser_successfull) {
						printf("entity_error: %d\n", entity.error);
						return entity.error;
					} 

					printf("\n");
					parser_entitys_print(&entity.value);
					printf("\n");

					i = j;
					break;
				}
			}
		} else if (parsers_is_struct_private(file, i)) {
			for (size_t j = i + struct_word.size - 1; j < file->size; j++) {
				if (file->data[j] == ';') {
					if ((j - i) > buffer.capacity - buffer.size) {
						return parser_buffer_overflow_error;
					}

					memcpy(buffer.data + buffer.size, file->data + i, j - i);
					printf("s(%s)\n", buffer.data);

					i = j;
					break;
				}
			}
		} else if (parsers_is_enum_private(file, i)) {
			for (size_t j = i + enum_word.size - 1; j < file->size; j++) {
				if (file->data[j] == ';') {
					if ((j - i) > buffer.capacity - buffer.size) {
						return parser_buffer_overflow_error;
					}

					memcpy(buffer.data + buffer.size, file->data + i, j - i);
					printf("e(%s)\n", buffer.data);

					i = j;
					break;
				}
			}
		} else if (parsers_is_union_private(file, i)) {
			for (size_t j = i + union_word.size - 1; j < file->size; j++) {
				if (file->data[j] == ';') {
					if ((j - i) > buffer.capacity - buffer.size) {
						return parser_buffer_overflow_error;
					}

					memcpy(buffer.data + buffer.size, file->data + i, j - i);
					printf("u(%s)\n", buffer.data);

					i = j;
					break;
				}
			}
		} else if (parsers_is_function_private(file, i, &function_end)) {
			if ((function_end - i) > buffer.capacity - buffer.size) {
				return parser_buffer_overflow_error;
			}

			memcpy(buffer.data + buffer.size, file->data + i, function_end - i);
			printf("f(%s)\n", buffer.data);

			i = function_end;
		} else {
			ssize_t next_space = -1;
			for (size_t j = i; j < file->size; j++) {
				if (file->data[j] == letter) {
					next_space = (ssize_t)j;
					break;
				}
			}
			
			if (next_space == -1) {
				break;
			}

			ssize_t pos = parsers_find_next_character_private(file, next_space);
			if (pos <= 0) {
				break;
			}

			if ((next_space - i) > buffer.capacity - buffer.size) {
				return parser_buffer_overflow_error;
			}

			memcpy(buffer.data + buffer.size, file->data + i, next_space - i);
			printf("k(%s)\n", buffer.data);

			i = (size_t)(pos - 1);
			continue;
		}
	}

	return ok;
}

error parsers_get_entities(const char *path, const allocator *mem) {
	#if cels_debug
		errors_abort("path", strs_check(path));
	#endif

	file *file = fopen(path, "r");
	if (!file) {
		printf("file not found\n");
		return fail;
	}

	estring file_buffer = files_read(file, mem);
	fclose(file);

	if (file_buffer.error != file_successfull) {
		printf("file not read\n");
		return fail;
	}

	parsers_normalize(&file_buffer.value);
	strings_println(&file_buffer.value);

	error error = parsers_get_entities_private(&file_buffer.value, mem);
	printf("error: %d\n", error);

	strings_free(&file_buffer.value, mem);
	return ok;
}
