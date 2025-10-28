#include "network.h"
#include "SYSTEM/sys/sys.h"
#include "SYSTEM/delay/delay.h"
#include "string.h"
#include "BSP/RS485/rs485.h"
#include "SYSTEM/usart/usart.h"
#include "BSP/STMFLASH/stmflash.h"
#include "BSP/RS485/rs485_uart5.h"
#include "Common/utils.h"
#include "stdint.h"
#include "stdlib.h"
#include "Common/timestamp.h"
#include "PLC/plc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stdbool.h"
#include "BSP/RS232/rs232.h"

#define NET_TX_LEN 400

extern uint32_t app_version; // app版本号(main.c)

uint16_t msg_header = 0x55aa; // 帧头
uint16_t msg_tail = 0xbd0a;   // 帧尾

extern uint8_t g_rs485_rx_buf[RS485_REC_LEN];
extern uint32_t g_rs485_rx_cnt;

uint8_t net_tx_buf[NET_TX_LEN];

uint32_t rxlen_485 = 0;

extern STMFLASH_CACHE stmflash_cache; // flash缓存

char at_msg_timestamp_head[] = "+TIMESTAMP: "; // 时间戳

char g_at_msg_buf[128]; // AT指令缓存
uint32_t at_len = 0;

bool FLAG_REPORT_IN = true;   // 内网上报开关(默认开启)
bool FLAG_REPORT_OUT = false; // 外网上报开关(默认关闭)

bool FLAG_REPORT_SENSOR_STATUS = false; // 上报传感器状态
bool FLAG_REPORT_PLC_STATUS = false;    // 上报PLC状态

NET_MSG_HEAD rx_head; // 通用帧结构:前缀

NET_MSG_DATA_YINGLI msg_yingli; // 应力传感器数据
NET_MSG_DATA_WENZHEN msg_wenzhen; // 温振传感器数据
NET_MSG_DATA_PLC msg_plc_diaoji; // 吊机plc数据
NET_MSG_REPORT_SENSOR_STATUS msg_s_status_diaoji; // 吊机传感器状态
NET_MSG_REPORT_PLC_STATUS msg_plc_status_diaoji; // 吊机plc状态

NET_MSG_DATA_GAOKONGCHE msg_gaokongche;

void network_init(void)
{
    at_sync_timestamp(); // 同步基站时间戳
}

