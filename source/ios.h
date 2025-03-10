#ifndef cels_ios_h
#define cels_ios_h

#include <termios.h>
#include "strings.h"


/*
 * The module 'ios' deals with input/output
 * functionality like terminal mode, 
 * getting user input and printing to the screen.
 */


/* ios*/

/*
 * Turns terminal into canonical mode.
 *
 * #to-review
 */ 
void ios_canonical(void);

/*
 * Turns terminal into raw mode.
 *
 * #to-review
 */
void ios_raw(void);

/*
 * Asks user question, and allocates 
 * a string of 1024 size with response.
 *
 * #to-review
 */
string ios_ask(const char *question, const allocator *mem);

/*
 * Prompts user to select from options, 
 * thus returning the index of selected 
 * answer. 
 *
 * #to-review
 */
size_t ios_select(const char *question, const string_vec options);

#endif
