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

void analyze_wenzhen3(void); // �����������񴫸�����Ӧ֡
void analyze_wendu(void);    // �����¶ȴ�������Ӧ֡
void analyze_yingli(void);   // ����Ӧ����������Ӧ֡

void read_next_gaokongche_sensor(void);
void read_gaokongche_s(uint8_t addr);
void analyze_gaokongche_data(void);

void analyze_qingjiao_gaokongche(void);
void analyze_yinigli_gaokongche(void);
void analyze_wenzhen_gaokongche(void);

void refine_data_diaoji_wenzhen_101(void); // ������������
void refine_data_diaoji_yingli_101(void);  // ����Ӧ������

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
