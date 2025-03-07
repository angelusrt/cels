#include "templets.h"


/* templets */

bool templets_check(const templet *self) {
	#if cels_debug
		errors_return("self", !self)
		errors_return("self.text", strings_check_extra(&self->text))
		bool is_within = 
			self->op >= 0 && 
			self->op < templet_private_operator;

		errors_return("self.op", !is_within)
	#else
		if (!self) return true;
		if (strings_check_extra(&self->text)) return true;

		bool is_within = 
			self->op >= 0 && 
			self->op < templet_private_operator;
		if (!is_within) return true;
	#endif

	return false;
}

void templets_print(const templet *self) {
	#if cels_debug
		errors_abort("self", templets_check(self));
	#endif

	printf("%d{", self->op);
	strings_imprint(&self->text);
	printf("}\n");
}

void templets_free(templet *self, const allocator *mem) {
	#if cels_debug
		errors_abort("self", templets_check(self));
	#endif

	strings_free(&self->text, mem);
}


/* templet_trees */

void templet_trees_println(const templet_tree *self) {
	#if cels_debug
		errors_abort("self", !self);
	#endif

	templet_tree_iterator it = {0};
	while(mutrees_next(self, &it)) {
		#if cels_debug
			errors_abort("self.data[i].data", templets_check(&it.data->data));
		#endif

		templets_print(&it.data->data);
	}

	printf("\n");
}

void templet_trees_free(templet_tree *self, const allocator *mem) {
	mutrees_free(self, (freefunc)templets_free, mem);
}


/* private */

cels_warn_unused
templet templets_parse_tag(own string *tag, const allocator *mem) {
	#if cels_debug
		errors_abort("tag", strings_check_view(tag));
	#endif

	const string alt_whitespaces = strings_premake("\t\r\n");
	const string the_whitespace = strings_premake(" ");

	strings_replace_from(tag, alt_whitespaces, ' ', 0);
	string_vec tokens = strings_split(tag, the_whitespace, 0, mem);
	strings_free(tag, mem);

	if (tokens.size == 0) {
		goto cleanup0;
	}

	static const string define_keyword = strings_premake("define");
	static const string for_keyword = strings_premake("for");
	static const string in_keyword = strings_premake("in");
	static const string end_keyword = strings_premake("end");

	if (strings_equals(&tokens.data[0], &define_keyword)) {
		if (tokens.size != 2) { 
			goto cleanup0;
		}

		string token = tokens.data[1];
		mems_dealloc(mem, tokens.data, tokens.capacity);
		strings_free(&tokens.data[0], mem);

		return (templet){
			.text=token,
			.op=templet_define_operator,
		};
	} else if (strings_equals(&tokens.data[0], &for_keyword)) {
		if (tokens.size != 4) { 
			goto cleanup0;
		}

		if (!strings_equals(&tokens.data[2], &in_keyword)) {
			goto cleanup0;
		}

		string token = tokens.data[3];
		string alias = tokens.data[1];

		mems_dealloc(mem, tokens.data, tokens.capacity);
		strings_free(&tokens.data[0], mem);
		strings_free(&tokens.data[2], mem);

		return (templet){
			.text=token,
			.alias=alias,
			.op=templet_for_operator,
		};
	} else if (strings_equals(&tokens.data[0], &end_keyword)) {
		vectors_free(&tokens, (freefunc)strings_free, mem);

		return (templet){
			.op=templet_end_operator,
		};
	} else {
		string token = tokens.data[0];
		mems_dealloc(mem, tokens.data, tokens.capacity);

		return (templet) {
			.text=token,
			.op=templet_assignment_operator
		};
	}

	cleanup0:
	vectors_free(&tokens, (freefunc)strings_free, mem);
	return (templet){0};
}

