#include "plc.h"
#include "BSP/RS232/rs232.h"
#include "SYSTEM/sys/sys.h"
#include "SYSTEM/delay/delay.h"
#include "Common/utils.h"
#include "stdio.h"
#include "string.h"
#include "BSP/STMFLASH/stmflash.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stdlib.h"
#include "Network/network.h"
#include "stdbool.h"

uint32_t rxlen_com2 = 0;
uint8_t plc_rx_buf[128];
// STRUCT_PLC_DATA plc_data; // 存储采集到的数据

extern STMFLASH_CACHE stmflash_cache; // flash缓存

uint32_t tick = 0; // 计数
uint32_t tick_max = 0;
uint32_t tick_interval = 0;

uint32_t tick_cnt = 0;       // 计数器
uint32_t plc_interval = 500; // 豪秒 指令发送间隔

uint8_t plc_reg_total = 8;                           // 需要读取的寄存器总数
uint8_t plc_reg_index = 0;                           // 当前读取寄存器在数组中的序号
uint8_t plc_reg = 0;                                 // 当前读取寄存器号
uint8_t plc_reg_array[] = {0, 1, 2, 3, 4, 5, 9, 11}; // 需要读取的寄存器号数组

extern NET_MSG_DATA_PLC msg_plc_diaoji;
extern NET_MSG_REPORT_PLC_STATUS msg_plc_status_diaoji;

int debug_id = 0;

void plc_init(void)
{
    tick_max = stmflash_cache.plc_cycle * (1000 / 100);
    tick_interval = plc_interval / 100;

    reset_plc_data();
}

void read_plc_data(void) // 发送数据
{
    if (tick % tick_interval == 0)
    {
        tick_cnt = tick / tick_interval;
        if (tick_cnt < plc_reg_total + 1) // 此处+1是因为采集完成后留一个tick_cnt进行上报
        {
            read_next_plc_reg();
        }
    }
    tick = (tick + 1) % tick_max;
}

void receive_plc_data(void) // 接收数据
{
    rxlen_com2 = g_rs232_com2_rx_cnt;

    delay_ms(10);

    if (g_rs232_com2_rx_cnt)
    {
        if (rxlen_com2 == g_rs232_com2_rx_cnt)
        {
            // printf("接收PLC %u 号寄存器< ", plc_reg);
            printstring((char *)g_rs232_com2_rx_buf, rxlen_com2);

            if (check_plc_data())
            {
                analyze_plc_data();
            }

            g_rs232_com2_rx_cnt = 0;
        }
        else
        {
            rxlen_com2 = g_rs232_com2_rx_cnt;
        }
    }
}

bool check_plc_data(void) // 检查PLC返回数据格式
{
    memcpy(&plc_rx_buf, &g_rs232_com2_rx_buf, rxlen_com2);

    char *buf = (char *)plc_rx_buf;
    char res[2];
    memcpy(res, buf + 5, 2);

    if (strncmp(res, "00", 2) == 0)
    {
        if (strncmp(res_plc_head, buf, 7) == 0)
        {
            return true;
        }
        else
        {
            printf("响应格式异常\n");
            return false;
        }
    }
    else
    {
        printf("应答码异常 = %c%c\n", res[0], res[1]);
        return false;
    }
}

