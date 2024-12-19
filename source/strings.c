#include "strings.h"
#include "vectors.h"

/* char_vecs */
void chars_print_private(const char *letter) {
	printf("%c\n", *letter);
}

vectors_generate_implementation(
	char, 
	char_vec, 
	defaults_check,
	chars_print_private, 
	defaults_compare, 
	defaults_seems, 
	defaults_free)

/* strings */

bool strings_check(const string *self) {
	#if cels_debug
		if (errors_check(utils_fcat(".self"), vectors_check((const vector *)self))) return true;
		if (errors_check(utils_fcat(".self.size == 0"), self->size == 0)) return true;
	#else 
		if (vectors_check((const vector *)self)) return true;
		if (self->size == 0) return true;
	#endif

	return false;
}

bool strings_check_extra(const string *self) {
	#if cels_debug
		if (errors_check(utils_fcat(".self"), strings_check(self))) return true;
		if (errors_check(utils_fcat(".self.data[0] == '\\0'"), self->data[0] == '\0')) return true;
		if (errors_check(utils_fcat(".self.size == 1", self->size == 1))) return true;
	#else
		if (strings_check(self)) return true;
		if (self->data[0] == '\0') return true;
		if (self->size == 1) return true;
	#endif

	return false;
}

bool strings_check_charset(const string *self, const string *charset) {
	#if cels_debug
		errors_panic(utils_fcat(".self"), strings_check_extra(self));
		errors_panic(utils_fcat(".charset"), strings_check_extra(charset));
	#endif

	for (size_t i = 0; i < self->size - 1; i++) {
		bool has_matched = false;

		for (size_t j = 0; j < charset->size - 1; j++) {
			if (self->data[i] == charset->data[j]) {
				has_matched = true;

				break;
			}
		}

		if (has_matched == false) return false;
	}

	return true;
}

string strings_init(size_t quantity, const allocator *mem) {
	return char_vecs_init(quantity, mem);
}

string strings_make(const char *text, const allocator *mem) {
	#if cels_debug
		errors_panic(utils.fcat(".text"), text == NULL);
		errors_panic(utils.fcat(".text == '\\0'"), text[0] == '\0');
		//perhaps convert panic to warn
	#endif

	const size_t text_size = strlen(text) + 1;
	size_t s_capacity_new = maths_nearest_two_power(text_size);

	string self = char_vecs_init(s_capacity_new, mem);
	errors_panic("strings_make.self.data", self.data == NULL);
	//TODO?: convert panic to error return

	strncpy(self.data, text, text_size);
	self.size = text_size;

	return self;
}

string strings_make_copy(const string *self, const allocator *mem) {
	#if cels_debug
		errors_panic(utils_fcat(".self"), strings_check(self));
	#endif

	string dest = char_vecs_init(self->size, mem);
	errors_panic("strings_make_copy.dest.data", dest.data == NULL);

	strncpy(dest.data, self->data, self->size);
	dest.size = self->size;

	return dest;
}

bool strings_push(string *self, char item, const allocator *mem) {
	#if cels_debug
		errors_panic(utils_fcat(".self"), strings_check(self));
		//TODO: more checks
	#endif

	bool error = false;

	if (self->size == 0) {
		error = char_vecs_push(self, item, mem);
	} else {
		self->data[self->size - 1] = item;
	}

	error = char_vecs_push(self, '\0', mem);

	return error;
}

void strings_free(string *self, const allocator *mem) {
	#if cels_debug
		errors_panic("strings_free.self", strings_check(self));
	#endif

	char_vecs_free(self, mem);
}

void strings_debug(const string *self) {
	#if cels_debug
		errors_panic("strings_debug.self", strings_check(self));
	#endif

	printf(
		"<string>{.size: %zu, .capacity: %zu, .data: %p}\n", 
		self->size, self->capacity, self->data);
}

void strings_print(const string *self) {
	#if cels_debug
		errors_panic("strings_print.self", strings_check_extra(self));
	#endif

    for (size_t i = 0; i < self->size - 1; i++) {
        printf("%c", self->data[i]);
    }

	fflush(stdout);
}

