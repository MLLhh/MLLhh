#include "utils.h"
#include "stdio.h"
#include "string.h"

void printhex(uint8_t *array, uint32_t len)
{
    if (len > 256)
    {
        printf("size = %u�ֽ�, ֻ��ʾǰ256�ֽ�\n", len);
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
        printf("���鳤�Ȳ����Դ洢�ַ���\n");
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

