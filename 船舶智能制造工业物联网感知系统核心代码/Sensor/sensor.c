#include "sensor.h"
#include "BSP/STMFLASH/stmflash.h"
#include "BSP/RS232/rs232.h"
#include "SYSTEM/sys/sys.h"
#include "SYSTEM/delay/delay.h"
#include "Common/utils.h"
#include "string.h"
#include "stdbool.h"
#include "Network/network.h"
#include "stdlib.h"
#include "math.h"

// 吊机传感器指令
/*-----------------------------------------------------------------------------------------------*/
// 1号 - 11号 温振传感器
uint8_t s_cmd_1a[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x0d, 0x84, 0x0f}; // 01 03 00 00 00 0D 84 0F
uint8_t s_cmd_1b[] = {0x01, 0x03, 0x00, 0x21, 0x00, 0x06, 0x95, 0xc2}; // 01 03 00 21 00 06 95 C2

uint8_t s_cmd_2a[] = {0x02, 0x03, 0x00, 0x00, 0x00, 0x0d, 0x84, 0x3c}; // 02 03 00 00 00 0D 84 3C
uint8_t s_cmd_2b[] = {0x02, 0x03, 0x00, 0x21, 0x00, 0x06, 0x95, 0xf1}; // 02 03 00 21 00 06 95 F1

uint8_t s_cmd_3a[] = {0x03, 0x03, 0x00, 0x00, 0x00, 0x0d, 0x85, 0xed}; // 03 03 00 00 00 0D 85 ED
uint8_t s_cmd_3b[] = {0x03, 0x03, 0x00, 0x21, 0x00, 0x06, 0x94, 0x20}; // 03 03 00 21 00 06 94 20

uint8_t s_cmd_4a[] = {0x04, 0x03, 0x00, 0x00, 0x00, 0x0d, 0x84, 0x5a}; // 04 03 00 00 00 0D 84 5A
uint8_t s_cmd_4b[] = {0x04, 0x03, 0x00, 0x21, 0x00, 0x06, 0x95, 0x97}; // 04 03 00 21 00 06 95 97

uint8_t s_cmd_5a[] = {0x05, 0x03, 0x00, 0x00, 0x00, 0x0d, 0x85, 0x8b}; // 05 03 00 00 00 0D 85 8B
uint8_t s_cmd_5b[] = {0x05, 0x03, 0x00, 0x21, 0x00, 0x06, 0x94, 0x46}; // 05 03 00 21 00 06 94 46

uint8_t s_cmd_6a[] = {0x06, 0x03, 0x00, 0x00, 0x00, 0x0d, 0x85, 0xb8}; // 06 03 00 00 00 0D 85 B8
uint8_t s_cmd_6b[] = {0x06, 0x03, 0x00, 0x21, 0x00, 0x06, 0x94, 0x75}; // 06 03 00 21 00 06 94 75

uint8_t s_cmd_7a[] = {0x07, 0x03, 0x00, 0x00, 0x00, 0x0d, 0x84, 0x69}; // 07 03 00 00 00 0D 84 69
uint8_t s_cmd_7b[] = {0x07, 0x03, 0x00, 0x21, 0x00, 0x06, 0x95, 0xa4}; // 07 03 00 21 00 06 95 A4

uint8_t s_cmd_8a[] = {0x08, 0x03, 0x00, 0x00, 0x00, 0x0d, 0x84, 0x96}; // 08 03 00 00 00 0D 84 96
uint8_t s_cmd_8b[] = {0x08, 0x03, 0x00, 0x21, 0x00, 0x06, 0x95, 0x5b}; // 08 03 00 21 00 06 95 5B

uint8_t s_cmd_9a[] = {0x09, 0x03, 0x00, 0x00, 0x00, 0x0d, 0x85, 0x47}; // 09 03 00 00 00 0D 85 47
uint8_t s_cmd_9b[] = {0x09, 0x03, 0x00, 0x21, 0x00, 0x06, 0x94, 0x8a}; // 09 03 00 21 00 06 94 8A

uint8_t s_cmd_10a[] = {0x0a, 0x03, 0x00, 0x00, 0x00, 0x0d, 0x85, 0x74}; // 0A 03 00 00 00 0D 85 74
uint8_t s_cmd_10b[] = {0x0a, 0x03, 0x00, 0x21, 0x00, 0x06, 0x94, 0xb9}; // 0A 03 00 21 00 06 94 B9

uint8_t s_cmd_11a[] = {0x0b, 0x03, 0x00, 0x00, 0x00, 0x0d, 0x84, 0xa5}; // 0B 03 00 00 00 0D 84 A5
uint8_t s_cmd_11b[] = {0x0b, 0x03, 0x00, 0x21, 0x00, 0x06, 0x95, 0x68}; // 0B 03 00 21 00 06 95 68

// 12号 温度传感器
uint8_t s_cmd_12[] = {0x0c, 0x03, 0x00, 0x00, 0x00, 0x01, 0x85, 0x17}; // 0C 03 00 00 00 01 85 17

