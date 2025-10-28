#include "ota.h"
#include "SYSTEM/sys/sys.h"
#include "SYSTEM/usart/usart.h"
#include "SYSTEM/delay/delay.h"
#include "BSP/STMFLASH/stmflash.h"
#include "string.h"
#include "BSP/RS485/rs485.h"
#include "IAP/iap.h"
#include "Common/utils.h"
#include "BSP/LED/led.h"

uint16_t msg_header = 0x55aa;   // 帧头
uint16_t msg_tail = 0xbd0a;     // 帧尾

extern uint8_t g_rs485_tx_buf[TXBUFFERSIZE]; // 发送缓冲区
extern uint8_t g_rs485_rx_buf[RS485_REC_LEN];
extern uint32_t g_rs485_rx_cnt;

extern STMFLASH_CACHE stmflash_cache; // flash缓存

char at_head[] = "\r\n";
char at_tail[] = "\r\n";
char at_msg_timestamp_head[] = "+TIMESTAMP: "; // 时间戳
char at_msg_registered[] = "\r\n+STATUS:NET STATE REGISTERED\r\n";
char at_msg_connected[] = "\r\n+STATUS: 1, CONNECTED\r\n"; // TCP连接成功
char at_msg_closed[] = "\r\n+STATUS: 1, CLOSED\r\n"; // TCP连接断开

char g_at_msg_buf[128];  // AT指令缓存
uint32_t at_len = 0;

uint8_t flag_connected = 0; // TCP连接成功标志
uint8_t flag_update_app = 0; // 收到更新app

NET_MSG_HEAD rx_head; // 通用帧结构:前缀

void net_download_app(void) // 请求更新app
{
    printf("请求更新app\n");
    
    NET_MSG_DOWNLOAD msg;
    msg.header = msg_header;                            // 帧头
    msg.frame_type = 0x01;                              // 帧类型
    msg.sub_type = 0x01;                                // 子类型
    msg.frame_length = sizeof(NET_MSG_DOWNLOAD);        // 帧长度
    msg.machine_type = stmflash_cache.machine_type;     // 设备类型
    msg.machine_id = stmflash_cache.machine_id;         // 设备ID
    msg.node_id = stmflash_cache.node_id;               // 采集单元ID
    msg.packet_no = 0;                                  // 保留
    msg.tail = msg_tail;                                // 帧尾
    
    memcpy(&g_rs485_tx_buf, &msg, sizeof(msg));
    
    rs485_send_data(g_rs485_tx_buf, msg.frame_length); // 发送数据
}

void receive_net_msg(void) // 接收网络数据
{
    uint32_t rxlen = g_rs485_rx_cnt;
    delay_ms(100);
    
    if (g_rs485_rx_cnt)
    {
        if (rxlen == g_rs485_rx_cnt)
        {
            printf("接收 <<< ");
            printhex(g_rs485_rx_buf, rxlen);
            
            // 解析指令
            memcpy(&rx_head, &g_rs485_rx_buf, sizeof(rx_head));
            
            printf("接收帧长度 = %u\n", rxlen);
            printf("指令帧长度 = %u\n", rx_head.frame_length);
            if (rx_head.header == msg_header) // 检查帧头
            {
                if (rxlen >= rx_head.frame_length) // 检查帧长度
                {
                    uint16_t tmp_tail;
                    memcpy(&tmp_tail, &g_rs485_rx_buf[rx_head.frame_length-2], 2);
                    if (tmp_tail == msg_tail) // 检查帧尾
                    {
                        // 检查设备ID与采集单元ID
                        printf("本机设备ID:%u\n", stmflash_cache.machine_id);
                        printf("指令设备ID:%u\n", rx_head.machine_id);
                        printf("本机采集单元ID:%u\n", stmflash_cache.node_id);
                        printf("指令采集单元ID:%u\n", rx_head.node_id);
                        if ((rx_head.machine_id == stmflash_cache.machine_id) && (rx_head.node_id == stmflash_cache.node_id))
                        {
                            if (rx_head.frame_type == 0xdd)
                            {
                                analyze_debug_msg(); // 解析调试指令
                            }
                            else
                            {
                                analyze_net_msg(); // 解析网络数据
                            }
                        } // end 检查设备ID与采集单元ID
                        else
                        {
                            analyze_debug_msg(); // 解析调试指令
                        }
                    }
                    else
                    {
                        printf("帧尾错误\n");
                    } // end 检查帧尾
                }
                else
                {
                   printf("帧长度错误\n");
                } // end 检查帧长度
            }
            else
            {
                analyze_at_msg(); // 解析AT指令
                
            } // end 检查帧头
            
            g_rs485_rx_cnt = 0; // 清零
        }
        else
        {
            rxlen = g_rs485_rx_cnt;
        }
    } // end 接收完成
}

