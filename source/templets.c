#include "templets.h"
#include <string.h>

bool templets_check(const templet *self) {
	#if cels_debug
		errors_return("self", !self)
		errors_return("self.text", strings_check_extra(&self->text))
		errors_return("self.op", !(self->op >= 0 && self->op < templet_private_operator))
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

trees_generate(templet_tree, templet, templets_check, templets_free)

void templet_trees_println(const templet_tree *self) {
	#if cels_debug
		errors_abort("self", !self);
	#endif

	templet_tree_iterator it = {0};
	while(templet_trees_next(self, &it)) {
		#if cels_debug
			errors_abort("self.data[i].data", templets_check(&it.data->data));
		#endif

		templets_print(&it.data->data);
	}

	printf("\n");
}

/* templet_maps */

maps_generate(
	templet_map,
	string, 
	templet_tree, 
	strings_check,
	templet_trees_check,
	strings_print,
	templet_trees_println,
	strings_hasherize,
	strings_free,
	templet_trees_free)

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

	if (tokens.size == 1) {
		string token = tokens.data[0];
		mems_dealloc(mem, tokens.data, tokens.capacity);

		return (templet) {
			.text=token,
			.op=templet_assignment_operator
		};
	} else if (tokens.size == 2) {
		if (tokens.data[1].size == 0) { goto cleanup0; }

		static const string define_keyword = strings_premake("define");
		if (strings_equals(&tokens.data[0], &define_keyword)) {
			string token = tokens.data[1];
			mems_dealloc(mem, tokens.data, tokens.capacity);
			strings_free(&tokens.data[0], mem);

			return (templet){
				.text=token,
				.op=templet_define_operator,
			};
		}
	}

	cleanup0:
	string_vecs_free(&tokens, mem);
	return (templet){0};
}

error templets_parse(templet_map *templets, const string *templet, const allocator *mem) {
	#if cels_debug
		errors_abort("templets", !templets);
		errors_abort("templet", strings_check_extra(templet));
		//errors_abort("templets", bynary_nodes_check((const bynary_node *)templets->data));
	#endif

	static const string tag_open = strings_premake("<|");
	static const string tag_close = strings_premake("|>");

	error error = 0;
	string key = {0};
	templet_tree sections = {0};

	for (size_t i = 0; i < templet->size; i++) {
		if (templet->data[i] == '<' && templet->data[i + 1] == '|') {
			ssize_t pos = strings_find_matching(templet, tag_open, tag_close, i);
			if (pos == -1) {
				error = templet_not_closed_error;
				goto cleanup0;
			}

			string tagging = strings_copy(
					templet, i + 2, pos - 1, mem);
			struct templet tag = templets_parse_tag(&tagging, mem);

			if (tag.op == 0) {
				error = templet_invalid_tag_error;
				goto cleanup0;
			} else if (tag.op == templet_define_operator) {
				if (key.size != 0) {
					error = templet_define_must_appear_once_error;
					goto cleanup0;
				}

				key = tag.text;
			} else {
				templet_tree_node *node = mems_alloc(mem, sizeof(templet_tree_node));
				if (!node) {
					strings_free(&tag.text, mem);
					goto cleanup0;
				}

				node->data = tag;

				bool push_error = templet_trees_push(&sections, sections.data, node);
				if (push_error) {
					strings_free(&tag.text, mem);
					goto cleanup0;
				}
			}

			i = pos + 1;
		} else {
			ssize_t pos = strings_find(templet, tag_open, i);
			if (pos == -1) {
				pos = templet->size - 2;
			}

			size_t start = i == 0 ? 0 : i - 1;
			string section = strings_copy(templet, start, pos, mem);
			struct templet sec = {.text=section, .op=templet_none_operator};

			templet_tree_node *node = mems_alloc(mem, sizeof(templet_tree_node));
			if (!node) {
				strings_free(&sec.text, mem);
				goto cleanup0;
			}

			node->data = sec;

			bool push_error = templet_trees_push(&sections, sections.data, node);
			if (push_error) {
				strings_free(&section, mem);

				error = templet_allocation_error;
				goto cleanup0;
			}

			if ((size_t)pos == templet->size - 2) {
				break;
			}

			i = pos - 1;
		}
	}

	bool push_error = templet_maps_push(templets, key, sections, mem);
	if (push_error) {
		strings_free(&key, mem);

		error = templet_allocation_error;
		goto cleanup0;
	}

	return templet_successfull;

	cleanup0:
	templet_trees_free(&sections, mem);
	return error;
}

/* public */

etemplet_map templets_make(const string path, const allocator *mem) {
	#if cels_debug
		errors_abort("path", strings_check_extra(&path));
	#endif

	estring_vec files = files_list(path, mem);
	if (files.error != file_successfull) {
		return (etemplet_map){.error=templet_not_found_error};
	}

	error error = 0;
	templet_map map = templet_maps_init();
	for (size_t i = 0; i < files.value.size; i++) {
		string filepath = strings_format(
			"%s%s", mem, path.data, files.value.data[i].data);

		file *file = fopen(filepath.data, "r");
		strings_free(&filepath, mem);

		if (!file) {
			error = templet_not_open_error;
			goto cleanup0;
		}

		estring fileread = files_read(file, mem);
		fclose(file);

		if (fileread.error != file_successfull) {
			error = templet_not_read_error;
			goto cleanup0;
		}

		error = templets_parse(&map, &fileread.value, mem);
		if (error != templet_successfull) {
			goto cleanup0;
		}
	}

	string_vecs_free(&files.value, mem);
	return (etemplet_map){.value=map};

	cleanup0:
	string_vecs_free(&files.value, mem);
	if (map.data) {
		templet_maps_free(&map, mem);
	}

	return (etemplet_map){.error=error};
}