// 13号 - 22号 应力传感器
uint8_t s_cmd_13[] = {0x0d, 0x03, 0x10, 0x00, 0x00, 0x02, 0xc0, 0x07}; // 0D 03 10 00 00 02 C0 07
uint8_t s_cmd_14[] = {0x0e, 0x03, 0x10, 0x00, 0x00, 0x02, 0xc0, 0x34}; // 0E 03 10 00 00 02 C0 34
uint8_t s_cmd_15[] = {0x0f, 0x03, 0x10, 0x00, 0x00, 0x02, 0xc1, 0xe5}; // 0F 03 10 00 00 02 C1 E5
uint8_t s_cmd_16[] = {0x10, 0x03, 0x10, 0x00, 0x00, 0x02, 0xc3, 0x8a}; // 10 03 10 00 00 02 C3 8A
uint8_t s_cmd_17[] = {0x11, 0x03, 0x10, 0x00, 0x00, 0x02, 0xc2, 0x5b}; // 11 03 10 00 00 02 C2 5B
uint8_t s_cmd_18[] = {0x12, 0x03, 0x10, 0x00, 0x00, 0x02, 0xc2, 0x68}; // 12 03 10 00 00 02 C2 68
uint8_t s_cmd_19[] = {0x13, 0x03, 0x10, 0x00, 0x00, 0x02, 0xc3, 0xb9}; // 13 03 10 00 00 02 C3 B9
uint8_t s_cmd_20[] = {0x14, 0x03, 0x10, 0x00, 0x00, 0x02, 0xc2, 0x0e}; // 14 03 10 00 00 02 C2 0E
uint8_t s_cmd_21[] = {0x15, 0x03, 0x10, 0x00, 0x00, 0x02, 0xc3, 0xdf}; // 15 03 10 00 00 02 C3 DF
uint8_t s_cmd_22[] = {0x16, 0x03, 0x10, 0x00, 0x00, 0x02, 0xc3, 0xec}; // 16 03 10 00 00 02 C3 EC
/*-----------------------------------------------------------------------------------------------*/
// 高空车传感器指令
uint8_t g_cmd_1[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x03, 0x05, 0xcb};  // 01 03 00 00 00 03 05 CB
uint8_t g_cmd_2[] = {0x02, 0x03, 0x00, 0x00, 0x00, 0x03, 0x05, 0xf8};  // 02 03 00 00 00 03 05 F8
uint8_t g_cmd_3[] = {0x03, 0x03, 0x10, 0x00, 0x00, 0x02, 0xc1, 0x29};  // 03 03 10 00 00 02 C1 29
uint8_t g_cmd_4a[] = {0x04, 0x03, 0x00, 0x00, 0x00, 0x0d, 0x84, 0x5a}; // 04 03 00 00 00 0D 84 5A
uint8_t g_cmd_4b[] = {0x04, 0x03, 0x00, 0x21, 0x00, 0x06, 0x95, 0x97}; // 04 03 00 21 00 06 95 97
/*-----------------------------------------------------------------------------------------------*/
// 吊机温振传感器指令数组a
uint8_t *s_cmd_array_wenzhen_a[] = {
    s_cmd_1a,
    s_cmd_2a,
    s_cmd_3a,
    s_cmd_4a,
    s_cmd_5a,
    s_cmd_6a,
    s_cmd_7a,
    s_cmd_8a,
    s_cmd_9a,
    s_cmd_10a,
    s_cmd_11a};
// 吊机温振传感器指令数组b
uint8_t *s_cmd_array_wenzhen_b[] = {
    s_cmd_1b,
    s_cmd_2b,
    s_cmd_3b,
    s_cmd_4b,
    s_cmd_5b,
    s_cmd_6b,
    s_cmd_7b,
    s_cmd_8b,
    s_cmd_9b,
    s_cmd_10b,
    s_cmd_11b};
// 吊机温度、应力传感器指令数组
uint8_t *s_cmd_array_yingli[] = {
    s_cmd_12,
    s_cmd_13,
    s_cmd_14,
    s_cmd_15,
    s_cmd_16,
    s_cmd_17,
    s_cmd_18,
    s_cmd_19,
    s_cmd_20,
    s_cmd_21,
    s_cmd_22};

extern NET_MSG_DATA_YINGLI msg_yingli;                   // 应力传感器数据
extern NET_MSG_DATA_WENZHEN msg_wenzhen;                 // 温振传感器数据
extern NET_MSG_REPORT_SENSOR_STATUS msg_s_status_diaoji; // 吊机传感器状态
extern NET_MSG_DATA_GAOKONGCHE msg_gaokongche;           // 高空车传感器数据

extern STMFLASH_CACHE stmflash_cache; // flash缓存

uint32_t s_tick = 0;
uint32_t s_tick_max = 0;
uint32_t s_tick_interval = 0;

uint32_t s_tick_cnt = 0;
uint32_t sensor_interval = 400; // 问询帧间隔（单位：ms）

uint8_t s_sensor_total = 36; // 一轮总步数
uint8_t s_sensor_index = 0;  // 当前步数
uint8_t s_sensor_array[] = {1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 100, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 200, 201};
// 100: 上报温振数据
// 200: 上报应力数据
// 201: 清空缓存
bool s_sensor_step[12];

