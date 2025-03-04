#include "errors.h"


/* errors */

error error_reports_check(const error_report *report) {
	bool is_bigger_than_total = report->successfull > report->total;

	#if cels_debug
		errors_return("is_bigger_than_total", is_bigger_than_total);
	#else
		if (is_bigger_than_total) return fail;
	#endif

	return ok;
}

void errors_expect(const char *message, bool statement, error_report *report) {
	if (statement) {
		printf("\033[32m✓ %s\033[0m\n", message);
		report->successfull++;
	} else {
		printf("\033[31m  %s\033[0m\n", message);
	}

	report->total++;
}

error errors_assert(const char *message, bool statement){
	if (!statement) {
		printf(colors_error("'%s' failed!"), message);
		return ok;
	}

	printf(colors_success("'%s' worked!"), message);
	return fail;
}

void errors_abort_helper(const char *function_name, const char *message, bool statement) {
	if (statement) {
		errors_backtrace();
		printf(colors_error("%s.%s"), function_name, message);
		exit(1);
	}
}

void errors_panic(const char *message, bool statement) {
	if (statement) {
		printf(colors_error("%s"), message);
		exit(1);
	}
}

error errors_ensure_helper(const char *function_name, const char *message, bool statement) {
	if (statement) {
		printf(colors_error("%s.%s"), function_name, message);
		return fail;
	}

	return ok;
}

error errors_check(const char *message, bool statement) {
	if (statement) {
		printf(colors_error("%s"), message);
		return fail;
	}

	return ok;
}

error errors_inform_helper(const char* function_name, const char *message, bool statement) {
	if (statement) {
		printf(colors_warn("%s.%s"), function_name, message);

		return fail;
	}

	return ok;
}

error errors_warn(const char *message, bool statement) {
	if (statement) {
		printf(colors_warn("%s"), message);

		return fail;
	}

	return ok;
}

void errors_backtrace(void) {
	void* callstack[128];
	int i, frames = backtrace(callstack, 128);

	char** symbols = backtrace_symbols(callstack, frames);
	if (!symbols) { return; }

	for (i = frames - 4; i >= 0; --i) {
		printf("%s\n", symbols[i]+2);
	}

	free(symbols);
}

void errors_print(errors_mode mode, const char *const message, ...) {
	#if cels_debug
		errors_abort("message", !message);
		errors_abort("message[0] == '\\0'", message[0] == '\0');
	#endif

    va_list args;
    va_start(args, message);

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


/* error_reports */

void error_reports_print(const error_report *self) {
	#if cels_debug
		errors_abort("self", error_reports_check(self));
	#endif
	
	printf("%zu successfull from %zu total\n\n", self->successfull, self->total);
}