void strings_println(const string *self) {
	#if cels_debug
		errors_panic("strings_println.self", strings_check_extra(self));
	#endif

	strings_print(self);
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

ssize_t strings_find(const string *self, const string *sep, size_t pos) {
	#if cels_debug
		errors_panic("strings_find.self", strings_check_extra(self));
		errors_panic("strings_find.sep", strings_check_extra(sep));
	#endif

	if (pos >= self->size - 1) {
		return -1;
	}

    for (size_t i = pos, j = 0; i < self->size - 1; i++) {
		char slower = tolower(self->data[i]);

        if (slower == tolower(sep->data[0])) {
            j = 1;
		} else if (tolower(self->data[i]) == tolower(sep->data[j])) {
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

size_vec strings_make_find(const string *self, const string *sep, size_t n, const allocator *mem) {
	#if cels_debug
		errors_panic("strings_make_find.self", strings_check_extra(self));
		errors_panic("strings_make_find.sep", strings_check_extra(sep));
	#endif

	size_vec indexes = size_vecs_init(vectors_min, mem);
	errors_panic("strings_make_find.indexes", vectors_check((vector *)&indexes));

    for (size_t i = 0, j = 0; i < self->size - 1; i++) {
		char slower = tolower(self->data[i]);

        if (slower == tolower(sep->data[0])) {
            j = 1;
		} else if (tolower(self->data[i]) == tolower(sep->data[j])) {
            j++;
		} else {
            j = 0;
        }

        if (j == sep->size - 1) {
			bool err = size_vecs_push(&indexes, i - (j - 1), mem);
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

void strings_replace(string *self, const string *seps, const char rep, size_t n) {
	#if cels_debug
		errors_panic("strings_replace.self", strings_check_extra(self));
		errors_panic("strings_replace.seps", strings_check_extra(seps));
	#endif

	size_t n_rep = 0;
    for (size_t i = 0; i < self->size - 1; i++) {
		for (size_t j = 0; j < seps->size - 1; j++) {
			if (self->data[i] == seps->data[j]) {
				if (rep < 0) {
					self->data[i] = -1;
				} else {
					self->data[i] = rep;
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
		for (size_t i = 0; i + offset < self->size - 1; i++) {
			if (offset > 0){
				self->data[i] = self->data[i + offset];
			}

			if (self->data[i] == -1) {
				while (self->data[i] == -1 && i + offset < self->size - 1) {
					offset++;
					self->data[i] = self->data[i + offset];
				}
			}
		}

		if (offset > 0) {
			for (size_t i = 0; i < offset; i++) {
				self->data[self->size - 2 - i] = '\0';
			}

			self->size -= offset;
		}
	}
}

string strings_make_replace(const string *self, const string *text, const string *rep, size_t n, const allocator *mem) {
	#if cels_debug
		errors_panic("strings_make_replace.self", strings_check_extra(self));
		errors_panic("strings_make_replace.text", strings_check_extra(text));
	#endif

	long diff_size = - (text->size - 1);
	if (rep != NULL) {
		#if cels_debug
			errors_panic("strings_make_replace.rep", strings_check_extra(rep));
		#endif

		diff_size += (rep->size - 1);
	}

	size_vec indexes = strings_make_find(self, text, n, mem);
	if (indexes.size == 0) {
		size_vecs_free(&indexes, mem);
		return strings_make_copy(self, mem);
	}

	size_t size = self->size + diff_size * indexes.size;
	size_t capacity = maths_nearest_two_power(size);
	string new_string = char_vecs_init(capacity, mem);
	errors_panic("strings_make_replace.new_string", new_string.data == NULL);

    new_string.size = size;
    size_t prev = 0, i = 0, j = 0;
    while (i < self->size - 1) {
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
                sentence_size = self->size - prev;
            } else {
                sentence_size = indexes.data[j] - prev;
            }

            strncpy(new_string.data + offset, self->data + prev, sentence_size);
            i += sentence_size;
        }
    }

	size_vecs_free(&indexes, mem);
    return new_string;
}

string_vec strings_make_split(const string *self, const string *sep, size_t n, const allocator *mem) {
	#if cels_debug
		errors_panic("strings_make_split.self", strings_check_extra(self));
		errors_panic("strings_make_split.sep", strings_check_extra(sep));
	#endif

	size_vec indexes = strings_make_find(self, sep, n, mem);
    string_vec sentences = string_vecs_init(vectors_min, mem);

	if (indexes.size == 0) {
		string scopy = strings_make_copy(self, mem);

		#if cels_debug
			bool error = string_vecs_push(&sentences, scopy, mem);
			errors_panic("strings_make_split.vectors_push failed", error);
		#else
			string_vecs_push(&sentences, scopy, mem);
		#endif

		size_vecs_free(&indexes, mem);
		return sentences;
	}

    size_t prev = 0, size = 0, index = 0;
    for (size_t i = 0; i < indexes.size + 1; i++) {
        if (i > indexes.size - 1) {
            size = self->size - prev;
        } else {
			index = indexes.data[i];
            size = index - prev + 1;
        }

        if (size <= 1) {
            prev = index + (sep->size - 1);
            continue;
        }

        if (self->data[prev] == '\0') {
            continue;
        }

		size_t capacity = maths_nearest_two_power(size);
		string new_string = char_vecs_init(capacity, mem);

		errors_panic("strings_make_split.new_string", new_string.data == NULL);
        new_string.size = size;

		for (size_t j = 0; j < size; j++) { new_string.data[j] = (self->data + prev)[j];
		}

        new_string.data[size - 1] = '\0';

        bool error = string_vecs_push(&sentences, new_string, mem);
		#if cels_debug
			errors_panic("strings_make_split.vectors_push failed", error);
		#endif

        if (error || i > indexes.size - 1) {
            break;
        }

        prev = index + (sep->size - 1);
    }

	size_vecs_free(&indexes, mem);
    return sentences;
}

string strings_make_format(const char *const form, const allocator *mem, ...) {
	#if cels_debug
		errors_panic("strings_make_format.form", form == NULL);
		errors_panic("strings_make_format.form < 1", strlen(form) < 1);
	#endif

    va_list args, args2;
    va_start(args, mem);
    *args2 = *args;

    size_t buff_size = vsnprintf(NULL, 0, form, args) + 1;
	size_t new_capacity = maths_nearest_two_power(buff_size);
    char *text = calloc(new_capacity, sizeof(char));
    errors_panic("strings_make_format.text", text == NULL);

    vsnprintf(text, buff_size, form, args2);
    va_end(args);

    return (string){.data=text, .size=buff_size, .capacity=new_capacity};
}

size_t strings_hasherize(const string *self) {
	#if cels_debug
		errors_panic("strings_hasherize.self", strings_check_extra(self));
	#endif

	#define strings_hash 7

    size_t hash = 0;
    for (size_t i = 0; i < self->size - 1; i++) {
        unsigned char character = tolower((unsigned char)self->data[i]);

        size_t power = pow(strings_hash, i + 1);
        hash += character * power;
    }

    return hash;
}

void strings_lower(string *self) {
	#if cels_debug
		errors_panic("strings_lower.self", strings_check_extra(self));
	#endif

	for (size_t i = 0; i < self->size - 1; i++) {
		self->data[i] = tolower(self->data[i]);
	}
}

void strings_upper(string *self) {
	#if cels_debug
		errors_panic("strings_upper.self", strings_check_extra(self));
	#endif

	for (size_t i = 0; i < self->size - 1; i++) {
		self->data[i] = toupper(self->data[i]);
	}
}

bool strings_next(const string *self, const string *sep, string *next) {
	#if cels_debug
		errors_panic("strings_next.self", strings_check(self));
		errors_panic("strings_next.sep", strings_check(sep));
	#endif

	bool has_data_been_setted = false;
	if (next->data == NULL) {
		next->data = self->data;
		next->size = 2;
		has_data_been_setted = true;
	}
	
	#if cels_debug
		errors_panic("strings_next_token.(next < self)", next->data < self->data);
		errors_panic("strings_next_token.(next > self.size)", next->data > self->data + self->size);
	#endif

	do {
		size_t next_pos = next->data - self->data;
		if (next_pos >= self->size) return true;

		if (has_data_been_setted) {
			long new_size = 0;
			ssize_t new_pos = strings_find(self, sep, next_pos);
			if (new_pos == -1) {
				new_size = (self->data + self->size) - next->data;

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

/* string_vecs */

vectors_generate_implementation(
	string, 
	string_vec, 
	strings_check,
	strings_println, 
	strings_equals, 
	strings_seems, 
	strings_free)

/* sets */

sets_generate_implementation(
	string, 
	string_set, 
	strings_check_extra, 
	strings_hasherize, 
	strings_free)

/* maps */

maps_generate_implementation(
	string, 
	string, 
	string_key_pair, 
	string_map, 
	strings_check_extra, 
	strings_check_extra, 
	strings_hasherize, 
	strings_free, 
	strings_free)

bool string_maps_make_push(string_map **self, const char *key, const char *value, const allocator *mem) {
	string skey = strings_make(key, mem);
	string svalue = strings_make(value, mem);

	string_key_pair item = {.key = skey, .value = svalue};

	string_map node = {
		.hash = strings_hasherize(&item.key),
		.data = item,
		.color = bnodes_black_color,
		.frequency = 1};

	string_map *new_bnode = mems_alloc(mem, sizeof(typeof(*new_bnode)));
	errors_panic("bnodes_make.self", new_bnode == ((void *)0));

	*new_bnode = node;

	bool error = bnodes_push((bnode **)self, (bnode *)new_bnode);
	if (error) { string_maps_free_private(new_bnode, mem); }

	return error;
}