error templets_parse(
	templet_map *templets, const string *templet, const allocator *mem) {

	#if cels_debug
		errors_abort("templets", !templets);
		errors_abort("templet", strings_check_extra(templet));
	#endif

	static const string tag_open = strings_premake("<|");
	static const string tag_close = strings_premake("|>");

	error err = 0;
	string key = {0};
	templet_tree sections = {0};
	templet_tree_node *leaf = sections.data;

	for (size_t i = 0; i < templet->size; i++) {
		if (templet->data[i] == '<' && templet->data[i + 1] == '|') {
			ssize_t pos = strings_find_matching(
				templet, tag_open, tag_close, i);

			if (pos == -1) {
				err = templet_not_closed_error;
				goto cleanup0;
			}

			string tagging = strings_copy(
					templet, i + 2, pos - 1, mem);
			struct templet tag = templets_parse_tag(&tagging, mem);

			if (tag.op == 0) {
				err = templet_invalid_tag_error;
				goto cleanup0;
			} else if (tag.op == templet_define_operator) {
				if (key.size != 0) {
					err = templet_define_must_appear_once_error;
					goto cleanup0;
				}

				key = tag.text;
			} else if (tag.op == templet_for_operator) {
				templet_tree_node *node = mems_alloc(
					mem, sizeof(templet_tree_node));

				if (!node) {
					strings_free(&tag.text, mem);
					strings_free(&tag.alias, mem);
					err = templet_allocation_error;
					goto cleanup0;
				}

				node->data = tag;
				error push_error = mutrees_push(&sections, leaf, node);
				if (push_error) {
					strings_free(&tag.text, mem);
					strings_free(&tag.alias, mem);
					err = templet_allocation_error;
					goto cleanup0;
				}

				leaf = node;
			} else if (tag.op == templet_end_operator) {
				if (!leaf->parent) {
					err = templet_invalid_tag_error;
					goto cleanup0;
				}

				leaf = leaf->parent;
			} else {
				templet_tree_node *node = mems_alloc(
					mem, sizeof(templet_tree_node));

				if (!node) {
					strings_free(&tag.text, mem);
					err = templet_allocation_error;
					goto cleanup0;
				}

				node->data = tag;

				error push_error = false;
				bool is_for = 
					leaf && 
					leaf->data.op == templet_for_operator && 
					!leaf->down;

				if (is_for) {
					push_error = mutrees_attach(&sections, leaf, node);
					leaf = node;
				} else {
					push_error = mutrees_push(&sections, leaf, node);
				}

				if (push_error) {
					strings_free(&tag.text, mem);
					err = templet_allocation_error;
					goto cleanup0;
				}

				if (!leaf) {
					//may be initializing
					leaf = node;
				}
			}

			i = pos + 1;
		} else {
			ssize_t pos = strings_find(templet, tag_open, i);
			if (pos == -1) {
				pos = templet->size - 1;
			}

			size_t start = i == 0 ? 0 : i - 1;
			string section = strings_copy(templet, start, pos, mem);
			struct templet sec = {.text=section, .op=templet_none_operator};

			templet_tree_node *node = mems_alloc(
				mem, sizeof(templet_tree_node));

			if (!node) {
				strings_free(&sec.text, mem);
				err = templet_allocation_error;
				goto cleanup0;
			}

			node->data = sec;

			error push_error = ok;

			bool is_for = 
				leaf && 
				leaf->data.op == templet_for_operator && 
				!leaf->down;

			if (is_for) {
				push_error = mutrees_attach(&sections, leaf, node);
				leaf = node;
			} else {
				push_error = mutrees_push(&sections, leaf, node);
			}

			if (push_error) {
				strings_free(&section, mem);
				err = templet_allocation_error;
				goto cleanup0;
			}

			if (!leaf) {
				//may be initializing
				leaf = node;
			}

			if ((size_t)pos == templet->size - 1) {
				break;
			}

			i = pos - 1;
		}
	}

	templet_map_pair pair = {.key=key, .value=sections};
	bool push_error = maps_push(templets, &pair, strings_hash(&pair.key), mem);
	if (push_error) {
		strings_free(&key, mem);

		err = templet_allocation_error;
		goto cleanup0;
	}

	return templet_successfull;

	cleanup0:
	mutrees_free(&sections, (freefunc)templets_free, mem);
	return err;
}

