#ifndef cels_templets_h
#define cels_templets_h

#include "mems.h"
#include "vectors.h"
#include "strings.h"
#include "files.h"
#include "utils.h"
#include "nodes.h"

/* templets */

/*
 * The module 'templets' is responsible 
 * for parsing html and creating views.
 */

/* templet_errors */

typedef enum templet_error {
	templet_successfull,
	templet_not_found_error,
	templet_not_open_error,
	templet_not_read_error,
	templet_not_closed_error,
	templet_invalid_tag_error,
	templet_define_must_appear_once_error,
	templet_allocation_error,
} templet_error;

typedef enum templet_operator {
	templet_none_operator,
	templet_assignment_operator,
	templet_define_operator,
	templet_private_operator,
} templet_operator;

typedef struct templet {
	string text;
	templet_operator op;
} templet;

trees_define(templet_tree, templet)

maps_define(templet_map, string, templet_tree)

typedef errors(templet_map) etemplet_map;

/*
 * Parses templet to templets.
 */
cels_warn_unused
error templets_parse(templet_map *templets, const string *templet, const allocator *mem);

/*
 * Creates a map of parsed-templet 
 * files provided by glob-path ready 
 * to be executed.
 */
cels_warn_unused
etemplet_map templets_make(const string path, const allocator *mem);

#endif
