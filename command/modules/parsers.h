#ifndef parsers_h
#define parsers_h

#include "../packages/cels/strings.h"
#include "../packages/cels/files.h"

/*
 * Gets entities. 
 */
error parsers_get_entities(const char *path, const allocator *mem);

#endif