/* public */

etemplet_map templets_make(const string path, const allocator *mem) {
	#if cels_debug
		errors_abort("path", strings_check_extra(&path));
	#endif

	estring_vec files = dirs_list(path, mem);
	if (files.error != file_successfull) {
		return (etemplet_map){.error=templet_not_found_error};
	}

	error error = 0;
	templet_map map = {0};
	maps_init(map);

	for (size_t i = 0; i < files.value.size; i++) {
		string filepath = strings_format(
			"%s%s", mem, path.data, files.value.data[i].data);

		file *file = fopen(filepath.data, "r");
		strings_free(&filepath, mem);

		if (!file) {
			error = templet_not_open_error;
			goto cleanup0;
		}

		ebyte_vec fileread = files_read(file, mem);
		fclose(file);

		if (fileread.error != file_successfull) {
			error = templet_not_read_error;
			goto cleanup0;
		}

		estring f = byte_vecs_to_string(&fileread.value, mem);
		if (f.error != ok) {
			error = templet_mal_formed_error;
			goto cleanup0;
		}

		error = templets_parse(&map, &f.value, mem);
		if (error != templet_successfull) {
			goto cleanup0;
		}
	}

	vectors_free(&files.value, (freefunc)strings_free, mem);
	return (etemplet_map){.value=map};

	cleanup0:
	vectors_free(&files.value, (freefunc)strings_free, mem);
	if (map.data) {
		maps_free(
			&map, 
			(freefunc)strings_free, 
			(freefunc)templet_trees_free, 
			mem);
	}

	return (etemplet_map){.error=error};
}

typedef struct templet_stack {
	munode *node;
	string_map list;
	string_map object;
	string alias;
	size_t cursor;
} templet_stack;

typedef struct {
  size_t size;
  size_t capacity;
  templet_stack *data;
} templet_stack_vec;

estring templets_get_value_private(
	string_map *map, string text, const allocator *mem) {

	#if cels_debug
		errors_abort("map", !map);
		errors_abort("map", bynodes_check((bynode *)map->data));
		errors_abort("text", strings_check_extra(&text));
	#endif

	error err = ok;
	string_map json = *map;
	string item = text;
	ssize_t object_start = strings_find_with(&text, ".", 0);
	string value = {0};

	while (true) {
		if (object_start > -1) {
			ssize_t object_end = strings_find_with(
				&text, ".", object_start + 1);

			if (object_end == -1) {
				object_end = text.size - 1;
			}

			if (object_end <= object_start + 1) {
				err = templet_invalid_size_error;
				goto cleanup0;
			}

			item = strings_view(&text, object_start + 1, object_end);

			if (object_end == -1) {
				object_start = -1;
			}
		}

		string *value = maps_get(&json, strings_hash(&item));
		if (!value) {
			err = templet_variable_missing_error;
			goto cleanup0;
		}

		if (object_start == -1) {
			break;
		}

		estring_map new_json = jsons_unmake(value, mem);
		if (new_json.error != json_successfull) {
			err = templet_json_mal_formed_error;
			goto cleanup0;
		} 

		if (json.data != map->data) {
			maps_free(
				&json, 
				(freefunc)strings_free, 
				(freefunc)strings_free, 
				mem);

			json.data = null;
		}

		json = new_json.value;
	}

	return (estring){.value=value};

	cleanup0:
	if (json.data != map->data) {
		maps_free(
			&json, 
			(freefunc)strings_free, 
			(freefunc)strings_free, 
			mem);
	}

	return (estring){.error=err};
}

