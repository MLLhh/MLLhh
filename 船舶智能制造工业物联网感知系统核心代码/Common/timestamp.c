#include "timestamp.h"
#include "Freertos.h"
#include "task.h"
#include "Network/network.h"

uint32_t init_timestamp = 0;    // ��ʼʱ���
TickType_t init_tickcount = 0;  // ��ʼtick

#define TICKS_TO_SEC(ticks)     ((ticks)/configTICK_RATE_HZ)

void set_boot_timestamp(uint32_t ts) // ���ó�ʼʱ���
{
    printf("���ó�ʼʱ���: %u\n", ts);
    init_timestamp = ts;
    init_tickcount = xTaskGetTickCount();
}

uint32_t get_timestamp(void) // ��ȡ��ǰʱ���
{
    TickType_t tick = xTaskGetTickCount();
    TickType_t past = tick - init_tickcount;
    uint32_t s = TICKS_TO_SEC(past);
    uint32_t timestamp = init_timestamp + s;
//    printf("��ȡ��ǰʱ���: %u\n", timestamp);
    return timestamp;
}

