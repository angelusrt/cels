#include "strings.h"
#include "vectors.h"

/* char_vecs */

void chars_print_private(const char *letter) {
	printf("%c\n", *letter);
}

bool chars_is_whitespace(char letter) {
	return letter == ' ' || letter == '\r' || letter == '\n' || letter == '\t';
}

void chars_print_special(char letter) {
	switch(letter) {
		case '\n': printf("\\n"); break;
		case '\t': printf("\\t"); break;
		case '\r': printf("\\r"); break;
		case '\b': printf("\\b"); break;
		case '\a': printf("\\a"); break;
		case '\v': printf("\\v"); break;
		case '\f': printf("\\f"); break;
		case '\\': printf("\\\\"); break;
		case '\'': printf("\\'"); break;
		case '\"': printf("\\\""); break;
		default: printf("\\x%02X", letter); break;
	}
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
		if (errors_check("strings_check.self", vectors_check((const vector *)self))) return true;
		//if (errors_check("strings_check.self.size == 0", self->size == 0)) return true;
	#else 
		if (vectors_check((const vector *)self)) return true;
		//if (self->size == 0) return true;
	#endif

	return false;
}

bool strings_check_extra(const string *self) {
	#if cels_debug
		if (errors_check("strings_check_extra.self", strings_check(self))) return true;
		if (errors_check("strings_check_extra.self.size == 1", self->size == 1)) return true;
		if (errors_check("strings_check_extra.self.data[0] == '\\0'", self->data[0] == '\0')) {
			return true;
		}
	#else
		if (strings_check(self)) return true;
		if (self->data[0] == '\0') return true;
		if (self->size == 1) return true;
	#endif

	return false;
}

bool strings_check_charset(const string *self, const string *charset) {
	#if cels_debug
		errors_panic("strings_check_charset.self", strings_check_extra(self));
		errors_panic("strings_check_charset.charset", strings_check_extra(charset));
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
		errors_panic("strings_make.text", text == NULL);
		errors_panic("strings_make.text == '\\0'", text[0] == '\0');
	#endif

	const size_t text_size = strlen(text) + 1;
	size_t s_capacity_new = text_size;
	//size_t s_capacity_new = maths_nearest_two_power(text_size);

	string self = char_vecs_init(s_capacity_new, mem);
	errors_panic("strings_make.self.data", self.data == NULL);

	strncpy(self.data, text, text_size);
	self.size = text_size;

	return self;
}

string strings_make_copy(const string *self, const allocator *mem) {
	#if cels_debug
		errors_panic("strings_make_copy.self", strings_check(self));
	#endif

	string dest = char_vecs_init(self->capacity, mem);
	errors_panic("strings_make_copy.dest.data", dest.data == null);

	memcpy(dest.data, self->data, self->size);
	dest.size = self->size;

	return dest;
}

string strings_unview(const string *self, const allocator *mem) {
	string view = *self;
	view.capacity++;

	string not_view = strings_make_copy(&view, mem);
	not_view.data[not_view.capacity - 1] = '\0';

	return not_view;
}


bool strings_pop(string *self, const allocator *mem) {
	#if cels_debug
		errors_panic("strings_pop.self", strings_check(self));
	#endif

	bool error = char_vecs_pop(self, mem);
	if (!error) {
		self->data[self->size - 1] = '\0';
	}

	return error;
}

bool strings_push(string *self, string item, const allocator *mem) {
	#if cels_debug
		errors_panic("strings_push.self", strings_check(self));
		errors_panic("strings_push.item", strings_check(&item));
		//TODO: more checks
	#endif

	if (self->size > 0) {
		self->size--;
	}

	for (size_t i = 0; i < item.size - 1; i++) {
		bool error = char_vecs_push(self, item.data[i], mem);
		if (error) {
			return true;
		}
	}

	bool error = char_vecs_push(self, '\0', mem);

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
		errors_panic("strings_print.self", strings_check(self));
	#endif

    for (size_t i = 0; i < self->size - 1; i++) {
        printf("%c", self->data[i]);
    }

	fflush(stdout);
}

void strings_println(const string *self) {
	#if cels_debug
		errors_panic("strings_println.self", strings_check(self));
	#endif

	strings_print(self);
    printf("\n");
}

