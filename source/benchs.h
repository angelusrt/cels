#ifndef benchs_h
#define benchs_h
#pragma once

#include <stddef.h>
#include <stdio.h>
#include <time.h>

#include "vectors.h"

#define benchs_times 5

void benchs_measure(const char *name, const size_vec *size_buckets, benchfunc callback);

#endif
