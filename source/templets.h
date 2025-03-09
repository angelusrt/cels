#ifndef cels_templets_h
#define cels_templets_h

#include "mems.h"
#include "vectors.h"
#include "strings.h"
#include "files.h"
#include "utils.h"
#include "nodes.h"
#include "jsons.h"


/*
 * The module 'templets' is responsible 
 * for parsing html and creating views.
 */


/* templets */

typedef enum templet_error {
	templet_successfull,
	templet_generic_error,
	templet_not_found_error,
	templet_not_open_error,
	templet_not_read_error,
	templet_not_closed_error,
	templet_invalid_tag_error,
	templet_invalid_size_error,
	templet_define_must_appear_once_error,
	templet_allocation_error,
	templet_variable_missing_error,
	templet_mal_formed_error,
	templet_json_mal_formed_error,
	templet_not_a_list_error,
} templet_error;

typedef enum templet_operator {
	templet_none_operator,
	templet_assignment_operator,
	templet_define_operator,
	templet_for_operator,
	templet_end_operator,
	templet_private_operator,
} templet_operator;

typedef struct templet {
	templet_operator op;
	string text;
	string alias;
} templet;

typedef struct templet_tree_node templet_tree_node;
typedef munodes(templet_tree_node, templet) templet_tree_node;
typedef mutrees(templet_tree_node) templet_tree;
typedef mutree_iterators(templet_tree_node) templet_tree_iterator;

maps(templet_map, string, templet_tree)
typedef errors(templet_map) etemplet_map;

/*
 * Parses templet to templets.
 */
cels_warn_unused
error templets_parse(
	templet_map *templets, const string *templet, const allocator *mem);

/*
 * Creates a map of parsed-templet 
 * files, provided by glob-path, ready 
 * to be executed.
 */
cels_warn_unused
etemplet_map templets_make(const string path, const allocator *mem);

/*
 * Executes the template, generating a 
 * string if process functioned properly 
 * else it returns an error.
 */
cels_warn_unused
estring templets_unmake(
	templet_map *templets, 
	const char *templet_name, 
	string_map *options, 
	const allocator *mem);

/*
 * Executes the template, generating a 
 * string if process functioned properly 
 * else it returns an error.
 */
cels_warn_unused
estring templets_unmake_with(
	templet_map *templets, 
	const char *templet_name, 
	const string *options, 
	const allocator *mem);

#endif
