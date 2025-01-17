#include "csvs.h"

string_bivec csvs_unmake(const string *text, const string column_sep, const allocator *mem) {
	string_bivec csv = string_bivecs_init(vector_min, mem);
	const string row_sep = strings_premake("\n");

	string_vec row = strings_split(text, &row_sep, 0, mem);
	for (size_t i = 0; i < row.size; i++) {
		string_vec column = strings_split(&row.data[i], &column_sep, 0, mem);
		string_bivecs_push(&csv, column, mem);
	}

	return csv;
}

bool csvs_split(string_vec *column_views, const string row_view, const string column_sep, const allocator *mem) {
	string column_view = {0};
	size_t k = 0;

	while (!strings_next(&row_view, &column_sep, &column_view)) {
		if (column_views->size <= k) {
			bool upscale_error = string_vecs_upscale(column_views, mem);
			if (upscale_error) {
				return true;
			}
		}

		column_views->data[k] = column_view;

		k++;
	}

	column_views->size = k;
	return column_view.data == null;
}

bool csvs_next(
	const string *csv, 
	string_vec *column_views, 
	string *row_view, 
	const string column_sep, 
	const allocator *mem
) {
	const string row_sep = strings_premake("\n");
	bool has_ended = strings_next(csv, &row_sep, row_view);
	if (has_ended) {
		return true;
	}

	return csvs_split(column_views, *row_view, column_sep, mem);
}

