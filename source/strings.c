#include "strings.h"

bool strings_check(const string *s) {
	#if cels_debug
		if (errors_check("strings_check.s", vectors_check((const vector *)s))) return true;
		if (errors_check("strings_check.s.size == 0", s->size == 0)) return true;
	#else 
		if (vectors_check((const vector *)s)) return true;
		if (s->size == 0) return true;
	#endif

	return false;
}

bool strings_check_extra(const string *s) {
	#if cels_debug
		if (errors_check("strings_check_extra.s", strings_check(s))) return true;
		if (errors_check("strings_check_extra.s.data[0] == '\\0'", s->data[0] == '\0')) return true;
		if (errors_check("strings_check_extra.s.size == 1", s->size == 1)) return true;
	#else
		if (strings_check(s)) return true;
		if (s->data[0] == '\0') return true;
		if (s->size == 1) return true;
	#endif

	return false;
}

bool strings_check_charset(const string *s, const string *charset) {
	#if cels_debug
		errors_panic("strings_check_charset.s", strings_check_extra(s));
		errors_panic("strings_check_charset.charset", strings_check_extra(charset));
	#endif

	for (size_t i = 0; i < s->size - 1; i++) {
		bool has_matched = false;

		for (size_t j = 0; j < charset->size - 1; j++) {
			if (s->data[i] == charset->data[j]) {
				has_matched = true;

				break;
			}
		}

		if (has_matched == false) return false;
	}

	return true;
}

string strings_make(const char *text) {
	#if cels_debug
		errors_panic("strings_make.text", text == NULL);
		errors_panic("strings_make.text == '\\0'", text[0] == '\0');
		//perhaps convert panic to warn
	#endif

	const size_t text_size = strlen(text) + 1;
	size_t s_capacity_new = maths_nearest_two_power(text_size);

	string s = strings_init(s_capacity_new);
	errors_panic("strings_make.s.data", s.data == NULL);
	//TODO?: convert panic to error return

	strncpy(s.data, text, text_size);
	s.size = text_size;

	return s;
}

string strings_make_copy(const string *s) {
	#if cels_debug
		errors_panic("strings_make_copy.s", strings_check(s));
	#endif

	string dest = strings_init(s->size);
	errors_panic("strings_make_copy.dest.data", dest.data == NULL);
	//TODO?: convert panic to error return

	strncpy(dest.data, s->data, s->size);
	dest.size = s->size;

	return dest;
}

void strings_free(string *s) {
	#if cels_debug
		errors_panic("strings_free.s", strings_check(s));
	#endif

	vectors_free((*s));
}

void strings_print(const string *s) {
	#if cels_debug
		errors_panic("strings_print.s", strings_check_extra(s));
	#endif

    for (size_t i = 0; i < s->size - 1; i++) {
        printf("%c", s->data[i]);
    }

	fflush(stdout);
}

void strings_println(const string *s) {
	#if cels_debug
		errors_panic("strings_println.s", strings_check_extra(s));
	#endif

	strings_print(s);
    printf("\n");
}


bool strings_compare(const string *first, const string *second) {
	#if cels_debug
		errors_panic("strings_compare.first", strings_check_extra(first));
		errors_panic("strings_compare.second", strings_check_extra(second));
	#endif

	size_t size = maths_max(first->size, second->size);

	for (size_t i = 0; i < size; i++) {
		long diff = tolower(first->data[i]) - tolower(second->data[i]);

		if (diff < 0) {
			return false;
		} else if (diff > 0) {
			return true;
		}
	}

	return false;
}

bool strings_equals(const string *first, const string *second) {
	#if cels_debug
		errors_panic("strings_equals.first", strings_check_extra(first));
		errors_panic("strings_equals.second", strings_check_extra(second));
	#endif

	if (first->size != second->size) {
		return false;
	}

	for (size_t i = 0; i < first->size - 1; i++) {
		long diff = first->data[i] - second->data[i];

		if (diff != 0) { return false; }
	}

	return true;
}

