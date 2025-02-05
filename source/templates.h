#ifndef cels_templates_h
#define cels_templates_h

#include "mems.h"
#include "vectors.h"
#include "strings.h"
#include "files.h"
#include "utils.h"

/*
 * The module 'templates' is responsible 
 * for parsing html and creating views.
 */

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
typedef errors(template_map) etemplate_map;

/* templates */

/*
 * Parses template to templates.
 */
cels_warn_unused
error templates_parse(template_map *templates, const string *template, const allocator *mem);

/*
 * Creates a map of parsed-template 
 * files provided by glob-path ready 
 * to be executed.
 */
cels_warn_unused
etemplate_map templates_make(const string path, const allocator *mem);


#endif
