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

extern STMFLASH_CACHE stmflash_cache; // flash缓存

extern uint8_t flag_connected; // TCP连接成功标志(ota.c)
extern uint8_t flag_update_app; // 收到更新app

int main(void)
{
    HAL_Init();                                                   /* 初始化HAL库 */
    sys_stm32_clock_init(360, 25, 2, 8);                          /* 设置时钟,180Mhz */
    delay_init(180);                                              /* 延时初始化 */
    usart_init(115200);                                           /* 串口初始化为115200 */
    led_init();                                                   /* 初始化LED */
    key_init();                                                   /* 初始化按键 */
    sdram_init();                                                 /* 初始化SDRAM */
    rs485_init(115200);                                           /* 初始化485接口 */
    
    printf("bootloader程序启动\n");
    
    flash_read_cache(); // 读本地缓存

    printf("cache.machine_type = %u\n", stmflash_cache.machine_type);
    printf("cache.machine_id = %u\n", stmflash_cache.machine_id);
    printf("cache.node_id = %u\n", stmflash_cache.node_id);
    printf("cache.version_id = %u\n", stmflash_cache.version_id);
    printf("cache.need_update = %u\n", stmflash_cache.need_update);
    printf("cache.read_cycle = %u\n", stmflash_cache.read_cycle);
    printf("cache.plc_cycle = %u\n", stmflash_cache.plc_cycle);
    
    printf("4G模块初始化...\n");
    
    uint32_t timer = 0;
    uint32_t timeout = 1000*10; // 超时时间 正式:10秒 调试:1秒
    
    while (1)
    {
        receive_net_msg();
        timer += 100;
        
        if (flag_connected) 
        {
            printf("TCP连接成功, 结束等待\n");
            delay_ms(100);
            break;
        }
        if (timer >= timeout)
        {
            printf("超时, 结束等待\n");
            break;
        }
        if (timer % 1000 == 0)
        {
            printf("%u秒\n", timer / 1000);
        }
    }
       
    timer = 0;
    timeout = 1000*15; // 15秒超时时间
    
    if (stmflash_cache.need_update) // 需要更新
    {
        printf("需要更新app, 请求更新app\n");
        net_download_app(); // 请求更新app
        while (1)
        {
            receive_net_msg(); // 接收网络数据
            timer += 100;
            
            if (timer % 1000 == 0)
            {
                printf("计时 = %u秒\n", timer/1000);
            }
            
            if (flag_update_app == 0) // 未收到更新app
            {
                if (timer >= timeout) // 超时
                {
                    printf("更新app超时, 放弃更新app\n");
                    iap_load_app(FLASH_APP1_ADDR); // 执行app
                }
            }
        }
    }
    else
    {
        printf("无需更新app, 启动app\n");
        iap_load_app(FLASH_APP1_ADDR); // 执行app
    }
}