void analyze_net_msg(void) // 解析网络数据
{
    printf("帧类型:0x%02X\n", rx_head.frame_type);
    printf("子类型:0x%02X\n", rx_head.sub_type);
    switch (rx_head.frame_type) // 解析帧类型和子类型
    {
        case 0xa1:
            {
                switch (rx_head.sub_type)
                {
                    case 0x01:
                        net_cmd_app(); // 帧类型识别:发送app
                        break;
                }
            }
            break;
        case 0xfe:
            net_cmd_reboot(); // 帧类型识别:重启
            break;
        default:
            printf("帧类型识别失败\n");
            break;
    }
}

void analyze_at_msg(void) // 解析AT指令
{
    at_len = g_rs485_rx_cnt;
    memcpy(g_at_msg_buf, g_rs485_rx_buf, g_rs485_rx_cnt);
    
    if ((0==strncmp(g_at_msg_buf, at_head, sizeof(at_head)-1)) && (0==strncmp(g_at_msg_buf + at_len - sizeof(at_tail) + 1, at_tail, sizeof(at_tail)-1)))
    {
        printf("AT响应:");
        printstring(g_at_msg_buf, at_len);
        
        if (0==strncmp(g_at_msg_buf, at_msg_registered, sizeof(at_msg_registered)-1)) // 通信基站连接成功
        {
            printf("通信基站连接成功\n");
        }
        else if (0==strncmp(g_at_msg_buf, at_msg_connected, sizeof(at_msg_connected)-1)) // TCP连接成功
        {
            printf("TCP连接成功\n");
            LED1(0);
            flag_connected = 1;
        }
        else if (0==strncmp(g_at_msg_buf, at_msg_closed, sizeof(at_msg_closed)-1)) // TCP连接断开
        {
            printf("TCP连接断开\n");
            LED1(1);
        }
    }
}

void net_cmd_reboot(void) // 网络指令:重启
{
    printf("重启\n");
    
    HAL_NVIC_SystemReset();
}

void net_cmd_app(void) // 网络指令:更新app
{
    printf("更新app\n");
    
    flag_update_app = 1;
    
    NET_MSG_APP msg; // 更新app指令头
    memcpy(&msg, &g_rs485_rx_buf, sizeof(NET_MSG_APP));
    
    printf("程序版本:%u\n", msg.app_version);
    printf("程序长度:%u\n", msg.app_length);
        
    int start = sizeof(NET_MSG_APP);
    
    printf("开始更新固件...\n");
    
    iap_write_appbin(FLASH_APP1_ADDR, g_rs485_rx_buf + start, msg.app_length); // 将app更新到flash
    
    printf("固件更新完成!\n");
    
    // 更新app版本信息
    stmflash_cache.version_id = msg.app_version;
    stmflash_cache.need_update = 0;
    flash_write_cache();
    
    delay_ms(10);
    
    iap_load_app(FLASH_APP1_ADDR); // 执行app
}

void debug_set_cache(void) // 调试指令:写入缓存
{
    printf("调试指令:写入缓存\n");
    
    DEBUG_SET_CACHE msg;
    memcpy(&msg, &g_rs485_rx_buf, sizeof(DEBUG_SET_CACHE));
    
    printf("machine_type = %u\n", msg.machine_type);
    printf("machine_id = %u\n", msg.machine_id);
    printf("node_id = %u\n", msg.node_id);
    printf("version_id = %u\n", msg.version_id);
    printf("need_update = %u\n", msg.need_update);
    
    stmflash_cache.machine_type = msg.machine_type;
    stmflash_cache.machine_id = msg.machine_id;
    stmflash_cache.node_id = msg.node_id;
    stmflash_cache.version_id = msg.version_id;
    stmflash_cache.need_update = msg.need_update;
    flash_write_cache();
}

void analyze_debug_msg(void) // 解析调试指令
{
    if (rx_head.frame_type == 0xdd)
    {
        switch (rx_head.sub_type)
        {
            case 0x01:
                debug_set_cache(); // 调试指令:写入缓存
                break;
        }
    }
}

void at_sync_timestamp(void) // AT指令:同步基站时间戳
{
    printf("同步基站时间戳\n");
    
    char *str = "@DTU:0000:TIMESTAMP";
    send_at_cmd(str, strlen(str));
}

void send_at_cmd(char *str, size_t size) // 发送AT指令(外网4G模块)
{
    uint8_t array[size];
    string_to_uint8_array(str, array, size);
    
    memcpy(&g_rs485_tx_buf, array, size);
    
    rs485_send_data(g_rs485_tx_buf, size);
}
