#include "../source/strings.h"
#include "../source/strings.c"

#include "../source/errors.c"
#include "../source/vectors.c"

void strings_spliting_text() {
	string text = strings_make("exemplo de texto", null);

	const string sep = strings_premake(" ");
	string_vec text_tokens = strings_make_split(&text, &sep, 0, null);

	for (size_t i = 0; i < text_tokens.size; i++) {
		strings_println(&text_tokens.data[i]);
	}

	string_vecs_free(&text_tokens, null);
	strings_free(&text, null);
}

void strings_iterating_text() {
	string text = strings_make("exemplo de texto", null);
	string sep = strings_premake(" ");

	string text_current = {0};
	while (!strings_next(&text, &sep, &text_current)) {
		strings_println(&text_current);
	}

	strings_free(&text, null);
}

int main(void) {
	strings_spliting_text();
	strings_iterating_text();

	return 0;
}
