#include "strings.h"


/* chars */

void chars_print_private(const char *letter) {
	printf("%c\n", *letter);
}

void chars_print_normal_private(char *letter) {
	char let = *letter;
	if (let >= 33 && let <= 126) {
		printf("%c", let);
	} else {
		chars_print_special(let);
	}
}

bool chars_is_whitespace(char letter) {
	return letter == ' ' || 
		   letter == '\r' || 
		   letter == '\n' || 
		   letter == '\t';
}

bool chars_is_alphanumeric(char letter) {
    return isalnum((unsigned char)letter) || letter == '_';
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

void chars_print_normal(char letter) {
	if (letter >= 32 && letter <= 126) {
		printf("%c", letter);
	} else {
		chars_print_special(letter);
	}
}


/* strs */

bool strs_check(const char *self) {
	#if cels_debug
		errors_return("self", !self)
		errors_return("#self", self[0] == '\0')
	#else
		if (!self) return true;
		if (self[0] == '\0') return true;
	#endif

	return false;
}


/* byte_vecs */

estring byte_vecs_to_string(own byte_vec *self, const allocator *mem) {
	#if cels_debug
		errors_abort("self", vectors_check((const vector *)self));
	#endif

	if (!self || !self->data) {
		return (estring){.error=fail};
	}
	
	if (self->size < 2) {
		return (estring){.error=fail};
	}

	for (size_t i = 0; i < self->size; i++) {
		if (i == self->size - 2) {
			if (self->data[i] != '\0') {
				error push_error = vectors_push(self, &(char){'\0'}, mem);
				if (push_error) {
					return (estring){.error=fail};
				}
			}
		} else {
			if (self->data[i] < 32 && self->data[i] > 126) {
				return (estring){.error=fail};
			}
		}
	}

	string s = {
		.data=(char *)self->data, 
		.size=self->size, 
		.capacity=self->capacity
	};

	return (estring){.value=s};
}

bool byte_vecs_is_string(own byte_vec *self) {
	#if cels_debug
		errors_abort("self", vectors_check((const vector *)self));
	#endif

	if (!self || !self->data) {
		return false;
	}
	
	if (self->size < 2) {
		return false;
	}

	for (size_t i = 0; i < self->size; i++) {
		bool is_last = i == self->size - 2;
		bool is_charset_valid = self->data[i] < 32 && self->data[i] > 126;

		if (is_last && self->data[i] != '\0') {
			return false;
		} else if (!is_last && !is_charset_valid) {
			return false;
		}
	}

	return true;
}


/* string_views */

bool string_views_check(const string *self) {
	#if cels_debug
		errors_return("self", !self)
		errors_return("self.data", !self->data)
		errors_return("self.capacity < 1", self->capacity < 1)

		bool is_bigger = self->size > self->capacity;
		errors_return("self.(size > capacity)", is_bigger)

		if (self->size > 1) {
			bool has_mismatch = self->data[self->size - 2] == '\0';
			errors_return("self.data[-2] mismatch", has_mismatch)
		}
	#else 
		if (!self) return true;
		if (!self->data) return true;
		if (self->capacity < 1) return true;
		if (self->size > self->capacity) return true;

		if (self->size > 1) {
			bool has_mismatch = self->data[self->size - 2] == '\0';
			if (has_mismatch) return true;
		}
	#endif

	return false;
}

string string_views_to_string(const string_view *self, const allocator *mem) {
	#if cels_debug
		errors_abort("self", string_views_check(self));
	#endif

	string not_view = strings_init(self->capacity, mem);
	memcpy(not_view.data, self->data, self->size);

	not_view.size = self->size;
	not_view.data[not_view.capacity - 1] = '\0';

	#if cels_debug
		errors_abort("not_view", strings_check_extra(&not_view));
	#endif

	return not_view;
}


/* strings */

bool strings_check(const string *self) {
	#if cels_debug
		errors_return("self", !self)
		errors_return("self.data", !self->data)
		errors_return("self.capacity < 1", self->capacity < 1)

		bool is_bigger = self->size > self->capacity;
		errors_return("self.(size > capacity)", is_bigger)

		if (self->size > 0) {
			bool has_mismatch = self->data[self->size - 1] != '\0';

			errors_return(
				"self.data[-1] mismatch (may be string_view)", 
				has_mismatch);
		}

		if (self->size > 1) {
			bool has_mismatch = self->data[self->size - 2] == '\0';
			errors_return("self.data[-2] mismatch", has_mismatch)
		}
	#else 
		if (!self) return true;
		if (!self->data) return true;
		if (self->capacity < 1) return true;
		if (self->size > self->capacity) return true;

		if (self->size > 0) {
			bool has_mismatch = self->data[self->size - 1] != '\0';
			if (has_mismatch) return true;
		}

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
	string self = {0};
	self.capacity = quantity;
	self.data = mems_alloc(mem, sizeof(char) * self.capacity);
	errors_abort("self.data", !self.data);
	
	return self;
}

string strings_make(const char *text, const allocator *mem) {
	#if cels_debug
		errors_abort("text", strs_check(text));
	#endif

	size_t size = strlen(text) + 1;
	string self = strings_init(size, mem);

	memcpy(self.data, text, size);
	self.size = size;

	#if cels_debug
		errors_abort("self", strings_check_extra(&self));
	#endif

	return self;
}

string strings_copy(
	const string *self, size_t start, size_t end, const allocator *mem) {

	#if cels_debug
		errors_abort("self", strings_check_extra(self));
	#endif

	string_view self_view = strings_view(self, start, end);
	return string_views_to_string(&self_view, mem);
}

string strings_clone(const string *self, const allocator *mem) {
	#if cels_debug
		errors_abort("self", strings_check_extra(self));
	#endif

	string dest = strings_init(self->capacity, mem);
	errors_abort("dest.data", !dest.data);

	memcpy(dest.data, self->data, self->size);
	dest.size = self->size;

	#if cels_debug
		errors_abort("dest", strings_check_extra(&dest));
	#endif

	return dest;
}

string strings_encapsulate(const char *literal) {
	#if cels_debug
		errors_abort("literal", strs_check(literal));
	#endif

	size_t len = strlen(literal) + 1;
	return (string){.data=(char *)literal, .size=len, .capacity=len};
}

string_view strings_view(const string *self, size_t start, size_t end) {
	#if cels_debug
		errors_abort("self", strings_check_extra(self));
	#endif

	bool is_start_over = (ssize_t)start > (ssize_t)(self->size - 1);
	bool is_end_over = (ssize_t)end > (ssize_t)(self->size - 1);
	bool is_start_over_end = (ssize_t)start >= (ssize_t)end;

	if (is_start_over || is_end_over || is_start_over_end) {
		return strings_do("");
	}

	string_view view = {
		.data=self->data+start,
		.size=(end+1)-start,
		.capacity=(end+1)-start,
	};

	#if cels_debug
		errors_abort("view", string_views_check(&view));
	#endif

	return view;
}

error strings_pop(string *self, const allocator *mem) {
	#if cels_debug
		errors_abort("self", strings_check(self));
	#endif

	if (self->size <= 1) {
		return fail;
	}
	
	self->size--; 

	if (self->size < self->capacity >> 1) { 
		size_t new_capacity = self->capacity >> 1; 
		void *new_data = mems_realloc(
			mem, self->data, self->capacity, new_capacity); 
		
		#if cels_debug
			errors_inform("new_data", !new_data); 
		#endif
		
		if (!new_data) { 
			self->size++; 
			return fail; 
		} 
		
		self->capacity = new_capacity; 
		self->data = new_data; 
	} 
	
	self->data[self->size - 1] = '\0';

	return ok;
}

error strings_push(string *self, string item, const allocator *mem) {
	#if cels_debug
		errors_abort("self", strings_check(self));
		errors_abort("item", strings_check_extra(&item));
	#endif

	if (self->size > 0) {
		self->size--;
	}

	size_t final_size = self->size + item.size;
	while (final_size > self->capacity) {
		size_t new_capacity = self->capacity << 1; 
		
		#if cels_debug
			errors_abort(
				"new_capacity (overflow)", 
				self->capacity > new_capacity); 
		#endif 
		
		void *new_data = mems_realloc(
			mem, self->data, self->capacity, new_capacity);

		#if cels_debug
			errors_inform("new_data", !new_data); 
		#endif
		
		if (!new_data) { 
			self->size--; 
			return fail; 
		} 
		
		self->capacity = new_capacity; 
		self->data = new_data; 
	}

	memcpy(self->data + self->size, item.data, item.size);
	self->size = final_size;

	return ok;
}

error strings_push_with(string *self, char *item, const allocator *mem) {
	string item_capsule = strings_encapsulate(item);
	return strings_push(self, item_capsule, mem);
}

void strings_free(own string *self, const allocator *mem) {
	if (!self && !self->data) {
		mems_dealloc(mem, self->data, self->capacity * sizeof(string));
	}
}

void strings_normalize(string *self) {
	#if cels_debug
		errors_abort("self", strings_check_extra(self));
	#endif

	for (size_t i = 0; i < self->size - 1; i++) {
		if (self->data[i] == '\\') {
			strings_shift(self, i, 1);

			switch (self->data[i]) {
			case '\\':
				continue;
			case 't':
				self->data[i] = '\t';
				continue;
			case 'r':
				self->data[i] = '\r';
				continue;
			case 'n':
				self->data[i] = '\n';
				continue;
			}
		}
	}
}

void strings_debug(const string *self) {
	#if cels_debug
		errors_abort("self", strings_check(self));
	#endif

	printf(
		"<string>{.size: %zu, .capacity: %zu, .data: {", 
		self->size, self->capacity);

	for (size_t i = 0; (long)i < (long)self->size - 1; i++) {
		chars_print_normal(self->data[i]);
	}

	printf("}}");
}

void strings_debugln(const string *self) {
	#if cels_debug
		errors_abort("self", strings_check(self));
	#endif

	strings_debug(self);
	printf("\n");
}

void strings_print(const string *self) {
	#if cels_debug
		errors_abort("self", strings_check(self));
	#endif

    for (size_t i = 0; (long)i < (long)self->size - 1; i++) {
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

	for (size_t i = 0; (long)i < (long)self->size - 1; i++) {
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

ssize_t strings_find_with(
	const string *self, const char *substring, size_t pos) {

	string substring_capsule = strings_encapsulate(substring);
	return strings_find(self, substring_capsule, pos);
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

size_vec strings_find_all(
	const string *self, 
	const string substring, 
	size_t n, 
	const allocator *mem) {

	#if cels_debug
		errors_abort("self", strings_check_extra(self));
		errors_abort("substring", strings_check_extra(&substring));
	#endif

	size_vec indexes = {0};
	vectors_init(&indexes, sizeof(size_t), vector_min, mem);

	errors_abort("indexes", vectors_check((const vector *)&indexes));

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
			size_t index = i - (j - 1);
			error push_error = vectors_push(&indexes, &index, mem);
			errors_abort("vectors_push failed", push_error);
        }

		if (n > 0 && indexes.size == n) {
			break;
		}
    }

	#if cels_debug
		errors_abort("indexes", vectors_check((const vector *)&indexes));
	#endif

    return indexes;
}

ssize_t strings_find_matching(
	const string *self, 
	const string open_tag, 
	const string close_tag, 
	size_t pos) {

	#if cels_debug
		errors_abort("self", strings_check_extra(self));
		errors_abort("open_tag", strings_check_extra(&open_tag));
		errors_abort("close_tag", strings_check_extra(&close_tag));
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

void strings_replace_from(
	string *self, const string seps, const char rep, size_t n) {

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

string strings_replace(
	const string *self, 
	const string substring, 
	const string rep, 
	size_t n, 
	const allocator *mem) {

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
		vectors_free(&indexes, null, mem);
		return strings_clone(self, mem);
	}

	size_t size = self->size + diff_size * indexes.size;
	string new_string = strings_init(size, mem);
	errors_abort("new_string", !new_string.data);

    new_string.size = size;
    size_t prev = 0, i = 0, j = 0;
    while (i < self->size - 1) {
		size_t offset = i + j * diff_size;
		size_t sentence_size = 0;

        if (j < indexes.size && indexes.data[j] == i) {
			if (rep.size > 1) {
				memcpy(new_string.data + offset, rep.data, rep.size);
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

            memcpy(new_string.data + offset, self->data + prev, sentence_size);
            i += sentence_size;
        }
    }

	new_string.data[new_string.size - 1] = '\0';
	vectors_free(&indexes, null, mem);

	#if cels_debug
		errors_abort("new_string", strings_check(&new_string));
	#endif

    return new_string;
}

string strings_replace_with(
	const string *self, 
	const string substring, 
	const char *rep, 
	size_t n, 
	const allocator *mem) {

	string rep_capsule = strings_encapsulate(rep);
	return strings_replace(self, substring, rep_capsule, n, mem);
}


string_vec strings_split(
	const string *self, const string sep, size_t n, const allocator *mem) {

	#if cels_debug
		errors_abort("self", strings_check_extra(self));
		errors_abort("sep", strings_check_extra(&sep));
	#endif

	size_vec indexes = strings_find_all(self, sep, n, mem);

	if (indexes.size == 0) {
		string_vec sentences = {
			.size=1,
			.capacity=1,
			.data=mems_alloc(mem, sizeof(string)),
		};
		errors_abort("sentences failed", !sentences.data);

		string scopy = strings_clone(self, mem);
		sentences.data[0] = scopy;

		vectors_free(&indexes, null, mem);
		return sentences;
	}

	size_t new_size = maths_nearest_two_power(indexes.size);

    string_vec sentences = {0};
	error init_error = vectors_init(&sentences, sizeof(string), new_size, mem);
	errors_abort("vectors_init failed", init_error);

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

        error push_error = vectors_push(&sentences, &new_string, mem);
		errors_abort("vectors_push failed", push_error);

        if (push_error || i > indexes.size - 1) {
            break;
        }

        prev = index + (sep.size - 1);
    }

	vectors_free(&indexes, null, mem);

	#if cels_debug
		errors_abort("sentences", vectors_check((void *)&sentences));
	#endif

    return sentences;
}

string_vec strings_split_with(
	const string *self, const char *sep, size_t n, const allocator *mem) {

	string sep_capsule = strings_encapsulate(sep);
	return strings_split(self, sep_capsule, n, mem);
}

string strings_format(const char *const format, const allocator *mem, ...) {
	#if cels_debug
		errors_abort("format", !format);
		errors_abort("format == '\\0'", format[0] == '\0');
	#endif

    va_list args, args2;
    va_start(args, mem);
    *args2 = *args;

    size_t buff_size = vsnprintf(null, 0, format, args) + 1;
	size_t new_capacity = buff_size;

    char *text = mems_alloc(mem, sizeof(char) * new_capacity);
    errors_abort("text", !text);

    vsnprintf(text, buff_size, format, args2);
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

size_t strings_hash(const string *self) {
	#if cels_debug
		errors_abort("self", strings_check_view(self));
	#endif

	#define string_hash 3

    size_t hash = 0;
    for (size_t i = 0; i < self->size - 1; i++) {
        unsigned char character = tolower((unsigned char)self->data[i]);

        size_t power = pow(i + 1, string_hash);
        hash += character * power;
    }

	#undef string_hash

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

//TODO?: maybe convert string_view to string and allocate it
bool strings_next(const string *self, const string sep, string_view *next) {
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

		errors_abort(
			"(next > self.size)", 
			next->data > self->data + self->size);
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
		errors_abort("next", string_views_check(next));
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

void strings_shift(string *self, size_t position, size_t amount) {
	#if cels_debug 
		errors_abort("self", strings_check_extra(self));
	#endif

	if (position + amount >= self->size || amount == 0) {
		return;
	}

	for (size_t i = position; i < self->size - amount; i++) {
		self->data[i] = self->data[i + amount];
	}

	self->size -= amount;

	#if cels_debug 
		errors_abort("self", strings_check(self));
	#endif
}

void strings_trim(string *self) {
	#if cels_debug
		errors_abort("self", strings_check_extra(self));
	#endif

	size_t start = 0;
	size_t end = 0;

	for (size_t i = 0; i < self->size; i++) {
		if (!chars_is_whitespace(self->data[i])) {
			start = i;
			break;
		} 
	}

	for (ssize_t i = self->size - 2; i >= 0; i--) {
		if (!chars_is_whitespace(self->data[i])) {
			end = i + 1;
			break;
		} 
	}

	strings_slice(self, start, end);

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

		/*
		 * TODO?: oblige the usage of allocators and 
		 * put allocation errors within allocator
		 *
		 */

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

string_vec string_vecs_make_helper(
	char *args[], size_t argn, const allocator *mem) {

	#if cels_debug
		errors_abort("args", !args);
		errors_abort("argn", argn == 0);
	#endif

	string_vec self = {0};
	error init_error = vectors_init(&self, sizeof(string), vector_min, mem);
	errors_abort("vectors_init failed", init_error);

	for (size_t i = 0; i < argn; i++) {
		#if cels_debug
			errors_abort("args[i]", strs_check(args[i]));
		#endif

		string text = strings_make(args[i], mem);
		error push_error = vectors_push(&self, &text, mem);
		errors_abort("string_vecs_push failed", push_error);
	}

	#if cels_debug
		errors_abort("self", vectors_check((const vector *)&self));
	#endif

	return self;
}


/* sets */

string_set string_sets_init(void) {
	return (string_set){
		.node_size=sizeof(string_set_node), 
		.type_size=sizeof(string),
	};
}

bool string_sets_next(const string_set *self, string_set_iterator *it) {
	#if cels_debug
		errors_abort("self", bynodes_check((bynode *)&self->data));
	#endif

	return sets_next(self, it);
}

string *string_sets_get(const string_set *self, string item) {
	#if cels_debug
		errors_abort("self", bynodes_check((bynode *)&self->data));
		errors_abort("item", strings_check_extra(&item));
	#endif

	size_t hash = strings_hash(&item);
	return sets_get(self, hash);
}

error string_sets_push(string_set *self, string item, const allocator *mem) {
	#if cels_debug
		errors_abort("item", strings_check_extra(&item));
	#endif

	size_t hash = strings_hash(&item);
	return sets_push(self, &item, hash, mem);
}


/* maps */

string_map string_maps_init(void) {
	return (string_map){
		.node_size=sizeof(string_map_node), 
		.type_size=sizeof(string_map_pair),
		.extra_size=(
			(size_t)((char *)&((string_map_pair *)0)->value - 
			(char *)0))
	};
}

bool string_maps_next(const string_map *self, string_map_iterator *it) {
	#if cels_debug
		errors_abort("self", bynodes_check((bynode *)&self->data));
	#endif

	return maps_next(self, it);
}

string *string_maps_get(const string_map *self, string key) {
	#if cels_debug
		errors_abort("self", bynodes_check((bynode *)&self->data));
		errors_abort("key", strings_check_extra(&key));
	#endif

	size_t hash = strings_hash(&key);
	return maps_get(self, hash);
}

error string_maps_push(
	string_map *self, string key, string value, const allocator *mem) {

	#if cels_debug
		errors_abort("key", strings_check_extra(&key));
		errors_abort("value", strings_check_extra(&value));
	#endif
		
	string_map_pair pair = {.key=key, .value=value};

	size_t hash = strings_hash(&pair.key);
	return maps_push(self, &pair, hash, mem);
}

error string_maps_push_with(
	string_map *self, 
	const char *key, 
	const char *value, 
	const allocator *mem) {

	#if cels_debug
		errors_abort("key", strs_check(key));
		errors_abort("value", strs_check(value));
	#endif

	string k = strings_make(key, mem);
	string v = strings_make(value, mem);
	return string_maps_push(self, k, v, mem);
}
