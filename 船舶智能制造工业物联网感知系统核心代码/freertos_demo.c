//
#include "freertos_demo.h"
#include "BSP/KEY/key.h"
#include "BSP/RS485/rs485.h"
#include "FreeRTOS.h"
#include "task.h"
#include "string.h"
#include "user_task_config.h"
#include "Network/network.h"
#include "BSP/STMFLASH/stmflash.h"
#include "PLC/plc.h"
#include "BSP/WDG/wdg.h"
#include "Sensor/sensor.h"
#include "Common/timestamp.h"

#include "Common/utils.h"
/******************************************************************************************************/

extern STMFLASH_CACHE stmflash_cache; // flash缓存
uint32_t timestamp_last_feed = 0;     // 上次喂狗时间戳
uint32_t timestamp_now = 0;           // 当前时间戳

// FreeRTOS任务优先级 0 < TASK_XXX_PRIO < configMAX_PRIORITIES-1
// 数字越大 优先级越高
/************************************************************/
/* start 任务 */
#define START_TASK_PRIO 1            // 优先级
#define START_STK_SIZE 128           // 堆栈
TaskHandle_t StartTask_Handler;      // 句柄
void start_task(void *pvParameters); // 任务函数

/************************************************************/
/* network 任务 */
#define TASK_NETWORK_PRIO 3            // 优先级
#define TASK_NETWORK_STACK_SIZE 256    // 堆栈
TaskHandle_t TaskNetwork_Handler;      // 句柄
void task_network(void *pvParameters); // 任务函数

/************************************************************/
/* plc 任务*/
#define TASK_PLC_PRIO 2            // 优先级
#define TASK_PLC_STACK_SIZE 256    // 堆栈
TaskHandle_t TaskPlc_Handler;      // 句柄
void task_plc(void *pvParameters); // 任务函数

/************************************************************/
/* sensor 任务 */
#define TASK_SENSOR_PRIO 2         // 优先级
#define TASK_SENSOR_STACK_SIZE 256 // 堆栈
TaskHandle_t TaskSensor_Handler;   // 句柄
void task_s(void *pvParameters);   // 任务函数

/******************************************************************************************************/

void freertos_demo(void)
{
    printf("FreeRTOS 启动\n");
    
    test();

    xTaskCreate((TaskFunction_t)start_task,          /* 任务函数 */
                (const char *)"start_task",          /* 任务名称 */
                (uint16_t)START_STK_SIZE,            /* 任务堆栈大小 */
                (void *)NULL,                        /* 传入给任务函数的参数 */
                (UBaseType_t)START_TASK_PRIO,        /* 任务优先级 */
                (TaskHandle_t *)&StartTask_Handler); /* 任务句柄 */

    vTaskStartScheduler();
}

/**
 * @brief       start_task
 * @param       pvParameters : 传入参数(未用到)
 * @retval      无
 */
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL(); /* 进入临界区 */

    // network 任务
    xTaskCreate((TaskFunction_t)task_network,          // 函数
                (const char *)"task_network",          // 名称
                (uint16_t)TASK_NETWORK_STACK_SIZE,     // 堆栈
                (void *)NULL,                          // 参数
                (UBaseType_t)TASK_NETWORK_PRIO,        // 优先级
                (TaskHandle_t *)&TaskNetwork_Handler); // 句柄

    // plc 任务
    xTaskCreate((TaskFunction_t)task_plc,          // 函数
                (const char *)"task_plc",          // 名称
                (uint16_t)TASK_PLC_STACK_SIZE,     // 堆栈
                (void *)NULL,                      // 参数
                (UBaseType_t)TASK_PLC_PRIO,        // 优先级
                (TaskHandle_t *)&TaskPlc_Handler); // 句柄

    // sensor 任务
    xTaskCreate((TaskFunction_t)task_s,
                (const char *)"task_s",
                (uint16_t)TASK_SENSOR_STACK_SIZE,
                (void *)NULL,
                (UBaseType_t)TASK_SENSOR_PRIO,
                (TaskHandle_t *)&TaskSensor_Handler);

    vTaskDelete(StartTask_Handler); /* 删除开始任务 */
    taskEXIT_CRITICAL();            /* 退出临界区 */
}

void task_network(void *pvParameters) // 网络任务
{
    vTaskDelay(1000);

    printf("Network 任务启动\n");

    network_init();

    while (1)
    {
        receive_net_msg(); // 接收

        vTaskDelay(100);

        // 看门狗
        timestamp_now = get_timestamp();
        if (timestamp_now - timestamp_last_feed >= WDG_TIMEOUT - 5)
        {
            // printf("喂狗 timestamp = %u \n", timestamp_now);
            iwdg_feed();
            timestamp_last_feed = get_timestamp();
        }
    }
}

void task_plc(void *pvParameters)
{
    vTaskDelay(3000);

    printf("PLC任务启动 \n");

    TickType_t xLastWakeTime = xTaskGetTickCount();

    plc_init(); // 初始化

    if (stmflash_cache.machine_type == 0x01) // 吊机
    {
        while (1)
        {
            read_plc_data(); // 发送

            receive_plc_data(); // 接收

            vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100)); // 每间隔100ms循环
        }
    }
    else if (stmflash_cache.machine_type == 0x02) // 高空车
    {
        while (1)
        {
            vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10000));
        }
    }
}

void task_s(void *pvParameters)
{
    vTaskDelay(2000);

    printf("Sensor 任务启动\n");

    TickType_t xLastWakeTime = xTaskGetTickCount();

    sensor_init(); // 初始化

    while (1)
    {
        read_sensor_data(); // 发送

        receive_sensor_data(); // 接收

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100)); // 每间隔100ms循环
    }
}


void test(void)
{
//    for (int i = 0; i < 20; i++)
//    {
//        int rand = gen_random(-1, 1);
//        printf(" %d ", rand);
//    }
//    printf("\n");
}
