
#include "SYSTEM/sys/sys.h"
#include "SYSTEM/usart/usart.h"
#include "SYSTEM/delay/delay.h"
#include "BSP/KEY/key.h"
#include "BSP/SDRAM/sdram.h"
#include "BSP/RS485/rs485.h"
#include "BSP/RS232/rs232.h"
#include "BSP/RS485/rs485_uart5.h"
#include "MALLOC/malloc.h"
#include "freertos_demo.h"
#include "BSP/STMFLASH/stmflash.h"
#include "Common/utils.h"
#include "Network/network.h"
#include "BSP/WDG/wdg.h"

extern STMFLASH_CACHE stmflash_cache; // flash����
uint32_t app_version = 27;

int main(void)
{

    /* �����ж�������ƫ���� */
    sys_nvic_set_vector_table(FLASH_BASE, 0x10000);

    HAL_Init();                          /* ��ʼ��HAL�� */
    sys_stm32_clock_init(360, 25, 2, 8); /* ����ʱ��,180Mhz */
    delay_init(180);                     /* ��ʱ��ʼ�� */
    usart_init(115200);                  // ��ʼ��usb����

    printf("app��������\n");

    rs485_init(115200);                     // ��ʼ��rs485���� (��������4Gģ��)
    rs232_com2_init(9600);                  // ��ʼ��com2����(rs232תrs485) (����PLC)
    uart5_init(115200);                     // ��ʼ��uart5����(���ttlת485) (��������4Gģ��)
    iwdg_init(IWDG_PRESCALER_256, WDG_RLR); // ��ʼ�����Ź� WDG_RLR������user_config.h

    //    led_init();           /* ��ʼ��LED */
    key_init();           /* ��ʼ������ */
    sdram_init();         /* SRAM��ʼ�� */
    my_mem_init(SRAMIN);  /* ��ʼ���ڲ��ڴ�� */
    my_mem_init(SRAMEX);  /* ��ʼ���ⲿ�ڴ�� */
    my_mem_init(SRAMCCM); /* ��ʼ��CCM�ڴ�� */

    printf("app��ʼ�����\n");

    flash_init(); // ��ȡ����ʼ���豸��Ϣ
    
//    stmflash_cache.machine_type = 2;
//    stmflash_cache.machine_id = 202;
//    stmflash_cache.node_id = 1202;

    printf("app�汾 = %u\n", app_version);

    printf("cache.machine_type = %u\n", stmflash_cache.machine_type);
    printf("cache.machine_id = %u\n", stmflash_cache.machine_id);
    printf("cache.node_id = %u\n", stmflash_cache.node_id);
    printf("cache.version_id = %u\n", stmflash_cache.version_id);
    printf("cache.need_update = %u\n", stmflash_cache.need_update);
    printf("cache.read_cycle = %u\n", stmflash_cache.read_cycle);
    printf("cache.plc_cycle = %u\n", stmflash_cache.plc_cycle);
    printf("cache.baudrate = %u\n", stmflash_cache.baudrate);

    // ʹ�����õĲ����ʳ�ʼ������������
    uint32_t baudrate = 9600;
    if (stmflash_cache.baudrate == 1)
    {
        baudrate = 9600;
    }
    else if (stmflash_cache.baudrate == 2)
    {
        baudrate = 115200;
    }
    rs232_com3_init(baudrate); // ��ʼ��com3����(rs232תrs485) (���ڶ�������)
    printf("������ = %u\n", baudrate);

    delay_ms(1000);

    net_cmd_report_app_run(); // �ϱ�app����

    freertos_demo(); /* ���������� */
}