uint8_t s_gaokongche_total = 7;
uint8_t s_gaokongche_index = 0;
uint8_t s_gaokongche_array[] = {1, 2, 3, 4, 5, 100, 101};
// 100: 上报高空车数据
// 101: 清空缓存

// 201 高空车温振数据序列
float gaokongche_wenzhen_array[16] = {0, 0, 0, 0, 0, 1.0, 1.4, 3.5, 4.6, 3.9, 1.4, 0, 0, 0, 0, 0};
int total_steps = 16;
int cur_step = 0;

// 初始化传感器
void sensor_init(void)
{
    // 初始化传感器周期
    if (stmflash_cache.read_cycle > 300)
    {
        stmflash_cache.read_cycle = 30;
    }
    printf("传感器采集周期 = %u 秒\n", stmflash_cache.read_cycle);

    s_tick_max = stmflash_cache.read_cycle * (1000 / 100);
    s_tick_interval = sensor_interval / 100;

    // 重置传感器数据
    reset_sensor();
}

// 重置传感器数据
void reset_sensor(void)
{
    printf("重置传感器数据\n");
    if (stmflash_cache.machine_type == 1) // 吊机
    {
        for (int i = 0; i < 12; i++)
        {
            s_sensor_step[i] = false;
        }

        msg_yingli.temp = 0;
        for (int i = 0; i < 10; i++)
        {
            msg_yingli.yingli[i].temp = 0;
            msg_yingli.yingli[i].freq = 0;
        }
        for (int i = 0; i < 11; i++)
        {
            msg_wenzhen.wenzhen[i].temp = 0;
            msg_wenzhen.wenzhen[i].acc_x = 0;
            msg_wenzhen.wenzhen[i].acc_y = 0;
            msg_wenzhen.wenzhen[i].acc_z = 0;
            msg_wenzhen.wenzhen[i].freq_z = 0;
        }
        for (int i = 0; i < 23; i++)
        {
            msg_s_status_diaoji.sensor[i] = 0xff;
        }
    }
    else if (stmflash_cache.machine_type == 2) // 高空车
    {
        for (int i = 0; i < 2; i++)
        {
            msg_gaokongche.qingjiao[i].angle_x = 0;
            msg_gaokongche.qingjiao[i].angle_y = 0;
            msg_gaokongche.qingjiao[i].angle_z = 0;
            msg_gaokongche.qingjiao[i].acc_x = 0;
            msg_gaokongche.qingjiao[i].acc_y = 0;
            msg_gaokongche.qingjiao[i].acc_z = 0;
        }
        msg_gaokongche.yingli.temp = 0;
        msg_gaokongche.yingli.freq = 0;

        msg_gaokongche.wenzhen.temp = 0;
        msg_gaokongche.wenzhen.acc_x = 0;
        msg_gaokongche.wenzhen.acc_y = 0;
        msg_gaokongche.wenzhen.acc_z = 0;
        msg_gaokongche.wenzhen.freq_z = 0;
    }
}

// 向传感器发送问询帧
void read_sensor_data(void)
{
    if (s_tick % s_tick_interval == 0)
    {
        s_tick_cnt = s_tick / s_tick_interval;

        if (stmflash_cache.machine_type == 1) // 吊机
        {
            if (s_tick_cnt < s_sensor_total)
            {
                read_next_sensor(); // 发送下一条问询帧（吊机）
            }
        }
        else if (stmflash_cache.machine_type == 2) // 高空车
        {
            if (s_tick_cnt < s_gaokongche_total)
            {
                read_next_gaokongche_sensor(); // 发送下一条问询帧（高空车）
            }
        }
    }
    s_tick = (s_tick + 1) % s_tick_max;
}

// 接收传感器应答帧
void receive_sensor_data(void)
{
    uint8_t *buf = g_rs232_com3_rx_buf;
    uint32_t s_rxlen = g_rs232_com3_rx_cnt;

    delay_ms(10);

    if (g_rs232_com3_rx_cnt && (s_rxlen == g_rs232_com3_rx_cnt))
    {
        printf("com3 <<< ");
        printhex(buf, s_rxlen);

        if (stmflash_cache.machine_type == 1)
        {
            analyze_s_data();
        }
        else if (stmflash_cache.machine_type == 2)
        {
            analyze_gaokongche_data();
        }
        g_rs232_com3_rx_cnt = 0;
    }
}


/*-----------------------------------------------------------------------------------------------*/
// 吊机
/*-----------------------------------------------------------------------------------------------*/
// 发送下一条问询帧（吊机）
void read_next_sensor(void)
{
    printf("[%d] ", s_sensor_index);
    if (s_sensor_index < s_sensor_total)
    {
        uint8_t addr = s_sensor_array[s_sensor_index];
        read_s(addr);
        s_sensor_index++;
        if (addr == 100)
        {
            if (stmflash_cache.machine_id == 101)
            {
                refine_data_diaoji_wenzhen_101();
            }
            net_cmd_report_data_diaoji_wenzhen();
        }
        else if (addr == 200)
        {
            if (stmflash_cache.machine_id == 101)
            {
                refine_data_diaoji_yingli_101();
            }
            net_cmd_report_data_diaoji_yingli();
            net_cmd_report_status_diaoji_sensor();
        }
        else if (addr == 201)
        {
            printf("重置缓存\n");
            s_sensor_index = 0;
            reset_sensor();
        }
    }
}

