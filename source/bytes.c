#include "bytes.h"


/* byte_vecs */

bool byte_vecs_check(const byte_vec *self) {
	#if cels_debug
		errors_return("self", vectors_check((const vector *)self))

		if (self->size > 0) {
			bool has_mismatch = self->data[self->size - 1] != '\0';

			errors_return("self.data[-1] mismatch", has_mismatch);
		}
	#else
		if (!vectors_check(self)) return true;
		if (self->size > self->capacity) return true;

		if (self->size > 0) {
			bool has_mismatch = self->data[self->size - 1] != '\0';
			if (has_mismatch) return true;
		}
	#endif

    return false;
}

void byte_vecs_print(const byte_vec *self) {
	for (size_t i = 0; i < self->size - 1; i++) {
		chars_print_normal(self->data[i]);
	}
}

void byte_vecs_free(byte_vec *self, const allocator *mem) {
	vectors_free(self, null, mem);
}

bool byte_vecs_is_string(byte_vec *self) {
	#if cels_debug
		errors_abort("self", byte_vecs_check(self));
	#endif

	if (!self || !self->data) {
		return false;
	}
	
	if (self->size < 2) {
		return false;
	}

	for (size_t i = 0; i < self->size - 1; i++) {
		bool is_charset_valid = 
			self->data[i] == 9 ||
			self->data[i] == 10 ||
			self->data[i] == 12 ||
			(self->data[i] >= 32 && self->data[i] <= 126);
		
		if (!is_charset_valid) {
			return false;
		}
	}

	return true;
}

ebyte_vec byte_vecs_receive(
	int socket_descriptor, int socket_flags, size_t max_size, const allocator *mem) {

	byte_vec response = {0};
	vectors_init(&response, sizeof(byte), string_small_size, mem);

    while(response.size < response.capacity) {
        long bytes = recv(
			socket_descriptor, 
			response.data + response.size, 
			response.capacity - response.size, 
			socket_flags);

		printf("bytes: %ld\n", bytes);

		if (bytes < 0) {
			goto cleanup0;
		} 

        response.size += bytes;

		if (max_size > 0 && response.size >= max_size) {
			break;
		}

        if (bytes == 0) {
            break;
		} else if (response.size >= response.capacity) {
			error upscale_error = vectors_upscale(&response, mem);
			if (upscale_error) { goto cleanup0; }
		}
    }

	error push_error = vectors_push(&response, &(byte){'\0'}, mem);
	if (push_error) { goto cleanup0; }

	#if cels_debug
		errors_abort("response", byte_vecs_check(&response));
	#endif

	return (ebyte_vec){.value=response};

	cleanup0:
	vectors_free(&response, null, mem);
	return (ebyte_vec){.error=fail};
}

size_vec byte_vecs_find_all(
	const byte_vec *self, 
	const byte_vec substring, 
	size_t n, 
	const allocator *mem) {

	#if cels_debug
		errors_abort("self", byte_vecs_check(self));
		errors_abort("substring", byte_vecs_check(&substring));
	#endif

	size_vec indexes = {0};
	error init_error = vectors_init(&indexes, sizeof(size_t), vector_min, mem);
	errors_abort("indexes", init_error);

    for (size_t i = 0, j = 0; i < self->size - 1; i++) {
		byte letter = tolower(self->data[i]);

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

byte_mat byte_vecs_split(
	const byte_vec *self, const byte_vec sep, size_t n, const allocator *mem) {

	#if cels_debug
		errors_abort("self", byte_vecs_check(self));
		errors_abort("sep", byte_vecs_check(&sep));
	#endif

	size_vec indexes = byte_vecs_find_all(self, sep, n, mem);

	if (indexes.size == 0) {
		byte_mat sentences = {0};
		error init_error = vectors_init(
			&sentences, sizeof(byte_vec), vector_min, mem);
		errors_abort("sentences failed", init_error);

		init_error = vectors_init(
			&sentences.data[0], sizeof(byte), self->capacity, mem);
		errors_abort("sentences.data failed", init_error);

		memcpy(sentences.data[0].data, self->data, self->size);
		sentences.data[0].size = self->size;
		sentences.size = 1;

		vectors_free(&indexes, null, mem);
		return sentences;
	}

	size_t new_size = maths_nearest_two_power(indexes.size);

    byte_mat sentences = {0};
	error init_error = vectors_init(&sentences, sizeof(byte_vec), new_size, mem);
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

		byte_vec new_byte_vec = {0};
		vectors_init(&new_byte_vec, sizeof(byte), size, mem);

        new_byte_vec.size = size;
		memcpy(new_byte_vec.data, self->data + prev, size);
        new_byte_vec.data[size - 1] = '\0';

        error push_error = vectors_push(&sentences, &new_byte_vec, mem);
		errors_abort("vectors_push failed", push_error);

        if (push_error || i > indexes.size - 1) {
            break;
        }

        prev = index + (sep.size - 1);
    }

	vectors_free(&indexes, null, mem);

	#if cels_debug
		errors_abort("sentences", vectors_check((const vector *)&sentences));
	#endif

    return sentences;
}
