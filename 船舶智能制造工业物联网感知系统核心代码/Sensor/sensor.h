#ifndef __SENSOR_H
#define __SENSOR_H

#include "stdio.h"
#include "stdint.h"

void sensor_init(void);
void reset_sensor(void);
void read_sensor_data(void);
void receive_sensor_data(void);
void read_next_sensor(void);
void read_s(uint8_t addr);
void analyze_s_data(void);

void analyze_wenzhen3(void); // 解析三轴温振传感器响应帧
void analyze_wendu(void);    // 解析温度传感器响应帧
void analyze_yingli(void);   // 解析应力传感器响应帧

void read_next_gaokongche_sensor(void);
void read_gaokongche_s(uint8_t addr);
void analyze_gaokongche_data(void);

void analyze_qingjiao_gaokongche(void);
void analyze_yinigli_gaokongche(void);
void analyze_wenzhen_gaokongche(void);

void refine_data_diaoji_wenzhen_101(void); // 补充温振数据
void refine_data_diaoji_yingli_101(void);  // 补充应力数据

void refine_data_gaokongche_yingli_201(void);

typedef struct __attribute__((packed))
{
    uint8_t addr;
    uint8_t func;
    uint8_t len;

    uint8_t temp[2];
    uint8_t spd_x[2];
    uint8_t spd_y[2];
    uint8_t spd_z[2];
    uint8_t disp_x[2];
    uint8_t disp_y[2];
    uint8_t disp_z[2];
    uint8_t none_0[4];
    uint8_t version[2];
    uint8_t acc_x[2];
    uint8_t acc_y[2];
    uint8_t acc_z[2];

    uint8_t checksum[2];

} S_STRUCT_WENZHEN_3A;

typedef struct __attribute__((packed))
{
    uint8_t addr;
    uint8_t func;
    uint8_t len;

    uint8_t freq_x[4];
    uint8_t freq_y[4];
    uint8_t freq_z[4];

    uint8_t checksum[2];

} S_STRUCT_WENZHEN_3B;

#endif
