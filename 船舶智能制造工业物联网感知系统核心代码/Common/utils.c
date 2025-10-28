#include "utils.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"

void printhex(uint8_t *array, uint32_t len)
{
    if (len > 256)
    {
        printf("size = %u字节, 只显示前256字节\n", len);
        len = 256;
    }

    for (uint32_t i = 0; i < len; i++)
    {
        printf("%02X ", array[i]);
    }
    printf("\n");
}

void string_to_uint8_array(const char *str, uint8_t *array, size_t size)
{
    size_t str_len = strlen(str);

    if (str_len > size)
    {
        printf("数组长度不足以存储字符串\n");
        return;
    }

    for (size_t i = 0; i < str_len; i++)
    {
        array[i] = (uint8_t)str[i];
    }
}

void printstring(const char *str, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        printf("%c", str[i]);
    }
    printf("\n");
}

uint8_t get_bit(uint16_t val, int i) // 取出数的某一位
{
    uint16_t mask = 1u << i;
    uint8_t bit = (val & mask) >> i;
    return bit;
}

float get_float_val_32(uint8_t *array) // 将单精度浮点型大端转换为真实值
{
    float val = 0;
    uint32_t buf = (array[0] << 24) | (array[1] << 16) | (array[2] << 8) | (array[3]);
    memcpy(&val, &buf, sizeof(float));
    return val;
}

float get_float_val_16(uint8_t *array) // 将单精度浮点型大端转换为真实值
{
    float val = 0;
    uint16_t buf = (array[0] << 8) | (array[1]);
    memcpy(&val, &buf, sizeof(float));
    return val;
}

uint16_t get_uint_16(uint8_t *array)
{
    uint16_t val = (array[0] << 8) | (array[1]);
    return val;
}

int16_t get_int_16(uint8_t *array)
{
    int16_t val = (int16_t)(array[0] << 8) | (int16_t)(array[1]);
    return val;
}

// 将2字节有符号整数转为浮点值, 并缩小times倍数
float get_float_int16_times(uint8_t *array, uint8_t times)
{
    int16_t val_int = get_int_16(array);
    float val_float = ((float)val_int) / times;
    return val_float;
}

// 将2字节无符号整数转为浮点值, 并缩小times倍数
float get_float_uint16_times(uint8_t *array, uint8_t times)
{
    uint16_t val_uint = get_uint_16(array);
    float val_float = ((float)val_uint) / times;
    return val_float;
}

int gen_random(int min, int max)
{
    return (rand() % (max - min + 1)) + min;
}

int gen_random_3in1(void)
{
    return rand() % 3 - 1;
}