estring_map templets_get_json_private(
	string_map *map, string text, const allocator *mem) {

	#if cels_debug
		errors_abort("map", !map);
		errors_abort("map", bynodes_check((bynode *)map->data));
		errors_abort("text", strings_check_extra(&text));
	#endif

	error err = ok;
	string_map json = *map;
	string item = text;
	ssize_t object_start = strings_find_with(&text, ".", 0);

	while (true) {
		if (object_start > -1) {
			ssize_t object_end = strings_find_with(
				&text, ".", object_start + 1);

			if (object_end == -1) {
				object_end = text.size - 1;
			}

			if (object_end <= object_start + 1) {
				err = templet_invalid_size_error;
				goto cleanup0;
			}

			item = strings_view(&text, object_start + 1, object_end);

			if (object_end == -1) {
				object_start = -1;
			}
		}

		string *value = maps_get(&json, strings_hash(&item));
		if (!value) {
			err = templet_variable_missing_error;
			goto cleanup0;
		}

		estring_map new_json = jsons_unmake(value, mem);
		if (new_json.error != json_successfull) {
			err = templet_json_mal_formed_error;
			goto cleanup0;
		} 

		if (json.data != map->data) {
			maps_free(
				&json, 
				(freefunc)strings_free, 
				(freefunc)strings_free, 
				mem);

			json.data = null;
		}

		json = new_json.value;
		if (object_start == -1) {
			break;
		}
	}

	return (estring_map){.value=json};

	cleanup0:
	if (json.data != map->data) {
		maps_free(
			&json, 
			(freefunc)strings_free, 
			(freefunc)strings_free, 
			mem);
	}

	return (estring_map){.error=err};
}

