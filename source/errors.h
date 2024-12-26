#ifndef errors_h
#define errors_h

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

/* colors */

#define colors_error(text) "\033[31m" text "\033[0m\n"

#define colors_warn(text) "\033[33m" text "\033[0m\n"

#define colors_success(text) "\033[32m" text "\033[0m\n"

/* errors */

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

#define errors(t) struct { int error; t value; }

/*
 * Tests if an assertion is true, 
 * returning true if it is.
 */
__attribute_warn_unused_result__
bool errors_assert(const char *message, bool statement);

/*
 * Panics if statement is true and 
 * prints message to terminal.
 *
 * #may-panic #depends:stdio.h
 */
void errors_panic(const char *message, bool statement);

/*
 * Warns to the terminal if statement holds.
 *
 * #depends:stdio.h
 */
bool errors_warn(const char *message, bool statement);

/*
 * Checks statement, printing message if 
 * statement holds and returning true otherwise false.
 *
 * #depends:stdio.h
 */
bool errors_check(const char *message, bool statement);

/*
 * An adapter to printf function to ease debug.
 *
 * #depends:stdio.h
 */
__attribute__ ((__format__ (printf, 1, 3)))
void errors_debug(const char *const message, errors_mode mode, ...);

/*
 * An adapter to printf function to be used in production.
 *
 * #depends:stdio.h
 */
__attribute__ ((__format__ (printf, 1, 3)))
void errors_note(const char *const message, errors_mode mode, ...);

/*
 * Utility that prints the ammount of assertions that failed.
 *
 * #depends:stdio.h
 */
void error_reports_print(const error_report *e);

#endif