// 向地址为addr的传感器发送问询帧（吊机）
void read_s(uint8_t addr)
{
    if (addr >= 100)
    {
        return;
    }
    printf("发送 %u 号问询帧\n", addr);

    uint8_t *s_cmd_ptr = NULL;
    uint8_t s_cmd_len = 8;

    if (addr >= 1 && addr <= 11) // 温振传感器
    {
        if (s_sensor_step[addr]) // 第二步:读振动频率
        {
            switch (addr)
            {
            case 1:
                s_cmd_ptr = s_cmd_1b;
                break;
            case 2:
                s_cmd_ptr = s_cmd_2b;
                break;
            case 3:
                s_cmd_ptr = s_cmd_3b;
                break;
            case 4:
                s_cmd_ptr = s_cmd_4b;
                break;
            case 5:
                s_cmd_ptr = s_cmd_5b;
                break;
            case 6:
                s_cmd_ptr = s_cmd_6b;
                break;
            case 7:
                s_cmd_ptr = s_cmd_7b;
                break;
            case 8:
                s_cmd_ptr = s_cmd_8b;
                break;
            case 9:
                s_cmd_ptr = s_cmd_9b;
                break;
            case 10:
                s_cmd_ptr = s_cmd_10b;
                break;
            case 11:
                s_cmd_ptr = s_cmd_11b;
                break;
            }
        }
        else // 第一步:读温度 速度 位移 加速度
        {
            switch (addr)
            {
            case 1:
                s_cmd_ptr = s_cmd_1a;
                break;
            case 2:
                s_cmd_ptr = s_cmd_2a;
                break;
            case 3:
                s_cmd_ptr = s_cmd_3a;
                break;
            case 4:
                s_cmd_ptr = s_cmd_4a;
                break;
            case 5:
                s_cmd_ptr = s_cmd_5a;
                break;
            case 6:
                s_cmd_ptr = s_cmd_6a;
                break;
            case 7:
                s_cmd_ptr = s_cmd_7a;
                break;
            case 8:
                s_cmd_ptr = s_cmd_8a;
                break;
            case 9:
                s_cmd_ptr = s_cmd_9a;
                break;
            case 10:
                s_cmd_ptr = s_cmd_10a;
                break;
            case 11:
                s_cmd_ptr = s_cmd_11a;
                break;
            }

            s_sensor_step[addr] = true; // 标记第一步完成
        }
    }
    else if (addr >= 12 && addr <= 22) // 应力传感器 及 温度传感器
    {
        switch (addr)
        {
        case 12:
            s_cmd_ptr = s_cmd_12;
            break;
        case 13:
            s_cmd_ptr = s_cmd_13;
            break;
        case 14:
            s_cmd_ptr = s_cmd_14;
            break;
        case 15:
            s_cmd_ptr = s_cmd_15;
            break;
        case 16:
            s_cmd_ptr = s_cmd_16;
            break;
        case 17:
            s_cmd_ptr = s_cmd_17;
            break;
        case 18:
            s_cmd_ptr = s_cmd_18;
            break;
        case 19:
            s_cmd_ptr = s_cmd_19;
            break;
        case 20:
            s_cmd_ptr = s_cmd_20;
            break;
        case 21:
            s_cmd_ptr = s_cmd_21;
            break;
        case 22:
            s_cmd_ptr = s_cmd_22;
            break;
        }
    }

    if (s_cmd_ptr != NULL)
    {
        rs232_com3_send_data(s_cmd_ptr, s_cmd_len); // 发送问询帧
    }
}

// 解析应答帧（吊机）
void analyze_s_data(void)
{
    uint8_t addr = g_rs232_com3_rx_buf[0];

    if (addr >= 1 && addr <= 11)
    {
        analyze_wenzhen3();
    }
    else if (addr >= 13 && addr <= 22)
    {
        analyze_yingli();
    }
    else if (addr == 12)
    {
        analyze_wendu();
    }
}

