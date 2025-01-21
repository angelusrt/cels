#include "strings.h"
#include "errors.h"
#include "vectors.h"

/* char_vecs */

/* private */
void chars_print(const char *letter) {
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
	defaults_clone,
	chars_print, 
	defaults_compare, 
	defaults_seems, 
	defaults_free)

/* strings */

bool strings_check(const string *self) {
	#if cels_debug
		errors_return("self", vectors_check((void *)self))

		if (self->size > 0) {
			bool has_mismatch = self->data[self->size - 1] != '\0';
			errors_inform("self.data[-1] mismatch (may be string_view)", has_mismatch);
		}

		if (self->size > 1) {
			bool has_mismatch = self->data[self->size - 2] == '\0';
			errors_return("self.data[-2] mismatch", has_mismatch)
		}
	#else 
		if (vectors_check((void *)self)) return true;

		if (self->size > 1) {
			bool has_mismatch = self->data[self->size - 2] == '\0';
			if (has_mismatch) return true;
		}
	#endif

	return false;
}

bool strings_check_extra(const string *self) {
	#if cels_debug
		errors_return("self", strings_check((void *)self))
		errors_return("self.size <= 1", self->size <= 1)
		errors_return("self.data[0] == '\\0'", self->data[0] == '\0')
	#else
		if (strings_check(self)) return true;
		if (self->data[0] == '\0') return true;
		if (self->size <= 1) return true;
	#endif

	return false;
}