bool strings_seems(const string *first, const string *second) {
	#if cels_debug
		errors_panic("strings_seems.first", strings_check_extra(first));
		errors_panic("strings_seems.second", strings_check_extra(second));
	#endif

	if (first->size != second->size) {
		return false;
	}

	for (size_t i = 0; i < first->size - 1; i++) {
		long diff = tolower(first->data[i]) - tolower(second->data[i]);

		if (diff != 0) { return false; }
	}

	return true;
}

ssize_t strings_find(const string *s, const string *sep, size_t pos) {
	#if cels_debug
		errors_panic("strings_find.s", strings_check_extra(s));
		errors_panic("strings_find.sep", strings_check_extra(sep));
	#endif

	if (pos >= s->size - 1) {
		return -1;
	}

    for (size_t i = pos, j = 0; i < s->size - 1; i++) {
		char slower = tolower(s->data[i]);

        if (slower == tolower(sep->data[0])) {
            j = 1;
		} else if (tolower(s->data[i]) == tolower(sep->data[j])) {
            j++;
		} else {
            j = 0;
        }

        if (j == sep->size - 1) {
			return i - (j - 1);
        }
    }

	return -1;
}

size_vec strings_make_find(const string *s, const string *sep, size_t n) {
	#if cels_debug
		errors_panic("strings_make_find.s", strings_check_extra(s));
		errors_panic("strings_make_find.sep", strings_check_extra(sep));
	#endif

	size_vec indexes = vectors_init(sizeof(size_t), vectors_min);
	errors_panic("strings_make_find.indexes", vectors_check((vector *)&indexes));

    for (size_t i = 0, j = 0; i < s->size - 1; i++) {
		char slower = tolower(s->data[i]);

        if (slower == tolower(sep->data[0])) {
            j = 1;
		} else if (tolower(s->data[i]) == tolower(sep->data[j])) {
            j++;
		} else {
            j = 0;
        }

        if (j == sep->size - 1) {
			bool err = false;
			vectors_push(indexes, i - (j - 1), &err);
			#if cels_debug
				errors_panic("strings_make_find.vectors_push failed", err);
			#endif

			if (err == true) {
				break;
			}
        }

		if (n > 0 && indexes.size == n) {
			break;
		}
    }

    return indexes;
}

void strings_replace(string *s, const string *seps, const char rep, size_t n) {
	#if cels_debug
		errors_panic("strings_replace.s", strings_check_extra(s));
		errors_panic("strings_replace.seps", strings_check_extra(seps));
	#endif

	size_t n_rep = 0;
    for (size_t i = 0; i < s->size - 1; i++) {
		for (size_t j = 0; j < seps->size - 1; j++) {
			if (s->data[i] == seps->data[j]) {
				if (rep < 0) {
					s->data[i] = -1;
				} else {
					s->data[i] = rep;
				}

				n_rep++;

				if (n > 0 && n_rep == n) {
					goto replace;
				}

				break;
			} 
		}
    }

	replace:
	if (rep < 0 && n_rep > 0) {
		size_t offset = 0;
		for (size_t i = 0; i + offset < s->size - 1; i++) {
			if (offset > 0){
				s->data[i] = s->data[i + offset];
			}

			if (s->data[i] == -1) {
				while (s->data[i] == -1 && i + offset < s->size - 1) {
					offset++;
					s->data[i] = s->data[i + offset];
				}
			}
		}

		if (offset > 0) {
			for (size_t i = 0; i < offset; i++) {
				s->data[s->size - 2 - i] = '\0';
			}

			s->size -= offset;
		}
	}
}