estring templets_unmake(
	templet_map *templets, 
	const char *templet_name, 
	string_map *options, 
	const allocator *mem) {

	#if cels_debug
		errors_abort("templets", !templets);
		errors_abort("templets", bynodes_check((bynode *)templets->data));
		errors_abort("templet_name", strs_check(templet_name));
		errors_abort("options", !options);
		errors_abort("options", bynodes_check((bynode *)options->data));
	#endif

	error err = ok;
	const string templet_name_capsule = strings_encapsulate(templet_name);
	templet_tree *templet = maps_get(
			templets, strings_hash(&templet_name_capsule));
	
	if (!templet) {
		err = templet_allocation_error;
		goto cleanup0;
	}

	templet_stack_vec stack = {0};
	error init_error = vectors_init(&stack, sizeof(templet_stack), 4, mem);
	if (init_error) { goto cleanup1; }

	string document = strings_init(string_small_size, mem);
	templet_tree_iterator it = {0};

	while (mutrees_next(templet, &it)) {
		if (it.data->data.op == templet_none_operator) {
			error push_error = strings_push(&document, it.data->data.text, mem);
			if (push_error) {
				err = templet_allocation_error;
				goto cleanup1;
			}
		} else if (it.data->data.op == templet_assignment_operator) {
			ssize_t stack_pos = -1;
			for (ssize_t i = (ssize_t)stack.size - 1; i >= 0; i--) {
				ssize_t pos = strings_find(
					&it.data->data.text, stack.data[i].alias, 0);

				if (pos == 0) {
					stack_pos = i;
					break;
				}
			}

			estring value = {0};
			if (stack_pos == -1) {
				value = templets_get_value_private(
					options, it.data->data.text, mem);
			} else {
				value = templets_get_value_private(
					&stack.data[stack_pos].object, it.data->data.text, mem);
			}

			if (value.error != ok) {
				err = value.error;
				goto cleanup1;
			}

			error push_error = strings_push(&document, value.value, mem);
			if (push_error) {
				goto cleanup1;
			}
		} else if (it.data->data.op == templet_for_operator) {
			estring_map value_map = {0};

			ssize_t stack_pos = -1;
			for (ssize_t i = (ssize_t)stack.size - 1; i >= 0; i--) {
				ssize_t pos = strings_find(
					&it.data->data.text, stack.data[i].alias, 0);

				if (pos == 0) {
					stack_pos = i;
					break;
				}
			}

			if (stack_pos == -1) {
				value_map = templets_get_json_private(
					options, it.data->data.text, mem);
			} else {
				value_map = templets_get_json_private(
					&stack.data[stack_pos].object, it.data->data.text, mem);
			}

			if (value_map.error != ok) {
				err = value_map.error;
				goto cleanup1;
			}

			static const string key = strings_premake("0");
			string *object = maps_get(&value_map.value, strings_hash(&key));
			if (!object) {
				err = templet_variable_missing_error;
				goto cleanup1;
			}

			//TODO: make list of primitives work!
			
			estring_map object_json = jsons_unmake(object, mem);
			if (object_json.error != json_successfull) {
				err = templet_not_a_list_error;
				goto cleanup1;
			}

			templet_stack node = {
				.node=it.internal.current,
				.list=value_map.value,
				.object=object_json.value,
				.alias=it.data->data.alias,
				.cursor=0
			};

			error push_error = vectors_push(&stack, &node, mem);
			if (push_error) { goto cleanup1; }
		} else if (it.data->data.op == templet_end_operator) {
			if (stack.size == 0) { goto cleanup1; }

			bool is_at_end = 
				stack.data[stack.size - 1].cursor == 
				stack.data[stack.size - 1].list.size - 1;

			if (is_at_end) {
				string_map *l = &stack.data[stack.size - 1].list;
				string_map *o = &stack.data[stack.size - 1].object;

				maps_free(
					l, 
					(freefunc)strings_free, 
					(freefunc)strings_free, 
					mem);

				maps_free(
					o, 
					(freefunc)strings_free, 
					(freefunc)strings_free, 
					mem);

				vectors_pop(&stack, null, mem);
			} else {
				string_map *l = &stack.data[stack.size - 1].list;
				string_map *o = &stack.data[stack.size - 1].object;

				++stack.data[stack.size - 1].cursor;
				maps_free(
					o, 
					(freefunc)strings_free, 
					(freefunc)strings_free, 
					mem);

				string key = strings_preinit(32);
				snprintf(
					key.data, 
					key.capacity, 
					"%zu", 
					stack.data[stack.size - 1].cursor);

				key.size = strlen(key.data) + 1;

				string *object = maps_get(l, strings_hash(&key));
				if (!object) {
					err = templet_variable_missing_error;
					goto cleanup1;
				}

				estring_map object_json = jsons_unmake(object, mem);
				if (object_json.error != json_successfull) {
					err = templet_not_a_list_error;
					goto cleanup1;
				}

				stack.data[stack.size - 1].object = object_json.value;
				it.internal.current = stack.data[stack.size - 1].node;
			}
		}
	}

	for (size_t i = 0; i < stack.size; i++) {
		string_map *l = &stack.data[stack.size - 1].list;
		string_map *o = &stack.data[stack.size - 1].object;

		maps_free(l, (freefunc)strings_free, (freefunc)strings_free, mem);
		maps_free(o, (freefunc)strings_free, (freefunc)strings_free, mem);
	}
	vectors_free(&stack, null, mem);

	return (estring){.value=document};

	cleanup1:
	strings_free(&document, mem);

	for (size_t i = 0; i < stack.size; i++) {
		string_map *l = &stack.data[stack.size - 1].list;
		string_map *o = &stack.data[stack.size - 1].object;

		maps_free(l, (freefunc)strings_free, (freefunc)strings_free, mem);
		maps_free(o, (freefunc)strings_free, (freefunc)strings_free, mem);
	}
	vectors_free(&stack, null, mem);

	cleanup0:
	return (estring){.error=err};
}

estring templets_unmake_with(
	templet_map *templets, 
	const char *templet_name, 
	const string *options, 
	const allocator *mem) {

	estring_map eoptions = jsons_unmake(options, mem);
	if (eoptions.error != json_successfull) {
		return (estring){.error=fail};
	}

	estring templet = templets_unmake(
		templets, templet_name, &eoptions.value, mem);

	maps_free(
		&eoptions.value, 
		(freefunc)strings_free, 
		(freefunc)strings_free, 
		mem);

	return templet;
}
