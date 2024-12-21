#include "errors.h"

bool error_reports_check(const error_report *report) {
	bool is_bigger_than_total = report->successfull > report->total;

	#if cels_debug
		if (errors_check("error_reports_check.is_bigger_than_total", is_bigger_than_total)) {
			return true;
		}
	#else
		if (is_bigger_than_total) return true;
	#endif

	return false;
}

bool errors_assert(const char *message, bool statement){
	if (!statement) {
		printf(colors_error("'%s' failed!"), message);
		return false;
	}

	printf(colors_success("'%s' worked!"), message);
	return true;
}

void errors_panic(const char *message, bool statement) {
	if (statement) {
		printf(colors_error("%s"), message);
		exit(1);
	}
}

bool errors_check(const char *message, bool statement) {
	if (statement) {
		printf(colors_error("%s"), message);
		return true;
	}

	return false;
}

bool errors_warn(const char *message, bool statement) {
	if (statement) {
		printf(colors_warn("%s"), message);

		return true;
	}

	return false;
}

void errors_debug(const char *const message, errors_mode mode, ...) {
	#if cels_debug
		errors_panic("errors_debug.message", message == NULL);
		errors_panic("errors_debug.message < 1", strlen(message) < 1);
	#endif

    va_list args;
    va_start(args, mode);

	switch (mode) {
	case errors_error_mode:
		printf("\033[31m");
		break;
	case errors_warn_mode:
		printf("\033[33m");
		break;
	case errors_success_mode:
		printf("\033[32m");
		break;
	case errors_none_mode:
		break;
	}

    vprintf(message, args);

	switch (mode) {
	case errors_none_mode:
		printf("\n");
		break;
	default:
		printf("\033[0m\n");
	}

    va_end(args);
}

void errors_note(const char *const message, errors_mode mode, ...) {
	#if cels_debug
		errors_panic("errors_note.message", message == NULL);
		errors_panic("errors_note.message < 1", strlen(message) < 1);
	#endif

    va_list args;
    va_start(args, mode);

	switch (mode) {
	case errors_error_mode:
		printf("\033[31m");
		break;
	case errors_warn_mode:
		printf("\033[33m");
		break;
	case errors_success_mode:
		printf("\033[32m");
		break;
	case errors_none_mode:
		break;
	}

    vprintf(message, args);

	switch (mode) {
	case errors_none_mode:
		printf("\n");
		break;
	default:
		printf("\033[0m\n");
	}

    va_end(args);
}

void error_reports_print(const error_report *e) {
	#if cels_debug
		errors_panic("error_reports_print.e", error_reports_check(e));
	#endif
	
	printf("%zu successfull from %zu total\n\n", e->successfull, e->total);
}

