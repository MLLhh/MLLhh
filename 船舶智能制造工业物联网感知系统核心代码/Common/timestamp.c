#include "timestamp.h"
#include "Freertos.h"
#include "task.h"
#include "Network/network.h"

uint32_t init_timestamp = 0;    // 初始时间戳
TickType_t init_tickcount = 0;  // 初始tick

#define TICKS_TO_SEC(ticks)     ((ticks)/configTICK_RATE_HZ)

void set_boot_timestamp(uint32_t ts) // 设置初始时间戳
{
    printf("设置初始时间戳: %u\n", ts);
    init_timestamp = ts;
    init_tickcount = xTaskGetTickCount();
}

uint32_t get_timestamp(void) // 获取当前时间戳
{
    TickType_t tick = xTaskGetTickCount();
    TickType_t past = tick - init_tickcount;
    uint32_t s = TICKS_TO_SEC(past);
    uint32_t timestamp = init_timestamp + s;
//    printf("获取当前时间戳: %u\n", timestamp);
    return timestamp;
}

