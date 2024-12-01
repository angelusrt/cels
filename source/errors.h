#ifndef errors_h
#define errors_h

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

typedef struct error_report {
	size_t successfull;
	size_t total;
} error_report;

typedef enum errors_mode {
	errors_error_mode,
	errors_warn_mode,
	errors_success_mode,
	errors_none_mode
} errors_mode;

#define colors_error(text) "\033[31m" text "\033[0m\n"
#define colors_warn(text) "\033[33m" text "\033[0m\n"
#define colors_success(text) "\033[32m" text "\033[0m\n"

#define errors(t) struct { t value; int error; }

__attribute_warn_unused_result__
bool errors_assert(const char *message, bool statement);

void errors_panic(const char *message, bool statement);

bool errors_warn(const char *message, bool statement);

bool errors_check(const char *message, bool statement);

__attribute__ ((__format__ (printf, 1, 3)))
void errors_debug(const char *const message, errors_mode mode, ...);

__attribute__ ((__format__ (printf, 1, 3)))
void errors_note(const char *const message, errors_mode mode, ...);

void error_reports_print(const error_report *e);

#endif
