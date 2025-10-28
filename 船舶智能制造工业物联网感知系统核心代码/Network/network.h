#ifndef __NETWORK_H
#define __NETWORK_H

#include "./SYSTEM/sys/sys.h"
#include "PLC/plc.h"
#include "stdbool.h"
/*
 * 通用指令目录(上行)
 * ee 01: 报告app启动
 * ee 02: 报告即将重启
 * ee 03: 上报app版本号
 *
 * 通用指令目录(下行)
 * dd 05: 撤销app待升级
 * dd 06: 询问app版本号
 * dd 07: 修改传感器口波特率为9600
 * dd 08: 修改传感器口波特率为115200
 * dd 09: 开启上报传感器状态
 * dd 0a: 关闭上报传感器状态
 * dd 0b: 开启上报PLC状态
 * dd 0c: 关闭上报PLC状态
 */
/************************************************************/
/* 网络数据结构 */

typedef struct __attribute__((packed))
{
    uint16_t header;       // 帧头       0xaa 0x55
    uint8_t frame_type;    // 帧类型
    uint8_t sub_type;      // 子类型
    uint32_t frame_length; // 帧长度
    uint16_t machine_type; // 设备类型
    uint16_t machine_id;   // 设备ID
    uint16_t node_id;      // 采集单元ID
} NET_MSG_HEAD;            // 通用帧结构:前缀

typedef struct __attribute__((packed))
{
    uint16_t header;       // 帧头        0xaa 0x55
    uint8_t frame_type;    // 帧类型      0xa2
    uint8_t sub_type;      // 子类型
    uint32_t frame_length; // 帧长度      0x10 0x00 0x00 0x00
    uint16_t machine_type; // 设备类型
    uint16_t machine_id;   // 设备ID
    uint16_t node_id;      // 采集单元ID
    uint16_t tail;         // 帧尾        0x0a 0xbd
} NET_MSG;                 // 服务器->客户端 通用指令

typedef struct __attribute__((packed))
{
    float temp;
    float freq;
} STRUCT_YINGLI; // 应力计数据结构

typedef struct __attribute__((packed))
{
    uint16_t header;       // 帧头       0xaa 0x55
    uint8_t frame_type;    // 帧类型      0x02
    uint8_t sub_type;      // 子类型      0x01
    uint32_t frame_length; // 帧长度
    uint16_t machine_type; // 设备类型     0x01
    uint16_t machine_id;   // 设备ID
    uint16_t node_id;      // 采集单元ID

    uint16_t group_type; // 传感器组类别   0x01
    uint16_t group_id;   // 传感器组ID     0x01
    uint32_t timestamp;  // 时间戳

    float temp;               // 温度传感器
    STRUCT_YINGLI yingli[10]; // 应力传感器

    uint16_t tail;     // 帧尾       0x0a 0xbd
} NET_MSG_DATA_YINGLI; // 客户端->服务器:上报数据(第1组) 吊机-应力

typedef struct __attribute__((packed))
{
    float temp;
    float acc_x;
    float acc_y;
    float acc_z; // 服务器取这个值
    float freq_z;
} STRUCT_WENZHEN; // 温振传感器数据结构

typedef struct __attribute__((packed))
{
    uint16_t header;       // 帧头       0xaa 0x55
    uint8_t frame_type;    // 帧类型      0x02
    uint8_t sub_type;      // 子类型      0x01
    uint32_t frame_length; // 帧长度
    uint16_t machine_type; // 设备类型     0x01
    uint16_t machine_id;   // 设备ID
    uint16_t node_id;      // 采集单元ID

    uint16_t group_type; // 传感器组类别   0x02
    uint16_t group_id;   // 传感器组ID     0x01
    uint32_t timestamp;  // 时间戳

    STRUCT_WENZHEN wenzhen[11]; // 温振传感器

    uint16_t tail;      // 帧尾       0x0a 0xbd
} NET_MSG_DATA_WENZHEN; // 客户端->服务器:上报数据(第2组) 吊机-温振

