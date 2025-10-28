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

extern STMFLASH_CACHE stmflash_cache; // flash����
uint32_t timestamp_last_feed = 0;     // �ϴ�ι��ʱ���
uint32_t timestamp_now = 0;           // ��ǰʱ���

// FreeRTOS�������ȼ� 0 < TASK_XXX_PRIO < configMAX_PRIORITIES-1
// ����Խ�� ���ȼ�Խ��
/************************************************************/
/* start ���� */
#define START_TASK_PRIO 1            // ���ȼ�
#define START_STK_SIZE 128           // ��ջ
TaskHandle_t StartTask_Handler;      // ���
void start_task(void *pvParameters); // ������

/************************************************************/
/* network ���� */
#define TASK_NETWORK_PRIO 3            // ���ȼ�
#define TASK_NETWORK_STACK_SIZE 256    // ��ջ
TaskHandle_t TaskNetwork_Handler;      // ���
void task_network(void *pvParameters); // ������

/************************************************************/
/* plc ����*/
#define TASK_PLC_PRIO 2            // ���ȼ�
#define TASK_PLC_STACK_SIZE 256    // ��ջ
TaskHandle_t TaskPlc_Handler;      // ���
void task_plc(void *pvParameters); // ������

/************************************************************/
/* sensor ���� */
#define TASK_SENSOR_PRIO 2         // ���ȼ�
#define TASK_SENSOR_STACK_SIZE 256 // ��ջ
TaskHandle_t TaskSensor_Handler;   // ���
void task_s(void *pvParameters);   // ������

/******************************************************************************************************/

void freertos_demo(void)
{
    printf("FreeRTOS ����\n");
    
    test();

    xTaskCreate((TaskFunction_t)start_task,          /* ������ */
                (const char *)"start_task",          /* �������� */
                (uint16_t)START_STK_SIZE,            /* �����ջ��С */
                (void *)NULL,                        /* ������������Ĳ��� */
                (UBaseType_t)START_TASK_PRIO,        /* �������ȼ� */
                (TaskHandle_t *)&StartTask_Handler); /* ������ */

    vTaskStartScheduler();
}

/**
 * @brief       start_task
 * @param       pvParameters : �������(δ�õ�)
 * @retval      ��
 */
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL(); /* �����ٽ��� */

    // network ����
    xTaskCreate((TaskFunction_t)task_network,          // ����
                (const char *)"task_network",          // ����
                (uint16_t)TASK_NETWORK_STACK_SIZE,     // ��ջ
                (void *)NULL,                          // ����
                (UBaseType_t)TASK_NETWORK_PRIO,        // ���ȼ�
                (TaskHandle_t *)&TaskNetwork_Handler); // ���

    // plc ����
    xTaskCreate((TaskFunction_t)task_plc,          // ����
                (const char *)"task_plc",          // ����
                (uint16_t)TASK_PLC_STACK_SIZE,     // ��ջ
                (void *)NULL,                      // ����
                (UBaseType_t)TASK_PLC_PRIO,        // ���ȼ�
                (TaskHandle_t *)&TaskPlc_Handler); // ���

    // sensor ����
    xTaskCreate((TaskFunction_t)task_s,
                (const char *)"task_s",
                (uint16_t)TASK_SENSOR_STACK_SIZE,
                (void *)NULL,
                (UBaseType_t)TASK_SENSOR_PRIO,
                (TaskHandle_t *)&TaskSensor_Handler);

    vTaskDelete(StartTask_Handler); /* ɾ����ʼ���� */
    taskEXIT_CRITICAL();            /* �˳��ٽ��� */
}

void task_network(void *pvParameters) // ��������
{
    vTaskDelay(1000);

    printf("Network ��������\n");

    network_init();

    while (1)
    {
        receive_net_msg(); // ����

        vTaskDelay(100);

        // ���Ź�
        timestamp_now = get_timestamp();
        if (timestamp_now - timestamp_last_feed >= WDG_TIMEOUT - 5)
        {
            // printf("ι�� timestamp = %u \n", timestamp_now);
            iwdg_feed();
            timestamp_last_feed = get_timestamp();
        }
    }
}

void task_plc(void *pvParameters)
{
    vTaskDelay(3000);

    printf("PLC�������� \n");

    TickType_t xLastWakeTime = xTaskGetTickCount();

    plc_init(); // ��ʼ��

    if (stmflash_cache.machine_type == 0x01) // ����
    {
        while (1)
        {
            read_plc_data(); // ����

            receive_plc_data(); // ����

            vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100)); // ÿ���100msѭ��
        }
    }
    else if (stmflash_cache.machine_type == 0x02) // �߿ճ�
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

    printf("Sensor ��������\n");

    TickType_t xLastWakeTime = xTaskGetTickCount();

    sensor_init(); // ��ʼ��

    while (1)
    {
        read_sensor_data(); // ����

        receive_sensor_data(); // ����

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100)); // ÿ���100msѭ��
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
