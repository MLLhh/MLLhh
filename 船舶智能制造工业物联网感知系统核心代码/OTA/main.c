#include "SYSTEM/sys/sys.h"
#include "SYSTEM/usart/usart.h"
#include "SYSTEM/delay/delay.h"
#include "BSP/LED/led.h"
#include "BSP/KEY/key.h"
#include "BSP/SDRAM/sdram.h"
#include "BSP/STMFLASH/stmflash.h"
#include "IAP/iap.h"
#include "IAP/ota.h"
#include "string.h"
#include "BSP/RS485/rs485.h"

extern uint32_t g_usart_rx_cnt;

extern STMFLASH_CACHE stmflash_cache; // flash����

extern uint8_t flag_connected; // TCP���ӳɹ���־(ota.c)
extern uint8_t flag_update_app; // �յ�����app

int main(void)
{
    HAL_Init();                                                   /* ��ʼ��HAL�� */
    sys_stm32_clock_init(360, 25, 2, 8);                          /* ����ʱ��,180Mhz */
    delay_init(180);                                              /* ��ʱ��ʼ�� */
    usart_init(115200);                                           /* ���ڳ�ʼ��Ϊ115200 */
    led_init();                                                   /* ��ʼ��LED */
    key_init();                                                   /* ��ʼ������ */
    sdram_init();                                                 /* ��ʼ��SDRAM */
    rs485_init(115200);                                           /* ��ʼ��485�ӿ� */
    
    printf("bootloader��������\n");
    
    flash_read_cache(); // �����ػ���

    printf("cache.machine_type = %u\n", stmflash_cache.machine_type);
    printf("cache.machine_id = %u\n", stmflash_cache.machine_id);
    printf("cache.node_id = %u\n", stmflash_cache.node_id);
    printf("cache.version_id = %u\n", stmflash_cache.version_id);
    printf("cache.need_update = %u\n", stmflash_cache.need_update);
    printf("cache.read_cycle = %u\n", stmflash_cache.read_cycle);
    printf("cache.plc_cycle = %u\n", stmflash_cache.plc_cycle);
    
    printf("4Gģ���ʼ��...\n");
    
    uint32_t timer = 0;
    uint32_t timeout = 1000*10; // ��ʱʱ�� ��ʽ:10�� ����:1��
    
    while (1)
    {
        receive_net_msg();
        timer += 100;
        
        if (flag_connected) 
        {
            printf("TCP���ӳɹ�, �����ȴ�\n");
            delay_ms(100);
            break;
        }
        if (timer >= timeout)
        {
            printf("��ʱ, �����ȴ�\n");
            break;
        }
        if (timer % 1000 == 0)
        {
            printf("%u��\n", timer / 1000);
        }
    }
       
    timer = 0;
    timeout = 1000*15; // 15�볬ʱʱ��
    
    if (stmflash_cache.need_update) // ��Ҫ����
    {
        printf("��Ҫ����app, �������app\n");
        net_download_app(); // �������app
        while (1)
        {
            receive_net_msg(); // ������������
            timer += 100;
            
            if (timer % 1000 == 0)
            {
                printf("��ʱ = %u��\n", timer/1000);
            }
            
            if (flag_update_app == 0) // δ�յ�����app
            {
                if (timer >= timeout) // ��ʱ
                {
                    printf("����app��ʱ, ��������app\n");
                    iap_load_app(FLASH_APP1_ADDR); // ִ��app
                }
            }
        }
    }
    else
    {
        printf("�������app, ����app\n");
        iap_load_app(FLASH_APP1_ADDR); // ִ��app
    }
}

