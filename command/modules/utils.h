#ifndef utils_h
#define utils_h

#include "../packages/cels/files.h"
#include "../packages/cels/csvs.h"
#include "../packages/cels/ios.h"

typedef enum {
	compiler_gcc,
	compiler_clang,
} compiler;

typedef struct {
	string name;
	string author;
	string flags;
	string main;
	compiler compiler;
} configuration;


/* utils */

/*
 * Finds file containing main function 
 * and returns its name.
 *
 * #to-review
 */
cels_warn_unused
estring utils_get_main_file(const allocator *mem);

/*
 * Gets all paths withing 'include' statements.
 *
 * #to-review
 */
cels_warn_unused
estring_vec utils_get_includes(const string path, const allocator *mem);

/*
 * Finds flags of packages used.
 *
 * #to-review
 */
cels_warn_unused
estring utils_get_flags(string main_file_name, const allocator *mem);

/*
 * Frees configuration.
 *
 * #to-review
 */
void configurations_free(configuration *configuration, const allocator *mem);

/*
 * Asks user for configuration information.
 *
 * #to-review
 */
cels_warn_unused
configuration utils_ask_configuration(const allocator *mem);

/*
 * Creates string from configuration object.
 *
 * #to-review
 */
cels_warn_unused
string utils_create_configuration(
	own configuration *configuration, const allocator *mem);

#endif
