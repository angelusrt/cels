#include "csvs.h"


/* csvs */

string_mat csvs_unmake(
	const string *text, 
	const string column_sep, 
	const allocator *mem) {

	string_mat csv = {0}; 
	vectors_init(&csv, sizeof(string_vec), vector_min, mem);

	const string row_sep = strings_premake("\n");
	string_vec row = strings_split(text, row_sep, 0, mem);
	for (size_t i = 0; i < row.size; i++) {
		string_vec column = strings_split(&row.data[i], column_sep, 0, mem);
		vectors_push(&csv, &column, mem);
	}

	return csv;
}

bool csvs_split(
	string_view_vec *columns, 
	const string row, 
	const string column_sep, 
	const allocator *mem) {

	string_view column = {0};
	size_t k = 0;

	while (!strings_next(&row, column_sep, &column)) {
		if (columns->size <= k) {
			bool upscale_error = vectors_upscale(columns, mem);
			if (upscale_error) {
				return true;
			}
		}

		columns->data[k] = column;

		k++;
	}

	columns->size = k;
	return !column.data;
}

bool csvs_next(
	const string *csv, 
	string_view_vec *columns, 
	string_view *row, 
	const string column_sep, 
	const allocator *mem) {

	const string row_sep = strings_premake("\n");
	bool has_ended = strings_next(csv, row_sep, row);
	if (has_ended) { return true; }

	return csvs_split(columns, *row, column_sep, mem);
}
