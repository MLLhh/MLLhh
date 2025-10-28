#ifndef __UTILS_H
#define __UTILS_H

#include "./SYSTEM/sys/sys.h"

void printhex(uint8_t *array, uint32_t len);
void string_to_uint8_array(const char *str, uint8_t *array, size_t size);
void printstring(const char *str, size_t size);

#endif