void analyze_plc_data(void) // 解析PLC返回数据
{
    char *buf = (char *)plc_rx_buf;
    // char buf[] = "@00RR00010100*"; // 调试
    char ascii[4];              // 读数(ascii格式的十六进制数)
    memcpy(ascii, buf + 7, 4);  // 从PLC返回数据中取出读数
    uint16_t val;               // 真值
    sscanf(ascii, "%hx", &val); // 将ascii格式的十六进制数转换为真值

    msg_plc_status_diaoji.reg[plc_reg] = 0x01; // 更新PLC状态

    switch (plc_reg)
    {
    case 0:
    {
        msg_plc_diaoji.data[1] = get_bit(val, 11); // 1       0.11    主起升上升限位
        msg_plc_diaoji.data[3] = get_bit(val, 12); // 3       0.12    主起升下降限位
        msg_plc_diaoji.data[15] = get_bit(val, 6); // 15      0.06    主/副起升变频器故障
        break;
    }
    case 1:
    {
        msg_plc_diaoji.data[4] = get_bit(val, 11); // 4       1.11    副起升上升限位
        msg_plc_diaoji.data[6] = get_bit(val, 12); // 6       1.12    副起升下降限位
        break;
    }
    case 2:
    {
        msg_plc_diaoji.data[7] = get_bit(val, 11);  // 7       2.11    变幅减幅限位
        msg_plc_diaoji.data[9] = get_bit(val, 12);  // 9       2.12    变幅增幅限位
        msg_plc_diaoji.data[10] = get_bit(val, 13); // 10      2.13    变幅放到限位
        msg_plc_diaoji.data[16] = get_bit(val, 5);  // 16      2.05    变幅变频器故障
        break;
    }
    case 3:
    {
        msg_plc_diaoji.data[17] = get_bit(val, 5);  // 17      3.05    旋转变频器故障
        msg_plc_diaoji.data[18] = get_bit(val, 13); // 18      3.13    行走变频器故障
        break;
    }
    case 4:
    {
        msg_plc_diaoji.data[11] = get_bit(val, 9); // 11      4.09    行走锚定限位
        msg_plc_diaoji.data[12] = get_bit(val, 5); // 12      4.05    左行限位
        msg_plc_diaoji.data[13] = get_bit(val, 6); // 13      4.06    右行限位
        msg_plc_diaoji.data[19] = get_bit(val, 7); // 19      4.07    电缆终端限位
        break;
    }
    case 5:
    {
        msg_plc_diaoji.data[14] = get_bit(val, 7); // 14      5.07    超载信号
        break;
    }
    case 9:
    {
        msg_plc_diaoji.data[2] = get_bit(val, 9);  // 2       9.09    主起升上升极限位
        msg_plc_diaoji.data[5] = get_bit(val, 10); // 5       9.10    副起升上升极限位
        msg_plc_diaoji.data[8] = get_bit(val, 11); // 8       9.11    变幅减幅极限限位
        break;
    }
    case 11:
    {
        msg_plc_diaoji.data[0] = get_bit(val, 7); // 0       11.07   主接触器中继
        break;
    }
    }
}

// 读取下一个寄存器
void read_next_plc_reg(void)
{
    if (plc_reg_index < plc_reg_total)
    {
        plc_reg = plc_reg_array[plc_reg_index];
        read_reg(plc_reg);
        plc_reg_index++;
    }
    else if (plc_reg_index == plc_reg_total)
    {
        refine_plc_data();
        
        net_cmd_report_data_diaoji_plc();
        net_cmd_report_status_diaoji_plc();
        
        plc_reg_index++;
    }
    else
    {
        reset_plc_data(); // 重置数据缓存
        plc_reg_index = 0;
    }
}

// 读指定寄存器
void read_reg(uint8_t reg)
{
    // printf("读取PLC %u 号寄存器: ", reg);
    int len = 19;
    uint8_t buf[len];

    switch (reg)
    {
    case 0:
        memcpy(buf, read_plc_0, len);
        break;
    case 1:
        memcpy(buf, read_plc_1, len);
        break;
    case 2:
        memcpy(buf, read_plc_2, len);
        break;
    case 3:
        memcpy(buf, read_plc_3, len);
        break;
    case 4:
        memcpy(buf, read_plc_4, len);
        break;
    case 5:
        memcpy(buf, read_plc_5, len);
        break;
    case 9:
        memcpy(buf, read_plc_9, len);
        break;
    case 11:
        memcpy(buf, read_plc_11, len);
        break;
    default:
        printf("试图读取未定义寄存器 %u\n", reg);
        return;
    }

    rs232_com2_send_data(buf, sizeof(buf));
}

void reset_plc_data(void) // 重置PLC数据缓存
{
    printf("重置数据缓存(PLC)\n");

    for (int i = 0; i < 20; i++)
    {
        msg_plc_diaoji.data[i] = 0;
    }

    for (int i = 0; i < 12; i++)
    {
        msg_plc_status_diaoji.reg[i] = 0xff;
    }
}

void refine_plc_data(void)
{
    // 补充数据
    if (stmflash_cache.machine_id == 101)
    {
        msg_plc_diaoji.data[0] = 1; // 主接触器中继(代表设备在线)
    }
    else if (stmflash_cache.machine_id == 102 || stmflash_cache.machine_id == 103)
    {
        for (int i =0; i < 20; i++)
        {
            msg_plc_diaoji.data[i] = 2;
        }
    }
}