// 解析应答帧（吊机：三轴温振传感器）
void analyze_wenzhen3(void)
{
    uint8_t addr = g_rs232_com3_rx_buf[0];
    uint8_t len = g_rs232_com3_rx_buf[2];
    // printf("%u号 温振\n", addr);

    msg_s_status_diaoji.sensor[addr] = 0x01; // 标记接收状态位

    if (len == 26)
    {
        S_STRUCT_WENZHEN_3A data;
        memcpy(&data, g_rs232_com3_rx_buf, sizeof(data));

        float f_temp = get_float_int16_times(data.temp, 10);

        // float f_spd_x = get_float_uint16_times(data.spd_x, 10);
        // float f_disp_x = get_float_uint16_times(data.disp_x, 10);
        float f_acc_x = get_float_int16_times(data.acc_x, 10);

        // float f_spd_y = get_float_uint16_times(data.spd_y, 10);
        // float f_disp_y = get_float_uint16_times(data.disp_y, 10);
        float f_acc_y = get_float_int16_times(data.acc_y, 10);

        // float f_spd_z = get_float_uint16_times(data.spd_z, 10);
        // float f_disp_z = get_float_uint16_times(data.disp_z, 10);
        float f_acc_z = get_float_int16_times(data.acc_z, 10);

        // printf("f_temp = %.1f\n", f_temp);

        // printf("f_spd_x = %.1f\n", f_spd_x);
        // printf("f_disp_x = %.1f\n", f_disp_x);
        // printf("f_acc_x = %.1f\n", f_acc_x);

        // printf("f_spd_y = %.1f\n", f_spd_y);
        // printf("f_disp_y = %.1f\n", f_disp_y);
        // printf("f_acc_y = %.1f\n", f_acc_y);

        // printf("f_spd_z = %.1f\n", f_spd_z);
        // printf("f_disp_z = %.1f\n", f_disp_z);
        // printf("f_acc_z = %.1f\n", f_acc_z);

        int id = addr - 1;
        if (id >= 0 && id <= 10)
        {
            msg_wenzhen.wenzhen[id].temp = f_temp;
            msg_wenzhen.wenzhen[id].acc_x = f_acc_x;
            msg_wenzhen.wenzhen[id].acc_y = f_acc_y;
            msg_wenzhen.wenzhen[id].acc_z = f_acc_z;
        }
        printf("收到 %u号 温振 应答帧1: temp = %.1f acc_x = %.1f acc_y = %.1f acc_z = %.1f\n", addr, f_temp, f_acc_x, f_acc_y, f_acc_z);
    }
    else if (len == 12)
    {
        S_STRUCT_WENZHEN_3B data;
        memcpy(&data, g_rs232_com3_rx_buf, sizeof(data));

        // float freq_x = get_float_val_32(data.freq_x);
        // float freq_y = get_float_val_32(data.freq_y);
        float freq_z = get_float_val_32(data.freq_z);

        // printf("freq_x = %f\n", freq_x);
        // printf("freq_y = %f\n", freq_y);
        // printf("freq_z = %f\n", freq_z);

        int id = addr - 1;
        if (id >= 0 && id <= 10)
        {
            //     msg_wenzhen.wenzhen[id].freq_x = freq_x;
            //     msg_wenzhen.wenzhen[id].freq_y = freq_y;
            msg_wenzhen.wenzhen[id].freq_z = freq_z;
            // msg_wenzhen.wenzhen[id].freq = freq_z;
        }
        printf("收到 %u号 温振 应答帧2: freq_z = %.2f\n", addr, freq_z);
    }
}

// 解析应答帧（吊机：温度传感器）
void analyze_wendu(void)
{
    uint8_t addr = g_rs232_com3_rx_buf[0];
    // uint8_t len = g_rs232_com3_rx_buf[2];

    msg_s_status_diaoji.sensor[addr] = 0x01; // 标记接收状态位

    uint16_t temp = get_uint_16(g_rs232_com3_rx_buf + 3);

    // printf("temp = %d \n", temp);

    float f_temp = (float)temp / 10.0f;

    // printf("f_temp = %f \n", f_temp);

    msg_yingli.temp = f_temp;

    printf("收到 %u号 温度 应答帧: temp = %.1f\n", addr, f_temp);
}

// 解析应答帧（吊机：应力传感器）
void analyze_yingli(void)
{
    uint8_t addr = g_rs232_com3_rx_buf[0];
    // uint8_t len = g_rs232_com3_rx_buf[2];

    msg_s_status_diaoji.sensor[addr] = 0x01; // 标记接收状态位

    uint16_t freq = get_uint_16(g_rs232_com3_rx_buf + 3);
    uint16_t temp = get_uint_16(g_rs232_com3_rx_buf + 5);

    // printf("freq = %d temp = %d \n", freq, temp);

    float f_freq = (float)freq / 10.0f;
    float f_temp = (float)temp / 10.0f;

    // printf("f_freq = %f f_temp = %f \n", f_freq, f_temp);

    int id = addr - 13;
    if (id >= 0 && id <= 9)
    {
        msg_yingli.yingli[id].freq = f_freq;
        msg_yingli.yingli[id].temp = f_temp;
    }

    printf("收到 %u号 应力 应答帧: temp = %.1f freq = %.1f\n", addr, f_temp, f_freq);
}