typedef struct __attribute__((packed))
{
    uint16_t header;       // 帧头
    uint8_t frame_type;    // 帧类型
    uint8_t sub_type;      // 子类型
    uint32_t frame_length; // 帧长度
    uint16_t machine_type; // 设备类型 0x01
    uint16_t machine_id;   // 设备ID
    uint16_t node_id;      // 采集单元ID

    uint16_t group_type; // 传感器组类别 0x03
    uint16_t group_id;   // 传感器组ID 0x01
    uint32_t timestamp;  // 时间戳

    uint8_t data[20]; // PLC寄存器数据

    uint16_t tail;  // 帧尾
} NET_MSG_DATA_PLC; // 客户端->服务器:上报数据(第3组) 吊机-PLC

typedef struct __attribute__((packed))
{
    float angle_x;
    float angle_y;
    float angle_z;
    float acc_x;
    float acc_y;
    float acc_z;
} STRUCT_QINGJIAO; // 倾角传感器数据结构

typedef struct __attribute__((packed))
{
    uint16_t header;       // 帧头
    uint8_t frame_type;    // 帧类型
    uint8_t sub_type;      // 子类型
    uint32_t frame_length; // 帧长度
    uint16_t machine_type; // 设备类型 0x02
    uint16_t machine_id;   // 设备ID
    uint16_t node_id;      // 采集单元ID

    uint16_t group_type; // 传感器组类别 0x04
    uint16_t group_id;   // 传感器组ID 0x01
    uint32_t timestamp;  // 时间戳

    STRUCT_QINGJIAO qingjiao[2]; // 倾角传感器
    STRUCT_YINGLI yingli;        // 应力传感器
    STRUCT_WENZHEN wenzhen;      // 温振传感器

    uint16_t tail; // 帧尾

} NET_MSG_DATA_GAOKONGCHE; // 客户端->服务器:上报数据(第4组) 高空车-传感器

typedef struct __attribute__((packed))
{
    uint16_t header;       // 帧头
    uint8_t frame_type;    // 帧类型
    uint8_t sub_type;      // 子类型
    uint32_t frame_length; // 帧长度
    uint16_t machine_type; // 设备类型
    uint16_t machine_id;   // 设备ID
    uint16_t node_id;      // 采集单元ID

    uint16_t group_type; // 传感器组类别
    uint16_t group_id;   // 传感器组ID
    uint16_t cycle;      // 采集周期

    uint16_t tail; // 帧尾
} NET_MSG_CYCLE;   // 服务器->客户端:设置采集周期

/************************************************************/

// 调试指令:写入缓存
typedef struct __attribute__((packed))
{
    uint16_t header;       // 帧头            aa 55
    uint8_t frame_type;    // 帧类型          dd
    uint8_t sub_type;      // 子类型          01
    uint32_t frame_length; // 帧长度          13 00 00 00
    uint16_t machine_type; // 设备类型        01 00 (门座吊机)  02 00 (高空车)
    uint16_t machine_id;   // 设备ID          65 00 (101)  66 00 (102)  67 00 (103)  C9 00 (201)  CA 00 (202)  CB 00 (203)
    uint16_t node_id;      // 采集单元ID      4d 04 (1101)  4e 04 (1102)  4f 04 (1103)  b1 04 (1201)  b2 04 (1202)  b3 04 (1203)
    uint16_t version_id;   // app版本ID       09 00
    uint8_t need_update;   // 需要更新        00 不需要 01 需要
    uint16_t tail;         // 帧尾            0a bd
} DEBUG_SET_CACHE;

typedef struct __attribute__((packed))
{
    uint16_t header;
    uint8_t frame_type;
    uint8_t sub_type;
    uint32_t frame_length;
    uint16_t machine_type;
    uint16_t machine_id;
    uint16_t node_id;

    uint8_t sensor[23];

    uint16_t tail;
} NET_MSG_REPORT_SENSOR_STATUS; // 上报传感器状态

typedef struct __attribute__((packed))
{
    uint16_t header;
    uint8_t frame_type;
    uint8_t sub_type;
    uint32_t frame_length;
    uint16_t machine_type;
    uint16_t machine_id;
    uint16_t node_id;

    uint8_t reg[12];

    uint16_t tail;
} NET_MSG_REPORT_PLC_STATUS; // 上报PLC状态

