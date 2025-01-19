#ifndef cels_utils_h
#define cels_utils_h

#include "../../source/files.h"
#include "../../source/csvs.h"
#include "../../source/ios.h"

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

/**/
__attribute_warn_unused_result__
estring utils_get_main_file(const allocator *mem);

/**/
__attribute_warn_unused_result__
estring utils_get_flags(string main_file_name, const allocator *mem);

void configurations_free(configuration *configuration, const allocator *mem);

__attribute_warn_unused_result__
configuration utils_ask_configuration(void);

__attribute_warn_unused_result__
string utils_create_configuration(own configuration *configuration, const allocator *mem);

#endif