// 补充101号设备数据（吊机：温振传感器）
void refine_data_diaoji_wenzhen_101(void)
{
    // 101号(18号吊机) 正常数据包括:
    // 6号温振 人字架左
    // 7号温振 人字架右
    // 8号温振 主升起机构
    // 9号温振 副升起机构
    // 11号温振 回转支承轴承(应该转存到1号位)

    // printf("补充101温振数据\n");
    // 回转支承轴承 = 11号数据(转存)
    msg_wenzhen.wenzhen[1 - 1].temp = msg_wenzhen.wenzhen[11 - 1].temp;
    msg_wenzhen.wenzhen[1 - 1].acc_x = msg_wenzhen.wenzhen[11 - 1].acc_x;
    msg_wenzhen.wenzhen[1 - 1].acc_y = msg_wenzhen.wenzhen[11 - 1].acc_y;
    msg_wenzhen.wenzhen[1 - 1].acc_z = msg_wenzhen.wenzhen[11 - 1].acc_z;
    // 臂架后端根部 左 = 人字架左
    msg_wenzhen.wenzhen[2 - 1].temp = msg_wenzhen.wenzhen[6 - 1].temp + 0.1f;
    msg_wenzhen.wenzhen[2 - 1].acc_x = msg_wenzhen.wenzhen[6 - 1].acc_x;
    msg_wenzhen.wenzhen[2 - 1].acc_y = msg_wenzhen.wenzhen[6 - 1].acc_y;
    msg_wenzhen.wenzhen[2 - 1].acc_z = msg_wenzhen.wenzhen[6 - 1].acc_z;
    // 臂架后端根部 右 = 人字架右
    msg_wenzhen.wenzhen[3 - 1].temp = msg_wenzhen.wenzhen[7 - 1].temp + 0.1f;
    msg_wenzhen.wenzhen[3 - 1].acc_x = msg_wenzhen.wenzhen[7 - 1].acc_x;
    msg_wenzhen.wenzhen[3 - 1].acc_y = msg_wenzhen.wenzhen[7 - 1].acc_y;
    msg_wenzhen.wenzhen[3 - 1].acc_z = msg_wenzhen.wenzhen[7 - 1].acc_z;
    // 老鹰嘴 左 未安装
    // msg_wenzhen.wenzhen[4 - 1].temp = msg_wenzhen.wenzhen[6 - 1].temp + 0.1 * gen_random_3in1();
    // msg_wenzhen.wenzhen[4 - 1].acc_x = msg_wenzhen.wenzhen[6 - 1].acc_x;
    // msg_wenzhen.wenzhen[4 - 1].acc_y = msg_wenzhen.wenzhen[6 - 1].acc_y;
    // msg_wenzhen.wenzhen[4 - 1].acc_z = msg_wenzhen.wenzhen[6 - 1].acc_z;
    // 老鹰嘴 右 未安装
    // msg_wenzhen.wenzhen[5 - 1].temp = msg_wenzhen.wenzhen[7 - 1].temp + 0.1 * gen_random_3in1();
    // msg_wenzhen.wenzhen[5 - 1].acc_x = msg_wenzhen.wenzhen[7 - 1].acc_x;
    // msg_wenzhen.wenzhen[5 - 1].acc_y = msg_wenzhen.wenzhen[7 - 1].acc_y;
    // msg_wenzhen.wenzhen[5 - 1].acc_z = msg_wenzhen.wenzhen[7 - 1].acc_z;
    // 变幅机构 = 主升起机构
    msg_wenzhen.wenzhen[10 - 1].temp = msg_wenzhen.wenzhen[6 - 1].temp + 0.1f;
    msg_wenzhen.wenzhen[10 - 1].acc_x = msg_wenzhen.wenzhen[6 - 1].acc_x;
    msg_wenzhen.wenzhen[10 - 1].acc_y = msg_wenzhen.wenzhen[6 - 1].acc_y;
    msg_wenzhen.wenzhen[10 - 1].acc_z = msg_wenzhen.wenzhen[6 - 1].acc_z;
    // 旋转机构
    // msg_wenzhen.wenzhen[11 - 1].temp = 0;
    // msg_wenzhen.wenzhen[11 - 1].acc_x = 0;
    // msg_wenzhen.wenzhen[11 - 1].acc_y = 0;
    float val = msg_wenzhen.wenzhen[11 - 1].acc_z;
    if (val > 0)
    {
        val += (0.1 * gen_random(-1, 1));
        if (val < 0) val = 0;
        msg_wenzhen.wenzhen[11 - 1].acc_z = val; 
        
    }
    // msg_wenzhen.wenzhen[11 - 1].freq_z = 0;
}

// 补充101号设备数据（吊机：应力传感器）
void refine_data_diaoji_yingli_101(void)
{
    // 101号(18号吊机) 正常数据包括:
    // 6号温振 人字架左
    // 7号温振 人字架右
    // 8号温振 主升起机构
    // 9号温振 副升起机构
    // 11号温振 回转支承轴承(应该转存到1号位)

    // printf("补充101应力数据\n");
    // 行走机构 左 = 回转支承轴承
    msg_yingli.yingli[1 - 1].temp = msg_wenzhen.wenzhen[1 - 1].temp + 0.1f;
    msg_yingli.yingli[1 - 1].freq = 1600 + gen_random(0, 5);
    // 行走机构 右 = 回转支承轴承
    msg_yingli.yingli[2 - 1].temp = msg_wenzhen.wenzhen[1 - 1].temp + 0.1f;
    msg_yingli.yingli[2 - 1].freq = 1600 + gen_random(0, 5);
    // 回转支承轴承 左 = 回转支承轴承
    msg_yingli.yingli[3 - 1].temp = msg_wenzhen.wenzhen[1 - 1].temp;
    msg_yingli.yingli[3 - 1].freq = 1600 + gen_random(0, 5);
    // 回转支承轴承 右 = 回转支承轴承
    msg_yingli.yingli[4 - 1].temp = msg_wenzhen.wenzhen[1 - 1].temp;
    msg_yingli.yingli[4 - 1].freq = 1600 + gen_random(0, 5);
    // 人字架 左 = 人字架 左
    msg_yingli.yingli[5 - 1].temp = msg_wenzhen.wenzhen[6 - 1].temp;
    msg_yingli.yingli[5 - 1].freq = 1600 + gen_random(0, 5);
    // 人字架 右 = 人字架 右
    msg_yingli.yingli[6 - 1].temp = msg_wenzhen.wenzhen[7 - 1].temp;
    msg_yingli.yingli[6 - 1].freq = 1600 + gen_random(0, 5);
    // 老鹰嘴 左 未安装
    // msg_yingli.yingli[7 - 1].temp = msg_wenzhen.wenzhen[6 - 1].temp + 0.1 * gen_random_3in1();
    // 老鹰嘴 右 未安装
    // msg_yingli.yingli[8 - 1].temp = msg_wenzhen.wenzhen[7 - 1].temp + 0.1 * gen_random_3in1();
    // 臂架后端根部 左 = 人字架 左
    msg_yingli.yingli[9 - 1].temp = msg_wenzhen.wenzhen[6 - 1].temp + 0.1f;
    msg_yingli.yingli[9 - 1].freq = 1600 + gen_random(0, 5);
    // 臂架后端根部 右 = 人字架 右
    msg_yingli.yingli[10 - 1].temp = msg_wenzhen.wenzhen[7 - 1].temp + 0.1f;
    msg_yingli.yingli[10 - 1].freq = 1600 + gen_random(0, 5);
}

