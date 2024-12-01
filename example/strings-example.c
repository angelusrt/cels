#include "../source/strings.h"
#include "../source/strings.c"

#include "../source/errors.c"
#include "../source/vectors.c"

void strings_spliting_text() {
	string text = strings_make("exemplo de texto");

	const string sep = strings_premake(" ");
	string_vec text_tokens = strings_make_split(&text, &sep, 0);

	for (size_t i = 0; i < text_tokens.size; i++) {
		strings_println(&text_tokens.data[i]);
	}

	string_vecs_free(&text_tokens);
	strings_free(&text);
}

void strings_iterating_text() {
	string text = strings_make("exemplo de texto");
	string sep = strings_premake(" ");

	string text_current = {0};
	while (!strings_next(&text, &sep, &text_current)) {
		strings_println(&text_current);
	}

	strings_free(&text);
}

int main(void) {
	strings_spliting_text();
	strings_iterating_text();

	return 0;
}
