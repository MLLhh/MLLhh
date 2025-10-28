#ifndef __PLC_H
#define __PLC_H

#include "stdint.h"
#include "stdbool.h"

// PLC寄存器读取指令
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

#define res_plc_head "@00RR00" // PLC返回数据头

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

void plc_init(void);          // 初始化参数
void read_plc_data(void);     // 读取PLC数据
void receive_plc_data(void);  // 接收PLC数据
bool check_plc_data(void);    // 检查PLC数据格式
void read_next_plc_reg(void); // 发送下一条读取指令
void read_reg(uint8_t reg);   // 读指定寄存器
void analyze_plc_data(void);  // 解析PLC数据
void reset_plc_data(void);    // 重置PLC数据缓存
void debug_find_node_id(void);
void refine_plc_data(void);

typedef struct __attribute__((packed))
{
    uint8_t data[20];
} STRUCT_PLC_DATA;

/*

0       11.07   主接触器中继
1       0.11    主起升上升限位
2       9.09    主起升上升极限位
3       0.12    主起升下降限位
4       1.11    副起升上升限位
5       9.10    副起升上升极限位
6       1.12    副起升下降限位
7       2.11    变幅减幅限位
8       9.11    变幅减幅极限限位
9       2.12    变幅增幅限位
10      2.13    变幅放到限位
11      4.09    行走锚定限位
12      4.05    左行限位
13      4.06    有行限位
14      5.07    超载信号
15      0.06    主/副起升变频器故障
16      2.05    变幅变频器故障
17      3.05    旋转变频器故障
18      3.13    行走变频器故障
19      4.07    电缆终端限位

*/

/* 0 1 2 3 4 5 9 11 */

#endif
