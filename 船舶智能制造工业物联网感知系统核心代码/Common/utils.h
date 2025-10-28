#ifndef __UTILS_H
#define __UTILS_H

#include "./SYSTEM/sys/sys.h"

void printhex(uint8_t *array, uint32_t len);
void string_to_uint8_array(const char *str, uint8_t *array, size_t size);
void printstring(const char *str, size_t size);
uint8_t get_bit(uint16_t val_hex, int bit);
float get_float_val_32(uint8_t *array);
float get_float_val_16(uint8_t *array);
uint16_t get_uint_16(uint8_t *array);
int16_t get_int_16(uint8_t *array);
float get_float_int16_times(uint8_t *array, uint8_t times);
float get_float_uint16_times(uint8_t *array, uint8_t times);
int gen_random(int min, int max);
int gen_random_3in1(void);

#endif
