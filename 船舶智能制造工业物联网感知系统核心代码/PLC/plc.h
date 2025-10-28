#ifndef __PLC_H
#define __PLC_H

#include "stdint.h"
#include "stdbool.h"

// PLC�Ĵ�����ȡָ��
#define read_plc_0 "@00RR0000000141*\r*\r"
#define read_plc_1 "@00RR0001000140*\r*\r"
#define read_plc_2 "@00RR0002000143*\r*\r"
#define read_plc_3 "@00RR0003000142*\r*\r"
#define read_plc_4 "@00RR0004000145*\r*\r"
#define read_plc_5 "@00RR0005000144*\r*\r"
#define read_plc_6 "@00RR0006000147*\r*\r"
#define read_plc_7 "@00RR0007000146*\r*\r"
#define read_plc_8 "@00RR0008000149*\r*\r"
#define read_plc_9 "@00RR0009000148*\r*\r"
#define read_plc_10 "@00RR0010000140*\r*\r"
#define read_plc_11 "@00RR0011000141*\r*\r"

#define res_plc_head "@00RR00" // PLC��������ͷ

#define debug_node_0 "@00RR0001000140*\r*\r"
#define debug_node_1 "@01RR0001000141*\r*\r"
#define debug_node_2 "@02RR0001000142*\r*\r"
#define debug_node_3 "@03RR0001000143*\r*\r"
#define debug_node_4 "@04RR0001000144*\r*\r"
#define debug_node_5 "@05RR0001000145*\r*\r"
#define debug_node_6 "@06RR0001000146*\r*\r"
#define debug_node_7 "@07RR0001000147*\r*\r"
#define debug_node_8 "@08RR0001000148*\r*\r"
#define debug_node_9 "@09RR0001000149*\r*\r"
#define debug_node_10 "@0ARR0001000131*\r*\r"
#define debug_node_11 "@0BRR0001000132*\r*\r"
#define debug_node_12 "@0CRR0001000132*\r*\r"
#define debug_node_13 "@0DRR0001000132*\r*\r"
#define debug_node_14 "@0ERR0001000132*\r*\r"
#define debug_node_15 "@0FRR0001000132*\r*\r"
#define debug_node_16 "@10RR0001000132*\r*\r"
#define debug_node_17 "@11RR0001000132*\r*\r"
#define debug_node_18 "@12RR0001000132*\r*\r"
#define debug_node_19 "@13RR0001000132*\r*\r"
#define debug_node_20 "@14RR0001000132*\r*\r"
#define debug_node_21 "@15RR0001000132*\r*\r"
#define debug_node_22 "@15RR0001000132*\r*\r"
#define debug_node_23 "@15RR0001000132*\r*\r"
#define debug_node_24 "@15RR0001000132*\r*\r"
#define debug_node_25 "@15RR0001000132*\r*\r"
#define debug_node_26 "@15RR0001000132*\r*\r"
#define debug_node_27 "@15RR0001000132*\r*\r"
#define debug_node_28 "@15RR0001000132*\r*\r"
#define debug_node_29 "@15RR0001000132*\r*\r"
#define debug_node_30 "@15RR0001000132*\r*\r"

void plc_init(void);          // ��ʼ������
void read_plc_data(void);     // ��ȡPLC����
void receive_plc_data(void);  // ����PLC����
bool check_plc_data(void);    // ���PLC���ݸ�ʽ
void read_next_plc_reg(void); // ������һ����ȡָ��
void read_reg(uint8_t reg);   // ��ָ���Ĵ���
void analyze_plc_data(void);  // ����PLC����
void reset_plc_data(void);    // ����PLC���ݻ���
void debug_find_node_id(void);
void refine_plc_data(void);

typedef struct __attribute__((packed))
{
    uint8_t data[20];
} STRUCT_PLC_DATA;

/*

0       11.07   ���Ӵ����м�
1       0.11    ������������λ
2       9.09    ��������������λ
3       0.12    �������½���λ
4       1.11    ������������λ
5       9.10    ��������������λ
6       1.12    �������½���λ
7       2.11    ���������λ
8       9.11    �������������λ
9       2.12    ���������λ
10      2.13    ����ŵ���λ
11      4.09    ����ê����λ
12      4.05    ������λ
13      4.06    ������λ
14      5.07    �����ź�
15      0.06    ��/��������Ƶ������
16      2.05    �����Ƶ������
17      3.05    ��ת��Ƶ������
18      3.13    ���߱�Ƶ������
19      4.07    �����ն���λ

*/

/* 0 1 2 3 4 5 9 11 */

#endif
