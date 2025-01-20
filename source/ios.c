#include "ios.h"

struct termios orig_termios;

void ios_canonical(void) {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void ios_raw(void) {
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(ios_canonical);

  struct termios raw = orig_termios;
  raw.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

string ios_ask(const char *question, const allocator *mem) {
	printf(colors_success("%s"), question);
	printf("\033[33m> \033[0m");

	string answer = strings_init(string_small_size, mem);

	fgets(answer.data, answer.capacity, stdin);
	size_t bytes = strlen(answer.data);
	answer.size = bytes;

	if (answer.size > 0) {
		answer.data[answer.size - 1] = '\0';
	}

	return answer;
}

void ios_select_private(const string_vec options, size_t cursor) {
	for (size_t i = 0; i < options.size; i++) {
		if (i == cursor) {
			printf("\033[33m> \033[0m");
			printf("%s\n", options.data[i].data);
		} else {
			printf("  %s\n", options.data[i].data);
		}
	}
}

size_t ios_select(const char *question, const string_vec options) {
	#if cels_debug
		errors_abort("question", vectors_check((void *)&options));
		errors_abort("#question", strlen(question) == 0);
		errors_abort("#options", options.size == 0);
	#endif

	printf(colors_success("%s"), question);

	ios_raw();

	size_t cursor = 0;
	char current = 0;
	bool is_ansi = false;

	ios_select_private(options, cursor);
	while (true) {
		current = fgetc(stdin);

		fallthrough:
		switch (current) {
		case '\033':
			fgetc(stdin);

			current = fgetc(stdin);
			is_ansi = true;
			goto fallthrough;
		break;
		case 'B':
			if (!is_ansi) {
				break;
			}
			// fall through
		case 'j':
			if (cursor + 1 < options.size) {
				cursor++;
				printf("\033[%zuA", options.size);
				printf("\033[0J");
				fflush(stdout);
				ios_select_private(options, cursor);
			}
		break;
		case 'A':
			if (!is_ansi) {
				break;
			}
			// fall through
		case 'k':
			if (cursor > 0) {
				cursor--;
				printf("\033[%zuA", options.size);
				printf("\033[0J");
				fflush(stdout);
				ios_select_private(options, cursor);
			}
		break;
		case '\n':
			goto end;
		break;
		}

		is_ansi = false;
	}

	end:
	ios_canonical();
	return cursor;
}
