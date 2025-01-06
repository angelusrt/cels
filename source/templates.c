#include "templates.h"
#include "strings.h"

/* templates */

bool templates_check_private(const template *self) {
	#if cels_debug
		if (errors_check("templates_check_private.self", self == NULL)) return true;
		if (errors_check("templates_check_private.self.text", strings_check_extra(&self->text)) return true;
	#else
		if (self == NULL) return true;
		if (strings_check_extra(&self->text)) return true;
	#endif

    return false;
}

void templates_print_private(const template *self) {
	#if cels_debug
		errors_panic("templates_print_private.self", templates_check_private(self));
	#endif

	printf("%c{", self->operator);
	for (size_t i = 0; i < self->text.size; i++) {
		if (chars_is_whitespace(self->text.data[i])) {
			printf(" ");
		} else {
			printf("%c", self->text.data[i]);
		}
	}
	printf("}\n");
}

bool templates_seems_private(const template *t0, const template *t1) {
	#if cels_debug
		errors_panic("templates_seems_private.t0", templates_check_private(t0));
		errors_panic("templates_seems_private.t1", templates_check_private(t1));
	#endif

	if (t0->operator != t1->operator) {
		return false;
	}

	return strings_seems(&t0->text, &t1->text);	
}

bool templates_equals_private(const template *t0, const template *t1) {
	#if cels_debug
		errors_panic("templates_equals_private.t0", templates_check_private(t0));
		errors_panic("templates_equals_private.t1", templates_check_private(t1));
	#endif

	if (t0->operator != t1->operator) {
		return false;
	}

	return strings_equals(&t0->text, &t1->text);	
}

void templates_free_private(template *self, const allocator *mem) {
	#if cels_debug
		errors_panic("templates_equals_private.self", templates_check_private(self));
	#endif

	strings_free(&self->text, mem);
}

/* template_vecs */

vectors_generate_implementation(
	template, 
	template_vec, 
	templates_check_private,
	templates_print_private,
	templates_equals_private,
	templates_seems_private,
	templates_free_private)

bool template_vecs_check(const template_vec *self) {
	return vectors_check((vector *)self);
}

/* template_maps */

maps_generate_implementation(
	string, 
	template_vec, 
	template_vec_key_pair, 
	template_map,
	strings_check,
	template_vecs_check,
	strings_print,
	template_vecs_print,
	strings_hasherize,
	strings_free,
	template_vecs_free)

/* private */

typedef enum template_operator {
	template_none_operator = 'n',
	template_assignment_operator = 'a',
	template_define_operator = 'd',
} template_operator;

__attribute_warn_unused_result__
template templates_parse_tag_private(string *tag, const allocator *mem) {
	const string alt_whitespaces = strings_premake("\t\r\n");
	const string the_whitespace = strings_premake(" ");

	strings_replace(tag, &alt_whitespaces, ' ', 0);

	string_vec tokens = strings_make_split(tag, &the_whitespace, 0, mem);

	if (tokens.size == 1) {
		mems_dealloc(mem, tokens.data, tokens.capacity);
		return (template) {
			.text=tokens.data[0],
			.operator=template_assignment_operator
		};
	} else if (tokens.size == 2) {
		if (tokens.data[1].size == 0) {
			goto error;
		}

		const string define_keyword = strings_premake("define");
		bool is_define = strings_equals(&tokens.data[0], &define_keyword);
		if (is_define) {
			return (template){
				.text=tokens.data[1],
				.operator=template_define_operator,
			};
		}

	}

	error:
	string_vecs_free(&tokens, mem);
	return (template){0};
}

__attribute_warn_unused_result__
error templates_parse_private(template_map **templates, const string *template, const allocator *mem) {
	static const string tag_open = strings_premake("<%");
	static const string tag_close = strings_premake("%>");

	error error = 0;
	string key = {0};
	template_vec sections = template_vecs_init(vector_min, mem);

	for (size_t i = 0; i < template->size; i++) {
		if (template->data[i] == '<' && template->data[i + 1] == '%') {
			ssize_t pos = strings_find_closing_tag(template, tag_open, tag_close, i);
			if (pos == -1) {
				error = template_not_closed_error;
				goto error;
			}

			string tag_view = {
				.data=template->data+(i + 2),
				.size=pos-(i + 2),
				.capacity=(pos-(i + 2))+1,
			};

			struct template tag = templates_parse_tag_private(&tag_view, mem);
			if (tag.operator == 0) {
				error = template_invalid_tag_error;
				goto error;
			} else if (tag.operator == template_define_operator) {
				if (key.size != 0) {
					error = template_define_must_appear_once_error;
					goto error;
				}

				key = tag.text;
			} else {
				bool push_error = template_vecs_push(&sections, tag, mem);
				if (push_error) {
					strings_free(&tag.text, mem);
					goto error;
				}
			}

			i = pos + 1;
		} else {
			ssize_t pos = strings_find(template, &tag_open, i);
			if (pos == -1) {
				pos = template->size - 2;
			}

			string section_view = {
				.data=template->data+(i-1),
				.size=pos-(i-1),
				.capacity=(pos-(i-1))+2,
			};

			string section = strings_make_copy(&section_view, mem);
			section.size++;
			section.data[section.capacity - 1] = '\0';

			struct template sec = {
				.text=section,
				.next=null,
				.operator=template_none_operator
			};

			bool push_error = template_vecs_push(&sections, sec, mem);
			if (push_error) {
				strings_free(&section, mem);

				error = template_allocation_error;
				goto error;
			}

			if ((size_t)pos == template->size - 2) {
				break;
			}

			i = pos - 1;
		}
	}

	bool push_error = template_maps_push(templates, key, sections, mem);
	if (push_error) {
		strings_free(&key, mem);

		error = template_allocation_error;
		goto error;
	}

	return template_successfull;

	error:
	template_vecs_free(&sections, mem);
	return error;
}

/* public */

etemplate_map templates_make(const string path, const allocator *mem) {
	estring_vec files = files_list(path, mem);
	if (files.error != file_successfull) {
		return (etemplate_map){.error=template_not_found_error};
	}

	error error = 0;
	template_map *map = null;
	for (size_t i = 0; i < files.value.size; i++) {
		string filepath = strings_make_format(
			"%s%s", mem, path.data, files.value.data[i].data);

		file *file = fopen(filepath.data, "r");
		strings_free(&filepath, mem);

		if (!file) {
			error = template_not_open_error;
			goto error0;
		}

		estring fileread = files_read(file, mem);
		fclose(file);

		if (fileread.error != file_successfull) {
			error = template_not_read_error;
			goto error0;
		}

		error = templates_parse_private(&map, &fileread.value, mem);
		if (error != template_successfull) {
			goto error0;
		}
	}

	string_vecs_free(&files.value, mem);
	return (etemplate_map){.value=map};

	error0:
	string_vecs_free(&files.value, mem);
	if (map) {
		template_maps_free(map, mem);
	}

	return (etemplate_map){.error=error};
}