string strings_make_replace(const string *s, const string *text, const string *rep, size_t n) {
	#if cels_debug
		errors_panic("strings_make_replace.s", strings_check_extra(s));
		errors_panic("strings_make_replace.text", strings_check_extra(text));
	#endif

	long diff_size = - (text->size - 1);
	if (rep != NULL) {
		#if cels_debug
			errors_panic("strings_make_replace.rep", strings_check_extra(rep));
		#endif

		diff_size += (rep->size - 1);
	}

	size_vec indexes = strings_make_find(s, text, n);
	if (indexes.size == 0) {
		vectors_free(indexes);
		return strings_make_copy(s);
	}

	size_t size = s->size + diff_size * indexes.size;
	size_t capacity = maths_nearest_two_power(size);
	string new_string = strings_init(capacity);
	errors_panic("strings_make_replace.new_string", new_string.data == NULL);

    new_string.size = size;
    size_t prev = 0, i = 0, j = 0;
    while (i < s->size - 1) {
		size_t offset = i + j * diff_size;
		size_t sentence_size = 0;

        if (j < indexes.size && indexes.data[j] == i) {
			if (rep != NULL) {
				strncpy(new_string.data + offset, rep->data, rep->size);
			}

            prev = indexes.data[j] + text->size - 1;
            i += text->size - 1;
            j += 1;
        } else {
            if (j > indexes.size - 1) {
                sentence_size = s->size - prev;
            } else {
                sentence_size = indexes.data[j] - prev;
            }

            strncpy(new_string.data + offset, s->data + prev, sentence_size);
            i += sentence_size;
        }
    }

	vectors_free(indexes);
    return new_string;
}

string_vec strings_make_split(const string *s, const string *sep, size_t n) {
	#if cels_debug
		errors_panic("strings_make_split.s", strings_check_extra(s));
		errors_panic("strings_make_split.sep", strings_check_extra(sep));
	#endif

	size_vec indexes = strings_make_find(s, sep, n);
    string_vec sentences = vectors_init(sizeof(string), vectors_min);

	if (indexes.size == 0) {
		string scopy = strings_make_copy(s);

		bool error = false;
		vectors_push(sentences, scopy, &error);
		#if cels_debug
			errors_panic("strings_make_split.vectors_push failed", error);
		#endif

		vectors_free(indexes);
		return sentences;
	}

    size_t prev = 0, size = 0, index = 0;
    for (size_t i = 0; i < indexes.size + 1; i++) {
        if (i > indexes.size - 1) {
            size = s->size - prev;
        } else {
			index = indexes.data[i];
            size = index - prev + 1;
        }

        if (size <= 1) {
            prev = index + (sep->size - 1);
            continue;
        }

        if (s->data[prev] == '\0') {
            continue;
        }

		size_t capacity = maths_nearest_two_power(size);
		string new_string = strings_init(capacity);
		errors_panic("strings_make_split.new_string", new_string.data == NULL);
        new_string.size = size;

		for (size_t j = 0; j < size; j++) {
			new_string.data[j] = (s->data + prev)[j];
		}

        new_string.data[size - 1] = '\0';

		bool error = false;
        vectors_push(sentences, new_string, &error);
		#if cels_debug
			errors_panic("strings_make_split.vectors_push failed", error);
		#endif

        if (error || i > indexes.size - 1) {
            break;
        }

        prev = index + (sep->size - 1);
    }

	vectors_free(indexes);
    return sentences;
}

string strings_make_format(const char *const form, ...) {
	#if cels_debug
		errors_panic("strings_make_format.form", form == NULL);
		errors_panic("strings_make_format.form < 1", strlen(form) < 1);
	#endif

    va_list args, args2;
    va_start(args, form);
    *args2 = *args;

    size_t buff_size = vsnprintf(NULL, 0, form, args) + 1;
	size_t new_capacity = maths_nearest_two_power(buff_size);
    char *text = calloc(new_capacity, sizeof(char));
    errors_panic("strings_make_format.text", text == NULL);

    vsnprintf(text, buff_size, form, args2);
    va_end(args);

    return (string){.data=text, .size=buff_size, .capacity=new_capacity};
}