bool strings_check_charset(const string *self, const string charset) {
	#if cels_debug
		errors_abort("self", strings_check_extra(self));
		errors_abort("charset", strings_check_extra(&charset));
	#endif

	for (size_t i = 0; i < self->size - 1; i++) {
		bool has_matched = false;

		for (size_t j = 0; j < charset.size - 1; j++) {
			if (self->data[i] == charset.data[j]) {
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
		errors_abort("text", !text);
		errors_abort("text == '\\0'", text[0] == '\0');
	#endif

	size_t size = strlen(text) + 1;
	string self = char_vecs_init(size, mem);
	errors_abort("self.data", !self.data);

	strncpy(self.data, text, size);
	self.size = size;

	#if cels_debug
		errors_abort("self", strings_check_extra(&self));
	#endif

	return self;
}

string strings_clone(const string *self, const allocator *mem) {
	#if cels_debug
		errors_abort("self", strings_check(self));
	#endif

	string dest = char_vecs_init(self->capacity, mem);
	errors_abort("dest.data", !dest.data);

	memcpy(dest.data, self->data, self->size);
	dest.size = self->size;

	#if cels_debug
		errors_abort("dest", strings_check_extra(&dest));
	#endif

	return dest;
}

string strings_view(const string *self, size_t start, size_t end) {
	#if cels_debug
		errors_abort("self", strings_check(self));
	#endif

	bool is_start_over = (long)start > (long)(self->size - 1);
	bool is_end_over = (long)end > (long)(self->size - 1);
	bool is_start_over_end = (long)start >= (long)end;

	if (is_start_over || is_end_over || is_start_over_end) {
		return strings_do("");
	}

	string view = {
		.data=self->data+start,
		.size=(end+1)-start,
		.capacity=(end+1)-start,
	};

	#if cels_debug
		errors_abort("view", strings_check_extra(&view));
	#endif

	return view;
}

string strings_unview(const string *self, const allocator *mem) {
	#if cels_debug
		errors_abort("self", strings_check(self));
	#endif

	string view = *self;
	view.capacity++;

	string not_view = strings_clone(&view, mem);
	not_view.data[not_view.capacity - 1] = '\0';

	#if cels_debug
		errors_abort("not_view", strings_check_extra(&not_view));
	#endif

	return not_view;
}

error strings_pop(string *self, const allocator *mem) {
	#if cels_debug
		errors_abort("self", strings_check(self));
	#endif

	if (self->size <= 1) {
		return fail;
	}
	
	error pop_error = char_vecs_pop(self, mem);
	self->data[self->size - 1] = '\0';

	return pop_error;
}

/*TODO: optimizable*/
error strings_push(string *self, string item, const allocator *mem) {
	#if cels_debug
		errors_abort("self", strings_check(self));
		errors_abort("item", strings_check_extra(&item));
	#endif

	if (self->size > 0) {
		self->size--;
	}

	for (size_t i = 0; i < item.size - 1; i++) {
		error error = char_vecs_push(self, item.data[i], mem);
		if (error) {
			return fail;
		}
	}

	error error = char_vecs_push(self, '\0', mem);

	return error;
}

void strings_free(string *self, const allocator *mem) {
	#if cels_debug
		errors_abort("self", strings_check(self));
	#endif

	char_vecs_free(self, mem);
}

void strings_debug(const string *self) {
	#if cels_debug
		errors_abort("self", strings_check(self));
	#endif

	printf(
		"<string>{.size: %zu, .capacity: %zu, .data: %p}\n", 
		self->size, self->capacity, self->data);
}

void strings_print(const string *self) {
	#if cels_debug
		errors_abort("self", strings_check(self));
	#endif

    for (size_t i = 0; i < self->size - 1; i++) {
        printf("%c", self->data[i]);
    }

	fflush(stdout);
}

void strings_println(const string *self) {
	#if cels_debug
		errors_abort("self", strings_check(self));
	#endif

	strings_print(self);
    printf("\n");
}

void strings_imprint(const string *self) {
	#if cels_debug
		errors_abort("self", strings_check(self));
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

bool strings_compare(const string *self, const string *other) {
	#if cels_debug
		errors_abort("self", strings_check_extra(self));
		errors_abort("other", strings_check_extra(other));
	#endif

	size_t size = maths_max(self->size, other->size);

	for (size_t i = 0; i < size; i++) {
		long diff = tolower(self->data[i]) - tolower(other->data[i]);

		if (diff < 0) {
			return false;
		} else if (diff > 0) {
			return true;
		}
	}

	return false;
}

bool strings_equals(const string *self, const string *other) {
	#if cels_debug
		errors_abort("self", strings_check_extra(self));
		errors_abort("other", strings_check_extra(other));
	#endif

	if (self->size != other->size) {
		return false;
	}

	for (size_t i = 0; i < self->size - 1; i++) {
		long diff = self->data[i] - other->data[i];

		if (diff != 0) { return false; }
	}

	return true;
}

bool strings_seems(const string *self, const string *other) {
	#if cels_debug
		errors_abort("self", strings_check_extra(self));
		errors_abort("other", strings_check_extra(other));
	#endif

	if (self->size != other->size) {
		return false;
	}

	for (size_t i = 0; i < self->size - 1; i++) {
		long diff = tolower(self->data[i]) - tolower(other->data[i]);

		if (diff != 0) { return false; }
	}

	return true;
}

ssize_t strings_find(const string *self, const string substring, size_t pos) {
	#if cels_debug
		errors_abort("self", strings_check_extra(self));
		errors_abort("substring", strings_check_extra(&substring));
	#endif
	
	if (pos >= self->size - 1) {
		return -1;
	}

    for (size_t i = pos, j = 0; i < self->size - 1; i++) {
		char letter = tolower(self->data[i]);

        if (letter == tolower(substring.data[j])) {
            j++;
		} else if (letter == tolower(substring.data[0])) {
            j = 1;
		} else {
            j = 0;
        }

        if (j == substring.size - 1) {
			return i - (j - 1);
        }
    }

	return -1;
}

ssize_t strings_find_from(const string *self, const string seps, size_t pos) {
	#if cels_debug
		errors_abort("self", strings_check_extra(self));
		errors_abort("seps", strings_check_extra(&seps));
	#endif

	if (pos >= self->size - 1) {
		return -1;
	}

    for (size_t i = pos; i < self->size - 1; i++) {
		char letter = tolower(self->data[i]);

		for (size_t j = 0; j < self->size - 1; j++) {
			if (letter == tolower(seps.data[j])) {
				return i;
			}
		}
    }

	return -1;
}

size_vec strings_find_all(const string *self, const string substring, size_t n, const allocator *mem) {
	#if cels_debug
		errors_abort("self", strings_check_extra(self));
		errors_abort("substring", strings_check_extra(&substring));
	#endif

	size_vec indexes = size_vecs_init(vector_min, mem);
	errors_abort("indexes", vectors_check((vector *)&indexes));

    for (size_t i = 0, j = 0; i < self->size - 1; i++) {
		uchar letter = tolower(self->data[i]);

        if (letter == tolower(substring.data[j])) {
            j++;
		} else if (letter == tolower(substring.data[0])) {
            j = 1;
		} else {
            j = 0;
        }

        if (j == substring.size - 1) {
			error error = size_vecs_push(&indexes, i - (j - 1), mem);
			errors_abort("size_vecs_push failed", error);
        }

		if (n > 0 && indexes.size == n) {
			break;
		}
    }

	#if cels_debug
		errors_abort("indexes", vectors_check((vector *)&indexes));
	#endif

    return indexes;
}

ssize_t strings_find_matching(const string *self, const string open_tag, const string close_tag, size_t pos) {
	#if cels_debug
		errors_abort("strings_find_matching.self", strings_check_extra(self));
		errors_abort("strings_find_matching.open_tag", strings_check_extra(&open_tag));
		errors_abort("strings_find_matching.close_tag", strings_check_extra(&close_tag));
	#endif

	if ((long)pos > (long)(self->size - 2)) {
		return -1;
	}

	bool has_started = false;
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
			has_started = true;
		}

		//

		if (has_started) {
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

void strings_replace_from(string *self, const string seps, const char rep, size_t n) {
	#if cels_debug
		errors_abort("self", strings_check_extra(self));
		errors_abort("seps", strings_check_extra(&seps));
	#endif

	size_t n_rep = 0;
    for (size_t i = 0; i < self->size - 1; i++) {
		for (size_t j = 0; j < seps.size - 1; j++) {
			if (self->data[i] == seps.data[j]) {
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

	#if cels_debug
		errors_abort("self", strings_check(self));
	#endif
}

string strings_replace(const string *self, const string substring, const string rep, size_t n, const allocator *mem) {
	#if cels_debug
		errors_abort("self", strings_check_extra(self));
		errors_abort("substring", strings_check_extra(&substring));
	#endif

	long diff_size = - (substring.size - 1);
	if (rep.size > 1) {
		#if cels_debug
			errors_abort("rep", strings_check_extra(&rep));
		#endif

		diff_size += (rep.size - 1);
	}

	size_vec indexes = strings_find_all(self, substring, n, mem);
	if (indexes.size == 0) {
		size_vecs_free(&indexes, mem);
		return strings_clone(self, mem);
	}

	size_t size = self->size + diff_size * indexes.size;
	string new_string = char_vecs_init(size, mem);
	errors_abort("new_string", !new_string.data);

    new_string.size = size;
    size_t prev = 0, i = 0, j = 0;
    while (i < self->size - 1) {
		size_t offset = i + j * diff_size;
		size_t sentence_size = 0;

        if (j < indexes.size && indexes.data[j] == i) {
			if (rep.size > 1) {
				strncpy(new_string.data + offset, rep.data, rep.size);
			}

            prev = indexes.data[j] + substring.size - 1;
            i += substring.size - 1;
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

	new_string.data[new_string.size - 1] = '\0';
	size_vecs_free(&indexes, mem);

	#if cels_debug
		errors_abort("new_string", strings_check(&new_string));
	#endif

    return new_string;
}

string_vec strings_split(const string *self, const string sep, size_t n, const allocator *mem) {
	#if cels_debug
		errors_abort("self", strings_check_extra(self));
		errors_abort("sep", strings_check_extra(&sep));
	#endif

	size_vec indexes = strings_find_all(self, sep, n, mem);
    string_vec sentences = string_vecs_init(vector_min, mem);

	if (indexes.size == 0) {
		string scopy = strings_clone(self, mem);

		error error = string_vecs_push(&sentences, scopy, mem);
		errors_abort("string_vecs_push failed", error);

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
            prev = index + (sep.size - 1);
            continue;
        }

        if (self->data[prev] == '\0') {
            continue;
        }

		string new_string = strings_init(size, mem);
        new_string.size = size;

		for (size_t j = 0; j < size; j++) { 
			new_string.data[j] = (self->data + prev)[j];
		}

        new_string.data[size - 1] = '\0';

        error error = string_vecs_push(&sentences, new_string, mem);
		errors_abort("string_vecs_push failed", error);

        if (error || i > indexes.size - 1) {
            break;
        }

        prev = index + (sep.size - 1);
    }

	size_vecs_free(&indexes, mem);

	#if cels_debug
		errors_abort("sentences", vectors_check((void *)&sentences));
	#endif

    return sentences;
}

string strings_format(const char *const form, const allocator *mem, ...) {
	#if cels_debug
		errors_abort("form", !form);
		errors_abort("form == '\\0'", form[0] == '\0');
	#endif

    va_list args, args2;
    va_start(args, mem);
    *args2 = *args;

    size_t buff_size = vsnprintf(null, 0, form, args) + 1;
	size_t new_capacity = buff_size;

    char *text = mems_alloc(mem, sizeof(char) * new_capacity);
    errors_abort("text", !text);

    vsnprintf(text, buff_size, form, args2);
    va_end(args);

	string self = {
		.data=text, 
		.size=buff_size, 
		.capacity=new_capacity
	};

	#if cels_debug
		errors_abort("self", strings_check_extra(&self));
	#endif

    return self;
}

size_t strings_hasherize(const string *self) {
	#if cels_debug
		errors_abort("self", strings_check_extra(self));
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
		errors_abort("self", strings_check_extra(self));
	#endif

	for (size_t i = 0; i < self->size - 1; i++) {
		self->data[i] = tolower(self->data[i]);
	}
}

void strings_upper(string *self) {
	#if cels_debug
		errors_abort("self", strings_check_extra(self));
	#endif

	for (size_t i = 0; i < self->size - 1; i++) {
		self->data[i] = toupper(self->data[i]);
	}
}

bool strings_next(const string *self, const string sep, string *next) {
	#if cels_debug
		errors_abort("self", strings_check(self));
		errors_abort("sep", strings_check_extra(&sep));
	#endif

	bool has_data_been_setted = false;
	if (next->data == null) {
		next->data = self->data;
		next->size = 2;
		next->capacity = 2;
		has_data_been_setted = true;
	}
	
	#if cels_debug
		errors_abort("(next < self)", next->data < self->data);
		errors_abort("(next > self.size)", next->data > self->data + self->size);
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
			next->data += sep.size + next->size - 2;
			has_data_been_setted = true;
		}
	} while (true);

	#if cels_debug
		errors_abort("next", strings_check_extra(next));
	#endif

	return false;
}

void strings_slice(string *self, size_t start, size_t end) {
	#if cels_debug
		errors_abort("self", strings_check_extra(self));
	#endif

	bool is_start_over = (long)start > (long)(self->size - 1);
	bool is_end_over = (long)end > (long)(self->size - 1);
	bool is_start_over_end = (long)start >= (long)end;
	bool is_end_invalid = end < 1;

	if (is_start_over || is_end_over || is_start_over_end || is_end_invalid) {
		return;
	}

	for (size_t i = 0; i < end - 1; i++) {
		self->data[i] = self->data[i+start];
	}

	self->size = (end - start) + 1;
	self->data[self->size - 1] = '\0';

	#if cels_debug
		errors_abort("self", strings_check_extra(self));
	#endif
}

void strings_shift(string *self, size_t position) {
	#if cels_debug 
		errors_abort("self", strings_check_extra(self));
	#endif

	if (position + 1 >= self->size) {
		return;
	}

	for (size_t i = position; i < self->size - 1; i++) {
		self->data[i] = self->data[i + 1];
	}

	self->size--;

	#if cels_debug 
		errors_abort("self", strings_check_extra(self));
	#endif
}

/*TODO?:downsize*/
/*TODO?:recreate*/
void strings_trim(string *self) {
	#if cels_debug
		errors_abort("self", strings_check_extra(self));
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

	#if cels_debug
		errors_abort("self", strings_check(self));
	#endif
}

string strings_cut(const string *self) {
	#if cels_debug
		errors_abort("self", strings_check_extra(self));
	#endif

	size_t start = 0;
	for (size_t i = 0; i < self->size; i++) {
		if (!chars_is_whitespace(self->data[i])) {
			start = i;
			break;
		} 
	}

	size_t end = 0;
	for (ssize_t i = self->size - 2; i >= 0; i--) {
		if (!chars_is_whitespace(self->data[i])) {
			end = i + 1;
			break;
		} 
	}

	string view = {
		.data=self->data+start,
		.size=(end+1)-start,
		.capacity=(end+1)-start,
	};

	#if cels_debug
		errors_abort("view", strings_check(&view));
	#endif

	return view;
}

bool strings_has_suffix(const string *self, const string suffix) {
	#if cels_debug
		errors_abort("self", strings_check_extra(self));
		errors_abort("suffix", strings_check_extra(&suffix));
	#endif

	if (suffix.size > self->size) {
		return false;
	}

	size_t offset = self->size - suffix.size;
	for (size_t i = 0; i < suffix.size - 1; i++) {
		if (self->data[offset + i] != suffix.data[i]) {
			return false;
		}
	}

	return true;
}

bool strings_has_prefix(const string *self, const string prefix) {
	#if cels_debug
		errors_abort("self", strings_check_extra(self));
		errors_abort("prefix", strings_check_extra(&prefix));
	#endif

	if (prefix.size > self->size) {
		return false;
	}

	for (size_t i = 0; i < prefix.size - 1; i++) {
		if (self->data[i] != prefix.data[i]) {
			return false;
		}
	}

	return true;
}

/* string_vecs */

vectors_generate_implementation(
	string, 
	string_vec, 
	strings_check,
	strings_clone,
	strings_println, 
	strings_equals, 
	strings_seems, 
	strings_free)

string string_vecs_join(string_vec *self, string sep, const allocator *mem) {
	#if cels_debug
		errors_abort("self", vectors_check((vector *)self));
	#endif

	if (sep.size == 0) {
		sep.size++;
	}
	
	size_t capacity = 0;
	for (size_t i = 0; i < self->size; i++) {
		#if cels_debug
			errors_abort("self.data", strings_check_extra(&self->data[i]));
		#endif

		capacity += self->data[i].size - 1;
		capacity += sep.size - 1;
	}

	capacity -= sep.size - 1;
	capacity++;

	string joined = strings_init(capacity, mem);
	for (size_t i = 0; i < self->size; i++) {
		error push_error = strings_push(&joined, self->data[i], mem);
		errors_abort("string_push failed", push_error);

		if (i < self->size - 1) {
			error push_error = strings_push(&joined, sep, mem);
			errors_abort("string_push failed", push_error);
		}
	}

	#if cels_debug
		errors_abort("joined", strings_check(&joined));
	#endif

	return joined;
}

string_vec string_vecs_make_helper(char *args[], size_t argn, const allocator *mem) {
	#if cels_debug
		errors_abort("args", !args);
		errors_abort("argn", argn == 0);
	#endif

	string_vec self = string_vecs_init(vector_min, mem);

	for (size_t i = 0; i < argn; i++) {
		string text = strings_make(args[i], mem);
		error push_error = string_vecs_push(&self, text, mem);
		errors_abort("string_vecs_push failed", push_error);
	}

	#if cels_debug
		errors_abort("self", vectors_check((void *)&self));
	#endif

	return self;
}

bool string_vecs_check_private(const string_vec *self) {
	return vectors_check((const vector *)self);
}

void string_vecs_print_private(const string_vec *self) {
	#if cels_debug
		errors_abort("self", vectors_check((void *)self));
	#endif

	for (size_t i = 0; i < self->size; i++) {
		strings_imprint(&self->data[i]);
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
	string_vecs_clone,
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
		errors_abort("key", key == null);
		errors_abort("value", value == null);
		errors_abort("#key", strlen(key) <= 1);
		errors_abort("#value", strlen(value) <= 1);
	#endif

	string skey = strings_make(key, mem);
	string svalue = strings_make(value, mem);
	string_key_pair item = {
		.key = skey, 
		.value = svalue};

	string_map node = {
		.hash = strings_hasherize(&item.key),
		.data = item,
		.color = bnodes_black_color,
		.frequency = 1};

	string_map *new_bnode = mems_alloc(mem, sizeof(string_map));
	errors_abort("new_bnode", !new_bnode);

	*new_bnode = node;

	error push_error = bnodes_push((bnode **)self, (bnode *)new_bnode);
	if (push_error) { 
		string_maps_free_private(new_bnode, mem); 
	}

	return push_error;
}