void strings_print_clean(const string *self) {
	#if cels_debug
		errors_panic("strings_print.self", strings_check(self));
	#endif

	for (size_t i = 0; i < self->size - 1; i++) {
		if (self->data[i] >= 32 && self->data[i] <= 126) {
			printf("%c", (unsigned char)self->data[i]);
		} else {
			chars_print_special(self->data[i]);
		}
	}

	fflush(stdout);
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

        if (slower == tolower(sep->data[j])) {
            j++;
		} else if (slower == tolower(sep->data[0])) {
            j = 1;
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

        if (slower == tolower(sep->data[j])) {
            j++;
		} else if (slower == tolower(sep->data[0])) {
            j = 1;
		} else {
            j = 0;
        }

        if (j == sep->size - 1) {
			bool err = size_vecs_push(&indexes, i - (j - 1), mem);
			#if cels_debug
				errors_panic("strings_make_find.size_vecs_push failed", err);
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

ssize_t strings_find_closing_tag(const string *self, const string open_tag, const string close_tag, size_t pos) {
	#if cels_debug
		errors_panic("strings_find_closing_tag.self", strings_check_extra(self));
		errors_panic("strings_find_closing_tag.open_tag", strings_check_extra(&open_tag));
		errors_panic("strings_find_closing_tag.close_tag", strings_check_extra(&close_tag));
	#endif

	if ((long)pos > (long)(self->size - 2)) {
		return -1;
	}

	bool has_startet = false;
	ssize_t count = 0;
	for (size_t i = pos, j = 0, k = 0; i < self->size; i++) {
		if (self->data[i] == open_tag.data[j]) {
			j++;
		} else {
			j = 0;
		}

		if (j == open_tag.size - 1) {
			j = 0;
			count++;
			has_startet = true;
		}

		//

		if (has_startet) {
			if (self->data[i] == close_tag.data[k]) {
				k++;
			} else {
				k = 0;
			}

			if (k == close_tag.size - 1) {
				k = 0;
				count--;
			}

			if (count == 0) {
				return i;
			}
		}
	}

	return -1;
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
	if (rep != null) {
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
	size_t capacity = size;
	//size_t capacity = maths_nearest_two_power(size);
	
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
			errors_panic("strings_make_split.string_vecs_push failed", error);
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

		size_t capacity = size;
		//size_t capacity = maths_nearest_two_power(size);
		
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
	size_t new_capacity = buff_size;

    char *text = mems_alloc(mem, sizeof(char) * new_capacity);
    errors_panic("strings_make_format.text", text == NULL);

    vsnprintf(text, buff_size, form, args2);
    va_end(args);

    return (string){.data=text, .size=buff_size, .capacity=new_capacity};
}

size_t strings_hasherize(const string *self) {
	#if cels_debug
		errors_panic("strings_hasherize.self", strings_check_extra(self));
	#endif

	#define strings_hash 3

    size_t hash = 0;
    for (size_t i = 0; i < self->size - 1; i++) {
        unsigned char character = tolower((unsigned char)self->data[i]);

        size_t power = pow(i + 1, strings_hash);
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
		next->capacity = 2;
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
			next->capacity = next->size;
			break;
		} else {
			next->data += sep->size + next->size - 2;
			has_data_been_setted = true;
		}
	} while (true);

	return false;
}

void strings_shift(string *self, size_t position) {
	#if cels_debug 
		errors_panic("strings_shift.self", strings_check_extra(self));
	#endif


	if (position + 1 >= self->size) {
		return;
	}

	for (size_t i = position; i < self->size - 1; i++) {
		self->data[i] = self->data[i + 1];
	}

	self->size--;
}

void strings_trim(string *self) {
	#if cels_debug
		errors_panic("strings_trim.self", strings_check_extra(self));
	#endif

	for (size_t i = 0; i < self->size; i++) {
		if (chars_is_whitespace(self->data[i])) {
			strings_shift(self, i);
			continue;
		} 
		
		break;
	}

	for (ssize_t i = self->size - 2; i >= 0; i--) {
		if (chars_is_whitespace(self->data[i])) {
			strings_shift(self, i);
			continue;
		} 
		
		break;
	}
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

string string_vecs_join(string_vec *self, string sep, const allocator *mem) {
	#if cels_debug
		errors_panic("string_vecs_join.self", vectors_check((vector *)self));
	#endif

	if (sep.size == 0) {
		sep.size++;
	}
	
	size_t capacity = 0;
	for (size_t i = 0; i < self->size; i++) {
		capacity += self->data[i].size - 1;
		capacity += sep.size - 1;
	}

	capacity -= sep.size - 1;
	capacity++;

	string joined = strings_init(capacity, mem);
	for (size_t i = 0; i < self->size; i++) {
		strings_push(&joined, self->data[i], mem);

		if (i < self->size - 1) {
			strings_push(&joined, sep, mem);
		}
	}

	return joined;
}

bool string_vecs_check_private(const string_vec *self) {
	return vectors_check((const vector *)self);
}

void string_vecs_print_private(const string_vec *self) {
	if (cels_debug) {
		errors_panic("string_vecs_print_private.self", vectors_check((const vector *)self));
	}

	for (size_t i = 0; i < self->size; i++) {
		strings_print_clean(&self->data[i]);
		if (i != self->size - 1) {
			printf(", ");
		}
	}

	printf("\n");
}

/* string_bivecs */

vectors_generate_implementation(
	string_vec, 
	string_bivec,
	string_vecs_check_private,
	string_vecs_print_private,
	string_vecs_equals,
	string_vecs_seems,
	string_vecs_free)

/* sets */

sets_generate_implementation(
	string, 
	string_set, 
	strings_check_extra, 
	strings_print,
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
	strings_print,
	strings_print,
	strings_hasherize,
	strings_free,
	strings_free)

bool string_maps_make_push(string_map **self, const char *key, const char *value, const allocator *mem) {
	#if cels_debug
		errors_panic("string_maps_make_push.key", key == null);
		errors_panic("string_maps_make_push.value", value == null);
		errors_panic("string_maps_make_push.#key", strlen(key) <= 1);
		errors_panic("string_maps_make_push.#value", strlen(value) <= 1);
	#endif


	string skey = strings_make(key, mem);
	string svalue = strings_make(value, mem);

	string_key_pair item = {.key = skey, .value = svalue};

	string_map node = {
		.hash = strings_hasherize(&item.key),
		.data = item,
		.color = bnodes_black_color,
		.frequency = 1};

	string_map *new_bnode = mems_alloc(mem, sizeof(string_map));
	errors_panic("string_maps_make_push.new_bnode", new_bnode == null);

	*new_bnode = node;

	bool push_error = bnodes_push((bnode **)self, (bnode *)new_bnode);
	if (push_error) { 
		string_maps_free_private(new_bnode, mem); 
	}

	return push_error;
}