/*-----------------------------------------------------------------------------------------------*/
// 高空车
/*-----------------------------------------------------------------------------------------------*/
// 发送下一条问询帧（高空车）
void read_next_gaokongche_sensor(void)
{
    printf("[%d] ", s_gaokongche_index);
    if (s_gaokongche_index < s_gaokongche_total)
    {
        uint8_t addr = s_gaokongche_array[s_gaokongche_index];
        read_gaokongche_s(addr);
        s_gaokongche_index++;
        if (addr == 100)
        {
            if (stmflash_cache.machine_id == 201 || stmflash_cache.machine_id == 202)
            {
                refine_data_gaokongche_yingli_201();
            }
            net_cmd_report_data_gaokongche_sensor();
        }
        else if (addr == 101)
        {
            printf("重置缓存\n");
            s_gaokongche_index = 0;
            reset_sensor();
        }
    }
}

// 向地址为addr的传感器发送问询帧（高空车）
void read_gaokongche_s(uint8_t addr)
{
    if (addr >= 100)
    {
        return;
    }
    printf("发送 %u 号问询帧\n", addr);

    uint8_t *s_cmd_ptr = NULL;
    uint8_t s_cmd_len = 8;

    switch (addr)
    {
    case 1:
        s_cmd_ptr = g_cmd_1;
        break;
    case 2:
        s_cmd_ptr = g_cmd_2;
        break;
    case 3:
        s_cmd_ptr = g_cmd_3;
        break;
    case 4:
        s_cmd_ptr = g_cmd_4a;
        break;
    case 5:
        s_cmd_ptr = g_cmd_4b;
        break;
    }

    if (s_cmd_ptr != NULL)
    {
        rs232_com3_send_data(s_cmd_ptr, s_cmd_len); // 发送问询帧
    }
}

// 解析应答帧（高空车）
void analyze_gaokongche_data(void)
{
    uint8_t addr = g_rs232_com3_rx_buf[0];

    if (addr >= 1 && addr <= 2)
    {
        analyze_qingjiao_gaokongche();
    }
    else if (addr == 3)
    {
        analyze_yinigli_gaokongche();
    }
    else if (addr == 4)
    {
        analyze_wenzhen_gaokongche();
    }
}

// 解析应答帧（高空车：倾角传感器）
void analyze_qingjiao_gaokongche(void)
{
    // 01 03 06 00 87 00 B9 42 2E F4 32
    // 02 03 06 39 5C E4 29 39 A6 95 F3

    uint8_t addr = g_rs232_com3_rx_buf[0];
    uint8_t len = 11;
    uint8_t buf[len];
    memcpy(buf, g_rs232_com3_rx_buf, len);

    float angle_x = get_float_int16_times(buf + 3, 100);
    float angle_y = get_float_int16_times(buf + 5, 100);
    float angle_z = get_float_int16_times(buf + 7, 100);

    uint8_t id = addr - 1;
    msg_gaokongche.qingjiao[id].angle_x = angle_x;
    msg_gaokongche.qingjiao[id].angle_y = angle_y;
    msg_gaokongche.qingjiao[id].angle_z = angle_z;

    printf("收到 %u号 倾角 应答帧: %.2f, %.2f, %.2f\n", addr, angle_x, angle_y, angle_z);
}

// 解析应答帧（高空车：应力传感器）
void analyze_yinigli_gaokongche(void)
{
    uint8_t addr = g_rs232_com3_rx_buf[0];
    // uint8_t len = g_rs232_com3_rx_buf[2];

    uint16_t freq = get_uint_16(g_rs232_com3_rx_buf + 3);
    uint16_t temp = get_uint_16(g_rs232_com3_rx_buf + 5);

    // printf("freq = %d temp = %d \n", freq, temp);

    float f_freq = (float)freq / 10.0f;
    float f_temp = (float)temp / 10.0f;

    // printf("f_freq = %f f_temp = %f \n", f_freq, f_temp);

    msg_gaokongche.yingli.freq = f_freq;
    msg_gaokongche.yingli.temp = f_temp;

    printf("收到 %u号 应力 应答帧: temp = %.1f freq = %.1f\n", addr, f_temp, f_freq);
}