typedef struct __attribute__((packed))
{
    uint16_t header;       // 帧头        0xaa 0x55
    uint8_t frame_type;    // 帧类型      0xa2
    uint8_t sub_type;      // 子类型
    uint32_t frame_length; // 帧长度      0x12 0x00 0x00 0x00
    uint16_t machine_type; // 设备类型
    uint16_t machine_id;   // 设备ID
    uint16_t node_id;      // 采集单元ID

    uint16_t version_id; // app版本号

    uint16_t tail;       // 帧尾        0x0a 0xbd
} NET_MSG_REPORT_APP_VERSION; // 客户端->服务器 上报app版本

typedef struct __attribute__((packed))
{
    uint16_t header;       // 帧头        0xaa 0x55
    uint8_t frame_type;    // 帧类型      0xdd
    uint8_t sub_type;      // 子类型      0x04
    uint32_t frame_length; // 帧长度      0x16 0x00 0x00 0x00
    uint16_t machine_type; // 设备类型
    uint16_t machine_id;   // 设备ID
    uint16_t node_id;      // 采集单元ID

    uint16_t new_machine_type; // 新设备类型
    uint16_t new_machine_id;   // 新设备ID
    uint16_t new_node_id;      // 新采集单元ID

    uint16_t tail; // 帧尾      0x0a 0xbd

} NET_MSG_MODIFY_NODE;

/************************************************************/

extern char at_msg_timestamp_head[];

/************************************************************/

void network_init(void); // 网络初始化

void receive_net_msg(void);        // 接收网络数据
void analyze_net_msg(void);        // 解析网络数据
void analyze_debug_msg(void);      // 解析调试指令
void analyze_at_msg(void);         // 解析AT指令
void analyze_debug_mode_msg(void); // 解析调试模式指令

void net_cmd_reboot(void);        // 下行:重启
void net_cmd_cycle(void);         // 下行:更新采集周期
void net_cmd_update(void);        // 下行:app待升级
void net_cmd_cancel_update(void); // 下行:撤销app待升级

void net_cmd_reboot_4g_in(void);   // 下行:重启内网4G模块
void net_cmd_reboot_4g_out(void);  // 下行:重启外网4G模块
void net_cmd_report_in_on(void);   // 下行:开启内网数据上报
void net_cmd_report_in_off(void);  // 下行:关闭内网数据上报
void net_cmd_report_out_on(void);  // 下行:开启外网数据上报
void net_cmd_report_out_off(void); // 下行:关闭外网数据上报

void net_cmd_report_version(void); // 上行:报告app版本号

void net_cmd_report_data_diaoji_yingli(void);     // 上行:报告吊机应力传感器数据
void net_cmd_report_data_diaoji_wenzhen(void);    // 上行:报告吊机温振传感器数据
void net_cmd_report_data_diaoji_plc(void);        // 上行:报告吊机plc数据
void net_cmd_report_data_gaokongche_sensor(void); // 上行:报告高空车传感器数据

void net_cmd_report_status_diaoji_sensor(void); // 上行:报告吊机传感器状态
void net_cmd_report_status_diaoji_plc(void);    // 上行:报告吊机plc状态

void net_cmd_modify_node(void); // 下行:修改节点信息

void net_cmd_report_rcv(uint8_t frame_type, uint8_t sub_type);

void net_cmd_report_common(uint8_t frame_type, uint8_t sub_type); // 上行:通用指令

void net_cmd_report_app_run(void);           // 上行:报告app启动
void net_cmd_report_reboot(void);            // 上行:报告即将重启
void net_cmd_set_baudrate_9600(void);        // 下行:设置传感器口波特率为9600
void net_cmd_set_baudrate_115200(void);      // 下行:设置传感器口波特率为115200
void net_cmd_on_report_sensor_status(void);  // 下行:开启上报传感器状态
void net_cmd_off_report_sensor_status(void); // 下行:关闭上报传感器状态
void net_cmd_on_report_plc_status(void);     // 下行:开启上报PLC状态
void net_cmd_off_report_plc_status(void);    // 下行:关闭上报PLC状态

void debug_set_cache(void); // 调试指令:写入缓存

void at_sync_timestamp(void);                 // AT指令:同步基站时间戳
void send_at_cmd_out(char *str, size_t size); // 发送AT指令(外网4G模块)
void send_at_cmd_in(char *str, size_t size);  // 发送AT指令(内网4G模块)

#endif
