#ifndef templates_h
#define templates_h

#include "mems.h"
#include "vectors.h"
#include "strings.h"
#include "files.h"
#include "utils.h"

/* template_errors */

typedef enum template_error {
	template_successfull,
	template_not_found_error,
	template_not_open_error,
	template_not_read_error,
	template_not_closed_error,
	template_invalid_tag_error,
	template_define_must_appear_once_error,
	template_allocation_error,
} template_error;

/* templates */

typedef struct template template;
vectors_generate_definition(template, template_vec)

typedef struct template {
	string text;
	uchar operator;
	template_vec *next;
} template;

/* template_maps */

maps_generate_definition(string, template_vec, template_vec_key_pair, template_map)
typedef errors(template_map *) etemplate_map;

/* templates */

__attribute_warn_unused_result__
etemplate_map templates_make(const string path, const allocator *mem);


#endif