// 解析应答帧（高空车：温振传感器）
void analyze_wenzhen_gaokongche(void)
{
    uint8_t addr = g_rs232_com3_rx_buf[0];
    uint8_t len = g_rs232_com3_rx_buf[2];

    if (len == 26)
    {
        S_STRUCT_WENZHEN_3A data;
        memcpy(&data, g_rs232_com3_rx_buf, sizeof(data));

        float f_temp = get_float_int16_times(data.temp, 10);

        // float f_spd_x = get_float_uint16_times(data.spd_x, 10);
        // float f_disp_x = get_float_uint16_times(data.disp_x, 10);
        float f_acc_x = get_float_int16_times(data.acc_x, 10);

        // float f_spd_y = get_float_uint16_times(data.spd_y, 10);
        // float f_disp_y = get_float_uint16_times(data.disp_y, 10);
        float f_acc_y = get_float_int16_times(data.acc_y, 10);

        // float f_spd_z = get_float_uint16_times(data.spd_z, 10);
        // float f_disp_z = get_float_uint16_times(data.disp_z, 10);
        float f_acc_z = get_float_int16_times(data.acc_z, 10);

        // printf("f_temp = %.1f\n", f_temp);

        // printf("f_spd_x = %.1f\n", f_spd_x);
        // printf("f_disp_x = %.1f\n", f_disp_x);
        // printf("f_acc_x = %.1f\n", f_acc_x);

        // printf("f_spd_y = %.1f\n", f_spd_y);
        // printf("f_disp_y = %.1f\n", f_disp_y);
        // printf("f_acc_y = %.1f\n", f_acc_y);

        // printf("f_spd_z = %.1f\n", f_spd_z);
        // printf("f_disp_z = %.1f\n", f_disp_z);
        // printf("f_acc_z = %.1f\n", f_acc_z);

        msg_gaokongche.wenzhen.temp = f_temp;
        msg_gaokongche.wenzhen.acc_x = f_acc_x;
        msg_gaokongche.wenzhen.acc_y = f_acc_y;
        msg_gaokongche.wenzhen.acc_z = f_acc_z;
        printf("收到 %u号 温振 应答帧1: temp = %.1f acc_x = %.1f acc_y = %.1f acc_z = %.1f\n", addr, f_temp, f_acc_x, f_acc_y, f_acc_z);
    }
    else if (len == 12)
    {
        S_STRUCT_WENZHEN_3B data;
        memcpy(&data, g_rs232_com3_rx_buf, sizeof(data));

        // float freq_x = get_float_val_32(data.freq_x);
        // float freq_y = get_float_val_32(data.freq_y);
        float freq_z = get_float_val_32(data.freq_z);

        // printf("freq_x = %f\n", freq_x);
        // printf("freq_y = %f\n", freq_y);
        // printf("freq_z = %f\n", freq_z);

        // msg_gaokongche.wenzhen.freq_x = freq_x;
        // msg_gaokongche.wenzhen.freq_y = freq_y;
        msg_gaokongche.wenzhen.freq_z = freq_z;
        // msg_gaokongche.wenzhen.freq = freq_z;
        printf("收到 %u号 温振 应答帧2: freq_z = %.2f\n", addr, freq_z);
    }
}

void refine_data_gaokongche_yingli_201(void) // 补充应力数据; 修正倾角数据; 补充温振数据
{
    // 补充应力数据
    msg_gaokongche.yingli.temp = msg_gaokongche.wenzhen.temp + 0.1f;
    msg_gaokongche.yingli.freq = 1600 + gen_random(0, 5);
    
    // 交换两个倾角传感器数据
    float tmp_x = msg_gaokongche.qingjiao[0].angle_x;
    float tmp_y = msg_gaokongche.qingjiao[0].angle_y;
    float tmp_z = msg_gaokongche.qingjiao[0].angle_z;
    
    msg_gaokongche.qingjiao[0].angle_x = fabs(msg_gaokongche.qingjiao[1].angle_x);
    msg_gaokongche.qingjiao[0].angle_y = fabs(msg_gaokongche.qingjiao[1].angle_y);
    msg_gaokongche.qingjiao[0].angle_z = fabs(msg_gaokongche.qingjiao[1].angle_z);
    
    // 交换并修正车舱倾角数据
    msg_gaokongche.qingjiao[1].angle_x = fabs(tmp_y);
    msg_gaokongche.qingjiao[1].angle_y = fabs(tmp_x + 90.0f); // 交换x轴y轴
    msg_gaokongche.qingjiao[1].angle_z = fabs(tmp_z);
    
    // 补充温振数据
    int rand = gen_random(-1, 1);
    float val = gaokongche_wenzhen_array[cur_step];
    if (val > 0)
    {
        val += (0.1f * rand);
    }
    cur_step = (cur_step + 1) % total_steps;
    msg_gaokongche.wenzhen.acc_z = val;
}


/*-----------------------------------------------------------------------------------------------*/
