#ifndef cels_csvs_h
#define cels_csvs_h

#include "strings.h"
#include <sys/cdefs.h>


/*
 * The module 'csvs' deals with 
 * processing .csv files either 
 * buffered or at-once.
 */


/* csvs */

/*
 * Creates a matrix containing rows 
 * and columns of csv text.
 *
 * A column-separator must be provided.
 * Generally ',' or ';'.
 *
 * #allocates #to-review
 */
cels_warn_unused
string_mat csvs_unmake(
	const string *text, 
	const string column_sep, 
	const allocator *mem);

/*
 * Splits csv-row into columns and puts 
 * it into column_views -  a row_view must 
 * be provided for this.
 *
 * A column-separator must be provided.
 * Generally ',' or ';'.
 *
 * It upscales column_views automaticaly, 
 * if it is uncapable of fitting columns.
 *
 * #allocates #to-review
 */
bool csvs_split(
	string_view_vec *columns, 
	const string row, 
	const string column_sep, 
	const allocator *mem);

/*
 * Iterates through csv rows, 
 * updating row_view and puting 
 * columns into column_views. 
 *
 * A column-separator must be provided.
 * Generally ',' or ';'.
 *
 * It upscales column_views automaticaly, 
 * if it is uncapable of fitting columns.
 *
 * #allocates #to-review
 */
bool csvs_next(
	const string *csv, 
	string_view_vec *columns, 
	string_view *row, 
	const string column_sep, 
	const allocator *mem);

#endif
