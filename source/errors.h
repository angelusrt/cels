#ifndef cels_errors_h
#define cels_errors_h

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <execinfo.h>

#include "colors.h"
#include "types.h"

/*
 * The module 'errors' implement 
 * ways to check unexpected 
 * behaviours, assert and test; 
 * it also implements go-like 
 * error as values.
 */

typedef struct error_report {
	size_t successfull;
	size_t total;
} error_report;

typedef void (*reportfunc)(error_report *);

typedef enum errors_mode {
	errors_error_mode,
	errors_warn_mode,
	errors_success_mode,
	errors_none_mode
} errors_mode;

typedef int error;

#define ok 0
#define fail 1

#define errors(t) struct { int error; t value; }

/*
 * A convenience over errors_ensure, 
 * that returns if statement is true.
 *
 * #to-review
 */
#define errors_return(message, statement) \
	if (errors_ensure(message, statement)) { return true; }

/*
 * A convenience over errors_ensure_helper 
 * that provides function name.
 */
#define errors_ensure(message, statement) \
	errors_ensure_helper(__func__, message, statement)

/*
 * A convenience over errors_abort_helper 
 * that provides function name.
 */
#define errors_abort(message, statement) \
	errors_abort_helper(__func__, message, statement)

/*
 * A convenience over errors_inform_helper 
 * that provides function name.
 */
#define errors_inform(message, statement) \
	errors_inform_helper(__func__, message, statement)

/*
 * Tests if an assertion is true, 
 * returning error if it is.
 *
 * #to-review
 */
cels_warn_unused
error errors_assert(const char *message, bool statement);

/*
 * Tests if an assertion is true, 
 * adding the result to report
 *
 * #to-review
 */
void errors_expect(
	const char *message, 
	bool statement, 
	error_report *report);

/*
 * Panics if statement is true and 
 * prints message to terminal.
 *
 * Shouldn't be used - use errors_abort 
 * instead.
 *
 * #may-panic #depends:stdio.h #to-review
 */
void errors_abort_helper(
	const char *function_name, 
	const char *message, 
	bool statement);

/*
 * Panics if statement is true and 
 * prints message to terminal.
 *
 * #may-panic #depends:stdio.h #to-review
 */
void errors_panic(const char *message, bool statement);

/*
 * Warns to the terminal if statement holds.
 *
 * Shouldn't be used - use errors_inform_helper. 
 *
 * #depends:stdio.h #to-review
 */
error errors_inform_helper(
	const char* function_name, 
	const char *message, 
	bool statement);

/*
 * Warns to the terminal if statement holds.
 *
 * #depends:stdio.h #to-review
 */
error errors_warn(const char *message, bool statement);

/*
 * Checks statements and prints message 
 * with function_name if statement holds 
 * returning error.
 *
 * Shouldn't be used - use errors_ensure 
 * instead.
 *
 * #depends:stdio.h #to-review
 */
error errors_ensure_helper(
	const char *function_name, 
	const char *message, 
	bool statement);

/*
 * Checks statement, printing message if 
 * statement holds and returning error status.
 *
 * #depends:stdio.h #to-review
 */
error errors_check(const char *message, bool statement);

/*
 * Enables to see the stack of the program.
 *
 * #allocates-implicitly
 */
void errors_backtrace(void);

/*
 * An adapter to printf function to ease debug.
 *
 * #depends:stdio.h #to-review
 */
__attribute__ ((__format__ (printf, 2, 3)))
void errors_print(errors_mode mode, const char *const message, ...);

/*
 * Utility that prints the ammount of assertions that failed.
 *
 * #depends:stdio.h #to-review
 */
void error_reports_print(const error_report *e);

#endif
