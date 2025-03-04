#include "parsers.h"


/* constants */

static const char letter = '.';
static const string typedef_word = strings_premake("typedef");
static const string struct_word = strings_premake("struct");
static const string enum_word = strings_premake("enum");
static const string union_word = strings_premake("union");


/* parser_entitys */

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
            printf(",.return=");
            strings_imprint(&entity->function_definition_return);
            break;
        case parser_function_implementation_entity_type:
            printf(".type=function_implementation,.argument=");
            strings_imprint(&entity->function_argument);
            printf(",.return=");
            strings_imprint(&entity->function_return);
            printf(",.body=");
            strings_imprint(&entity->function_body);
            break;
    }

    printf(",.range={%zu, %zu}}\n", entity->range[0], entity->range[1]);
}


/* parsers */

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

eparser_entity parsers_parse_typedef_private(
	string *file, size_t *cursor, const allocator *mem) {

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

	ssize_t second_term_pos = parsers_find_next_character_private(
		file, initial_pos);

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
		string alias = strings_copy(
			file, second_term_pos, second_term_end_pos, mem);

		entity.alias = alias;

		string name = strings_copy(
			file, third_term_pos, third_term_end_pos, mem);

		entity.name = name;

		return (eparser_entity){.value=entity};
	} 

	//other types
	
	ssize_t argument_pos = -1;
	size_t step = second_term_end_pos;

	for (size_t i = step; i > (size_t)second_term_pos; i--) {
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

eparser_entity parsers_parse_function_definition_private(
	string *file, size_t *cursor, size_t end, const allocator *mem
) {

	ssize_t parenthesis_end_pos = -1;
	ssize_t parenthesis_start_pos = -1;
	for (size_t i = end; i > *cursor; i--) {
		if (file->data[i] == ')') {
			parenthesis_end_pos = i;
		} else if (file->data[i] == '(') {
			parenthesis_start_pos = i;
			break;
		}
	}

	if (parenthesis_start_pos == -1 || parenthesis_end_pos == -1) {
		return (eparser_entity){.error=parser_mal_formed_entity_error};
	}

	if (parenthesis_start_pos + 1 >= parenthesis_end_pos) {
		return (eparser_entity){.error=parser_mal_formed_entity_error};
	}

	bool has_name_started = false;
	ssize_t name_end_pos = -1;
	ssize_t name_pos = -1;
	for (size_t i = parenthesis_start_pos; i > *cursor; i--) {
		if (!has_name_started && file->data[i] != letter) {
			name_end_pos = i;
			has_name_started = true;
		} else if (has_name_started && file->data[i] == letter) {
			name_pos = i + 1;
			break;
		}
	}

	if (name_end_pos == -1 || name_pos == -1) {
		return (eparser_entity){.error=parser_mal_formed_entity_error};
	}

	if (name_pos - 1 < 0) {
		return (eparser_entity){.error=parser_mal_formed_entity_error};
	}

	bool has_type_started = false;
	ssize_t type_end_pos = -1;
	ssize_t type_pos = -1;
	for (size_t i = name_pos - 1; i >= *cursor; i--) {
		if (!has_type_started && file->data[i] != letter) {
			type_end_pos = i + 1;
			has_type_started = true;
		} else if (has_type_started && file->data[i] == letter) {
			type_pos = i + 1;
			break;
		}
	}

	if (type_pos == -1) {
		type_pos = *cursor;
	}

	if (type_end_pos == -1) {
		return (eparser_entity){.error=parser_mal_formed_entity_error};
	}

	parser_entity entity = {0};
	entity.range[0] = *cursor;
	entity.range[1] = end;

	entity.type = parser_function_definition_entity_type;
	entity.name = strings_copy(file, name_pos, name_end_pos, mem);
	entity.function_definition_argument = strings_copy(
		file, parenthesis_start_pos + 1, parenthesis_end_pos, mem);
	entity.function_definition_return = strings_copy(
		file, type_pos, type_end_pos, mem);

	return (eparser_entity){.value=entity};
}

void parser_entity_vecs_push(
	parser_entity_vec *self, parser_entity item, const allocator *mem) {

	if (self->size >= self->capacity) {
		size_t new_capacity = self->capacity << 1;
		errors_abort("self->capacity (overflow)", 
			new_capacity < self->capacity);

		void *new_data = mems_realloc(
			mem, self->data, 
			self->capacity * sizeof(parser_entity), 
			new_capacity * sizeof(parser_entity));

		errors_abort("new_data", !new_data);

		self->data = new_data;
		self->capacity = new_capacity;
	}

	++self->size;
	self->data[self->size - 1] = item;
}

eparser_entity_vec parsers_get_entities_private(
	string *file, const allocator *mem) {

	parser_entity_vec entities = {0};
	entities.capacity = 16; 
	entities.data = mems_alloc(mem, sizeof(parser_entity) * 16);
	errors_abort("entities.data", !entities.data);

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

		if (parsers_is_typedef_private(file, i)) {
			for (size_t j = i + typedef_word.size - 1; j < file->size; j++) {
				if (file->data[j] == ';') {
					eparser_entity entity = parsers_parse_typedef_private(
						file, &i, mem);

					if (entity.error != parser_successfull) {
						printf("entity_error: %d\n", entity.error);
						return (eparser_entity_vec){.error=entity.error};
					} 

					parser_entity_vecs_push(&entities, entity.value, mem);

					/*printf("\n");
					parser_entitys_print(&entity.value);
					printf("\n");*/

					i = j;
					break;
				}
			}
		} else if (parsers_is_struct_private(file, i)) {
			for (size_t j = i + struct_word.size - 1; j < file->size; j++) {
				if (file->data[j] == ';') {
					i = j;
					break;
				}
			}
		} else if (parsers_is_enum_private(file, i)) {
			for (size_t j = i + enum_word.size - 1; j < file->size; j++) {
				if (file->data[j] == ';') {
					i = j;
					break;
				}
			}
		} else if (parsers_is_union_private(file, i)) {
			for (size_t j = i + union_word.size - 1; j < file->size; j++) {
				if (file->data[j] == ';') {
					i = j;
					break;
				}
			}
		} else if (parsers_is_function_private(file, i, &function_end)) {
			eparser_entity entity = parsers_parse_function_definition_private(
				file, &i, function_end, mem);

			if (entity.error != parser_successfull) {
				printf("entity_error: %d\n", entity.error);
				return (eparser_entity_vec){.error=entity.error};
			} 

			parser_entity_vecs_push(&entities, entity.value, mem);
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

			i = (size_t)(pos - 1);
			continue;
		}
	}

	return (eparser_entity_vec){.value=entities};
}

eparser_entity_vec parsers_get_entities(
	const char *path, const allocator *mem) {

	#if cels_debug
		errors_abort("path", strs_check(path));
	#endif

	file *file = fopen(path, "r");
	if (!file) {
		printf("file not found\n");
		return (eparser_entity_vec){.error=fail};
	}

	ebyte_vec file_buffer = files_read(file, mem);
	fclose(file);

	if (file_buffer.error != file_successfull) {
		printf("file not read\n");
		return (eparser_entity_vec){.error=fail};
	}

	estring fb = byte_vecs_to_string(&file_buffer.value, mem);
	if (fb.error != ok) {
		printf("file is bynary or mal-formed\n");
		return (eparser_entity_vec){.error=fail};
	}

	printf("arquivo:\n");
	strings_println(&fb.value);

	printf("arquivo normalizado:\n");
	parsers_normalize(&fb.value);
	strings_println(&fb.value);

	eparser_entity_vec entities = parsers_get_entities_private(&fb.value, mem);
	strings_free(&fb.value, mem);

	return entities;
}