size_t strings_hasherize(const string *s) {
	#if cels_debug
		errors_panic("strings_hasherize.s", strings_check_extra(s));
	#endif

	#define strings_hash 7

    size_t hash = 0;
    for (size_t i = 0; i < s->size - 1; i++) {
        unsigned char character = tolower((unsigned char)s->data[i]);

        size_t power = pow(strings_hash, i + 1);
        hash += character * power;
    }

    return hash;
}

void strings_lower(string *s) {
	#if cels_debug
		errors_panic("strings_lower.s", strings_check_extra(s));
	#endif

	for (size_t i = 0; i < s->size - 1; i++) {
		s->data[i] = tolower(s->data[i]);
	}
}

void strings_upper(string *s) {
	#if cels_debug
		errors_panic("strings_upper.s", strings_check_extra(s));
	#endif

	for (size_t i = 0; i < s->size - 1; i++) {
		s->data[i] = toupper(s->data[i]);
	}
}

bool strings_next(const string *s, const string *sep, string *next) {
	#if cels_debug
		errors_panic("strings_next.s", strings_check(s));
		errors_panic("strings_next.sep", strings_check(sep));
	#endif

	bool has_data_been_setted = false;
	if (next->data == NULL) {
		next->data = s->data;
		next->size = 2;
		has_data_been_setted = true;
	}
	
	#if cels_debug
		errors_panic("strings_next_token.(next < s)", next->data < s->data);
		errors_panic("strings_next_token.(next > s.size)", next->data > s->data + s->size);
	#endif

	do {
		size_t next_pos = next->data - s->data;
		if (next_pos >= s->size) return true;

		if (has_data_been_setted) {
			long new_size = 0;
			ssize_t new_pos = strings_find(s, sep, next_pos);
			if (new_pos == -1) {
				new_size = (s->data + s->size) - next->data;

				has_data_been_setted = true;
			} else {
				new_size = new_pos - next_pos + 1;
				has_data_been_setted = false;
			}

			if (new_size <= 0) return true;
			next->size = new_size;
			break;
		} else {
			next->data += sep->size;
			has_data_been_setted = true;
		}
	} while (true);

	return false;
}

bool string_vecs_equals(const string_vec *v0, const string_vec *v1) {
	#if cels_debug
		errors_panic("string_vecs_equals.v0", vectors_check((vector *)v0));
		errors_panic("string_vecs_equals.v1", vectors_check((vector *)v1));
	#endif

	if (v0->size != v1->size) return false;

	for (size_t i = 0; i < v0->size; i++) {
		if (!strings_equals(&v0->data[i], &v1->data[i])) return false;
	}

	return true;
}

bool string_vecs_seems(const string_vec *v0, const string_vec *v1) {
	#if cels_debug
		errors_panic("string_vecs_seems.v0", vectors_check((vector *)v0));
		errors_panic("string_vecs_seems.v1", vectors_check((vector *)v1));
	#endif

	if (v0->size != v1->size) return false;

	for (size_t i = 0; i < v0->size; i++) {
		if (!strings_seems(&v0->data[i], &v1->data[i])) return false;
	}

	return true;
}

void string_vecs_free(string_vec *sv) {
	//TODO: checks

	if (sv == NULL) return;

    for (size_t i = 0; i < sv->size; i++) {
		vectors_free(sv->data[i]);
	}

	free(sv->data);
	sv = NULL;
}

//extras

void *string_bnode_sets_free(bnode *n) {
	//TODO: checks
	
	bnodes_free(n, (cleanfunc)strings_free);

	return null;
}

void string_map_items_free(string_key_pair *ss) {
	//TODO: checks

	strings_free(&ss->key);
	strings_free(&ss->value);

	//free(ss);
}

void *string_bnode_maps_free(bnode *n) {
	//TODO: checks

	bnodes_free(n, (cleanfunc)string_map_items_free);

	return null;
}

string *string_maps_get_value(string_map *s, size_t hash) {
	string_map *node = (string_map *)bnodes_get((bnode *)s, hash);
	if (node == null) {
		return null;
	} 

	return &node->data.value;
}
