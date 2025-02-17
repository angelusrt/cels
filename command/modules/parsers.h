#ifndef parsers_h
#define parsers_h

#include "../packages/cels/strings.h"
#include "../packages/cels/files.h"

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

typedef vectors(parser_entity *) parser_entity_vec;
typedef errors(parser_entity_vec) eparser_entity_vec;

/*
 * Prints entity.
 */
void parser_entitys_print(parser_entity *entity);

/*
 * Gets entities. 
 */
eparser_entity_vec parsers_get_entities(const char *path, const allocator *mem);

#endif
