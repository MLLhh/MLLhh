
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

extern STMFLASH_CACHE stmflash_cache; // flash缓存
uint32_t app_version = 27;

int main(void)
{

    /* 设置中断向量表偏移量 */
    sys_nvic_set_vector_table(FLASH_BASE, 0x10000);

    HAL_Init();                          /* 初始化HAL库 */
    sys_stm32_clock_init(360, 25, 2, 8); /* 设置时钟,180Mhz */
    delay_init(180);                     /* 延时初始化 */
    usart_init(115200);                  // 初始化usb串口

    printf("app程序启动\n");

    rs485_init(115200);                     // 初始化rs485串口 (用于外网4G模块)
    rs232_com2_init(9600);                  // 初始化com2串口(rs232转rs485) (用于PLC)
    uart5_init(115200);                     // 初始化uart5串口(外接ttl转485) (用于内网4G模块)
    iwdg_init(IWDG_PRESCALER_256, WDG_RLR); // 初始化看门狗 WDG_RLR定义在user_config.h

    //    led_init();           /* 初始化LED */
    key_init();           /* 初始化按键 */
    sdram_init();         /* SRAM初始化 */
    my_mem_init(SRAMIN);  /* 初始化内部内存池 */
    my_mem_init(SRAMEX);  /* 初始化外部内存池 */
    my_mem_init(SRAMCCM); /* 初始化CCM内存池 */

    printf("app初始化完成\n");

    flash_init(); // 读取并初始化设备信息
    
//    stmflash_cache.machine_type = 2;
//    stmflash_cache.machine_id = 202;
//    stmflash_cache.node_id = 1202;

    printf("app版本 = %u\n", app_version);

    printf("cache.machine_type = %u\n", stmflash_cache.machine_type);
    printf("cache.machine_id = %u\n", stmflash_cache.machine_id);
    printf("cache.node_id = %u\n", stmflash_cache.node_id);
    printf("cache.version_id = %u\n", stmflash_cache.version_id);
    printf("cache.need_update = %u\n", stmflash_cache.need_update);
    printf("cache.read_cycle = %u\n", stmflash_cache.read_cycle);
    printf("cache.plc_cycle = %u\n", stmflash_cache.plc_cycle);
    printf("cache.baudrate = %u\n", stmflash_cache.baudrate);

    // 使用配置的波特率初始化传感器串口
    uint32_t baudrate = 9600;
    if (stmflash_cache.baudrate == 1)
    {
        baudrate = 9600;
    }
    else if (stmflash_cache.baudrate == 2)
    {
        baudrate = 115200;
    }
    rs232_com3_init(baudrate); // 初始化com3串口(rs232转rs485) (用于读传感器)
    printf("波特率 = %u\n", baudrate);

    delay_ms(1000);

    net_cmd_report_app_run(); // 上报app启动

    freertos_demo(); /* 运行主程序 */
}
