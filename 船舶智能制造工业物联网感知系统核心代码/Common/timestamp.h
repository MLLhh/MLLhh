#ifndef __TIMESTAMP__H
#define __TIMESTAMP__H

#include "stdint.h"

void set_boot_timestamp(uint32_t ts); // 设置初始时间戳
uint32_t get_timestamp(void); // 读取当前时间戳

#endif
