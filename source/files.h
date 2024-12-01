#ifndef files_h
#define files_h
#pragma once

#include <sys/cdefs.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "errors.h"
#include "strings.h"
#include "vectors.h"

typedef FILE file;

__attribute_warn_unused_result__
string_with_error files_read(file *f);

#endif