void receive_net_msg(void) // 接收网络数据
{
    rxlen_485 = g_rs485_rx_cnt;

    delay_ms(100);

    if (g_rs485_rx_cnt)
    {
        if (rxlen_485 == g_rs485_rx_cnt)
        {
            printf("外网接收 <<< ");
            printhex(g_rs485_rx_buf, rxlen_485);

            memcpy(&rx_head, &g_rs485_rx_buf, sizeof(rx_head));

            if (rx_head.header == msg_header) // 检查帧头
            {
                printf("接收帧长度 = %u\n", rxlen_485);
                printf("指令帧长度 = %u\n", rx_head.frame_length);

                if (rxlen_485 >= rx_head.frame_length) // 检查帧长度
                {
                    uint16_t tmp_tail;
                    memcpy(&tmp_tail, &g_rs485_rx_buf[rx_head.frame_length - 2], 2);
                    if (tmp_tail == msg_tail) // 检查帧尾
                    {
                        // 检查设备ID与采集单元ID
                        printf("本机设备ID:%u\n", stmflash_cache.machine_id);
                        printf("指令设备ID:%u\n", rx_head.machine_id);
                        printf("本机采集单元ID:%u\n", stmflash_cache.node_id);
                        printf("指令采集单元ID:%u\n", rx_head.node_id);
                        if ((rx_head.machine_id == stmflash_cache.machine_id) && (rx_head.node_id == stmflash_cache.node_id))
                        {
                            net_cmd_report_rcv(rx_head.frame_type, rx_head.sub_type);

                            if (rx_head.frame_type == 0xdd)
                            {
                                analyze_debug_mode_msg(); // 解析调试模式指令
                            }
                            else
                            {
                                analyze_net_msg(); // 解析网络数据
                            }
                        } // end 检查设备ID与采集单元ID
                        else
                        {
                            analyze_debug_msg(); // 解析本地调试指令
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
            rxlen_485 = g_rs485_rx_cnt;
        }
    } // end 接收完成
}

void analyze_net_msg(void) // 解析网络数据
{
    printf("解析网络数据 帧类型:0x%02X 子类型:0x%02X\n", rx_head.frame_type, rx_head.sub_type);

    switch (rx_head.frame_type) // 解析帧类型和子类型
    {
    case 0xa2:
    {
        switch (rx_head.sub_type)
        {
        case 0x01:
            net_cmd_cycle(); // 帧类型识别:更新采集周期
            break;
        case 0x02:
            net_cmd_update(); // 帧类型识别:app待升级
            break;
        case 0x03:
            net_cmd_reboot_4g_out(); // 帧类型识别;重启外网4G模块
            break;
        case 0x04:
            net_cmd_reboot_4g_in(); // 帧类型识别:重启内网4G模块
            break;
        case 0x05:
            net_cmd_report_out_on(); // 开启外网数据上报
            break;
        case 0x06:
            net_cmd_report_out_off(); // 关闭外网数据上报
            break;
        case 0x07:
            net_cmd_report_in_on(); // 开启内网数据上报
            break;
        case 0x08:
            net_cmd_report_in_off(); // 关闭内网数据上报
            break;
        }
        break;
    }
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

    printf("解析AT响应:");
    printstring(g_at_msg_buf, at_len);

    if (0 == strncmp(g_at_msg_buf, at_msg_timestamp_head, sizeof(at_msg_timestamp_head) - 1))
    {
        uint32_t timestamp = strtoul(g_at_msg_buf + sizeof(at_msg_timestamp_head) - 1, NULL, 10);
        printf("时间戳: %u\n", timestamp);
        set_boot_timestamp(timestamp);
    }
    //    }
}

void net_cmd_reboot(void) // 网络指令:重启
{
    net_cmd_report_reboot();
    printf("重启\n");
    delay_ms(500);
    HAL_NVIC_SystemReset();
}

void net_cmd_cycle(void) // 网络指令:更新采集周期
{
    printf("更新采集周期\n");

    NET_MSG_CYCLE msg;
    memcpy(&msg, &g_rs485_rx_buf, sizeof(NET_MSG_CYCLE));

    switch (msg.group_type)
    {
    case 0x01:
    case 0x02:
    case 0x04:
    {
        printf("修改传感器采集周期 %u秒 -> %u秒\n", stmflash_cache.read_cycle, msg.cycle);
        stmflash_cache.read_cycle = msg.cycle;
        flash_write_cache();
        break;
    }
    case 0x03:
    {
        printf("修改PLC采集周期 %u秒 -> %u秒\n", stmflash_cache.plc_cycle, msg.cycle);
        stmflash_cache.plc_cycle = msg.cycle;
        flash_write_cache();
        break;
    }
    }
}

void net_cmd_update(void) // 网络指令:app待升级
{
    printf("app待升级\n");
    stmflash_cache.need_update = 1;
    flash_write_cache();
}

void net_cmd_cancel_update(void)
{
    printf("撤销app待升级");
    stmflash_cache.need_update = 0;
    flash_write_cache();
}

void debug_set_cache(void) // 调试指令:写入缓存
{
//    printf("调试指令:写入缓存\n");

//    DEBUG_SET_CACHE msg;
//    memcpy(&msg, &g_rs485_rx_buf, sizeof(DEBUG_SET_CACHE));

//    stmflash_cache.machine_type = msg.machine_type;
//    stmflash_cache.machine_id = msg.machine_id;
//    stmflash_cache.node_id = msg.node_id;
//    stmflash_cache.version_id = msg.version_id;
//    stmflash_cache.need_update = msg.need_update;
//    flash_write_cache();
}

void analyze_debug_msg(void)
{
//    if (rx_head.frame_type == 0xdd)
//    {
//        switch (rx_head.sub_type)
//        {
//        case 0x01:
//            debug_set_cache(); // 调试指令:写入缓存
//            break;
//        }
//    }
}

void analyze_debug_mode_msg(void) // 解析调试模式指令
{
    printf("解析调试模式指令\n");

    if (rx_head.frame_type == 0xdd)
    {
        switch (rx_head.sub_type)
        {
        case 0x04:
            net_cmd_modify_node(); // 修改节点信息
            break;
        case 0x05:
            net_cmd_cancel_update(); // 撤销app待升级
            break;
        case 0x06:
            net_cmd_report_version(); // 上报app版本号
            break;
        case 0x07:
            net_cmd_set_baudrate_9600(); // 设置传感器口波特率为9600
            break;
        case 0x08:
            net_cmd_set_baudrate_115200(); // 设置传感器口波特率为115200
            break;
        case 0x09:
            net_cmd_on_report_sensor_status(); // 开启上报传感器状态
            break;
        case 0x0a:
            net_cmd_off_report_sensor_status(); // 关闭上报传感器状态
            break;
        case 0x0b:
            net_cmd_on_report_plc_status(); // 开启上报PLC状态
            break;
        case 0x0c:
            net_cmd_off_report_plc_status(); // 关闭上报PLC状态
            break;
        default:
            break;
        }
    }
}

void at_sync_timestamp(void) // AT指令:同步基站时间戳
{
    printf("同步基站时间戳\n");

    char *str = "@DTU:0000:TIMESTAMP";
    send_at_cmd_out(str, strlen(str));
}

void send_at_cmd_out(char *str, size_t size) // 发送AT指令(外网4G模块)
{
    uint8_t array[size];
    string_to_uint8_array(str, array, size);

    rs485_send_data(array, size);
}

void send_at_cmd_in(char *str, size_t size) // 发送AT指令(内网4G模块)
{
    uint8_t array[size];
    string_to_uint8_array(str, array, size);

    uart5_send_data(array, size);
}

void net_cmd_reboot_4g_in(void) // 下行:重启内网4G模块
{
    printf("重启内网4G模块\n");
    char *str = "@DTU:0000:POWEROFF";
    send_at_cmd_in(str, strlen(str));
}

void net_cmd_reboot_4g_out(void) // 下行:重启外网4G模块
{
    printf("重启外网4G模块\n");
    char *str = "@DTU:0000:POWEROFF";
    send_at_cmd_out(str, strlen(str));
}

void net_cmd_report_in_on(void) // 下行:开启内网数据上报
{
    FLAG_REPORT_IN = true;
    printf("开启内网数据上报\n");
}

void net_cmd_report_in_off(void) // 下行:关闭内网数据上报
{
    FLAG_REPORT_IN = false;
    printf("关闭内网数据上报\n");
}

void net_cmd_report_out_on(void) // 下行:开启外网数据上报
{
    FLAG_REPORT_OUT = true;
    printf("开启外网数据上报\n");
}

void net_cmd_report_out_off(void) // 下行:关闭外网数据上报
{
    FLAG_REPORT_OUT = false;
    printf("关闭外网数据上报\n");
}

void net_cmd_report_version(void) // 上报app版本
{
    NET_MSG_REPORT_APP_VERSION msg;
    msg.header = msg_header;
    msg.frame_type = 0xdd;
    msg.sub_type = 0x06;
    msg.frame_length = sizeof(msg);
    msg.machine_type = stmflash_cache.machine_type;
    msg.machine_id = stmflash_cache.machine_id;
    msg.node_id = stmflash_cache.node_id;

    // msg.version_id = stmflash_cache.version_id;
    msg.version_id = app_version; // app版本号

    msg.tail = msg_tail;

    memcpy(net_tx_buf, &msg, sizeof(msg));
    rs485_send_data(net_tx_buf, msg.frame_length);
}

void net_cmd_report_rcv(uint8_t frame_type, uint8_t sub_type) // 上报接收到
{
    NET_MSG msg;
    msg.header = msg_header;
    msg.frame_type = frame_type;
    msg.sub_type = sub_type;
    msg.frame_length = sizeof(msg);
    msg.machine_type = stmflash_cache.machine_type;
    msg.machine_id = stmflash_cache.machine_id;
    msg.node_id = stmflash_cache.node_id;
    msg.tail = msg_tail;

    memcpy(net_tx_buf, &msg, sizeof(msg));
    rs485_send_data(net_tx_buf, sizeof(msg));
}

void net_cmd_report_data_diaoji_yingli(void)
{
    printf("上报吊机应力数据\n");
    msg_yingli.header = msg_header;
    msg_yingli.frame_type = 0x02;
    msg_yingli.sub_type = 0x01;
    msg_yingli.frame_length = sizeof(msg_yingli);
    msg_yingli.machine_type = stmflash_cache.machine_type;
    msg_yingli.machine_id = stmflash_cache.machine_id;
    msg_yingli.node_id = stmflash_cache.node_id;
    msg_yingli.group_type = 0x01;
    msg_yingli.group_id = 0x01;
    msg_yingli.timestamp = get_timestamp();
    msg_yingli.tail = msg_tail;

    memcpy(net_tx_buf, &msg_yingli, sizeof(msg_yingli));
    if (FLAG_REPORT_OUT)
        rs485_send_data(net_tx_buf, sizeof(msg_yingli));
    if (FLAG_REPORT_IN)
        uart5_send_data(net_tx_buf, sizeof(msg_yingli));
}

void net_cmd_report_data_diaoji_wenzhen(void)
{
    printf("上报吊机温振数据\n");
    msg_wenzhen.header = msg_header;
    msg_wenzhen.frame_type = 0x02;
    msg_wenzhen.sub_type = 0x01;
    msg_wenzhen.frame_length = sizeof(msg_wenzhen);
    msg_wenzhen.machine_type = stmflash_cache.machine_type;
    msg_wenzhen.machine_id = stmflash_cache.machine_id;
    msg_wenzhen.node_id = stmflash_cache.node_id;
    msg_wenzhen.group_type = 0x02;
    msg_wenzhen.group_id = 0x01;
    msg_wenzhen.timestamp = get_timestamp();
    msg_wenzhen.tail = msg_tail;

    memcpy(net_tx_buf, &msg_wenzhen, sizeof(msg_wenzhen));
    if (FLAG_REPORT_OUT)
        rs485_send_data(net_tx_buf, sizeof(msg_wenzhen));
    if (FLAG_REPORT_IN)
        uart5_send_data(net_tx_buf, sizeof(msg_wenzhen));
}

void net_cmd_report_data_diaoji_plc(void)
{
    printf("上报吊机PLC数据\n");
    msg_plc_diaoji.header = msg_header;
    msg_plc_diaoji.frame_type = 0x02;
    msg_plc_diaoji.sub_type = 0x01;
    msg_plc_diaoji.frame_length = sizeof(msg_plc_diaoji);
    msg_plc_diaoji.machine_type = stmflash_cache.machine_type;
    msg_plc_diaoji.machine_id = stmflash_cache.machine_id;
    msg_plc_diaoji.node_id = stmflash_cache.node_id;
    msg_plc_diaoji.group_type = 0x03;
    msg_plc_diaoji.group_id = 0x01;
    msg_plc_diaoji.timestamp = get_timestamp();
    msg_plc_diaoji.tail = msg_tail;

    memcpy(net_tx_buf, &msg_plc_diaoji, sizeof(msg_plc_diaoji));
    if (FLAG_REPORT_OUT)
        rs485_send_data(net_tx_buf, sizeof(msg_plc_diaoji));
    if (FLAG_REPORT_IN)
        uart5_send_data(net_tx_buf, sizeof(msg_plc_diaoji));
}

void net_cmd_report_data_gaokongche_sensor(void)
{
    printf("上报高空车传感器数据\n");
    msg_gaokongche.header = msg_header;
    msg_gaokongche.frame_type = 0x02;
    msg_gaokongche.sub_type = 0x01;
    msg_gaokongche.frame_length = sizeof(msg_gaokongche);
    msg_gaokongche.machine_type = stmflash_cache.machine_type;
    msg_gaokongche.machine_id = stmflash_cache.machine_id;
    msg_gaokongche.node_id = stmflash_cache.node_id;
    msg_gaokongche.group_type = 0x04;
    msg_gaokongche.group_id = 0x01;
    msg_gaokongche.timestamp = get_timestamp();
    msg_gaokongche.tail = msg_tail;

    memcpy(net_tx_buf, &msg_gaokongche, sizeof(msg_gaokongche));
    if (FLAG_REPORT_OUT)
        rs485_send_data(net_tx_buf, sizeof(msg_gaokongche)); // 外网
    if (FLAG_REPORT_IN)
        uart5_send_data(net_tx_buf, sizeof(msg_gaokongche));     // 内网
}

void net_cmd_report_status_diaoji_sensor(void)
{
    if (FLAG_REPORT_SENSOR_STATUS == false)
        return;

    printf("上报吊机传感器状态\n");
    msg_s_status_diaoji.header = msg_header;
    msg_s_status_diaoji.frame_type = 0xdd;
    msg_s_status_diaoji.sub_type = 0x01;
    msg_s_status_diaoji.frame_length = sizeof(msg_s_status_diaoji);
    msg_s_status_diaoji.machine_type = stmflash_cache.machine_type;
    msg_s_status_diaoji.machine_id = stmflash_cache.machine_id;
    msg_s_status_diaoji.node_id = stmflash_cache.node_id;
    msg_s_status_diaoji.tail = msg_tail;

    memcpy(net_tx_buf, &msg_s_status_diaoji, sizeof(msg_s_status_diaoji));
    rs485_send_data(net_tx_buf, sizeof(msg_s_status_diaoji));
}

void net_cmd_report_status_diaoji_plc(void)
{
    if (FLAG_REPORT_PLC_STATUS == false)
        return;

    printf("上报吊机PLC状态\n");
    msg_plc_status_diaoji.header = msg_header;
    msg_plc_status_diaoji.frame_type = 0xdd;
    msg_plc_status_diaoji.sub_type = 0x02;
    msg_plc_status_diaoji.frame_length = sizeof(msg_plc_status_diaoji);
    msg_plc_status_diaoji.machine_type = stmflash_cache.machine_type;
    msg_plc_status_diaoji.machine_id = stmflash_cache.machine_id;
    msg_plc_status_diaoji.node_id = stmflash_cache.node_id;
    msg_plc_status_diaoji.tail = msg_tail;

    memcpy(net_tx_buf, &msg_plc_status_diaoji, sizeof(msg_plc_status_diaoji));
    rs485_send_data(net_tx_buf, sizeof(msg_plc_status_diaoji));
}

void net_cmd_modify_node(void) // 修改节点信息
{
    printf("修改节点信息\n");

    NET_MSG_MODIFY_NODE msg;
    memcpy(&msg, &g_rs485_rx_buf, sizeof(msg));

    uint16_t new_machine_type = msg.new_machine_type;
    uint16_t new_machine_id = msg.new_machine_id;
    uint16_t new_node_id = msg.new_node_id;

    printf("machine_type: %u -> %u \n", stmflash_cache.machine_type, new_machine_type);
    printf("machine_id: %u -> %u \n", stmflash_cache.machine_id, new_machine_id);
    printf("node_id: %u -> %u \n", stmflash_cache.node_id, new_node_id);

    stmflash_cache.machine_type = new_machine_type;
    stmflash_cache.machine_id = new_machine_id;
    stmflash_cache.node_id = new_node_id;
    flash_write_cache();
}

void net_cmd_report_common(uint8_t frame_type, uint8_t sub_type) // 通用指令
{
    NET_MSG msg;
    msg.header = msg_header;
    msg.frame_type = frame_type;
    msg.sub_type = sub_type;
    msg.frame_length = 0x10;
    msg.machine_type = stmflash_cache.machine_type;
    msg.machine_id = stmflash_cache.machine_id;
    msg.node_id = stmflash_cache.node_id;
    msg.tail = msg_tail;
    memcpy(net_tx_buf, &msg, sizeof(msg));
    rs485_send_data(net_tx_buf, sizeof(msg));
}

void net_cmd_report_app_run(void)
{
    net_cmd_report_common(0xee, 0x01);
    printf("报告app启动\n");
}

void net_cmd_report_reboot(void)
{
    net_cmd_report_common(0xee, 0x02);
    printf("报告即将重启");
}

void net_cmd_set_baudrate_9600(void)
{
    printf("设置传感器口波特率为9600\n");
    stmflash_cache.baudrate = 1;
    flash_write_cache();
}

void net_cmd_set_baudrate_115200(void)
{
    printf("设置传感器口波特率为115200\n");
    stmflash_cache.baudrate = 2;
    flash_write_cache();
}

void net_cmd_on_report_sensor_status(void) // 开启上报传感器状态
{
    printf("开启上报传感器状态\n");
    FLAG_REPORT_SENSOR_STATUS = true;
}

void net_cmd_off_report_sensor_status(void) // 关闭上报传感器状态
{
    printf("关闭上报传感器状态\n");
    FLAG_REPORT_SENSOR_STATUS = false;
}

void net_cmd_on_report_plc_status(void) // 开启上报PLC状态
{
    printf("开启上报PLC状态\n");
    FLAG_REPORT_PLC_STATUS = true;
}

void net_cmd_off_report_plc_status(void) // 关闭上报PLC状态
{
    printf("关闭上报PLC状态\n");
    FLAG_REPORT_PLC_STATUS = false;
}
