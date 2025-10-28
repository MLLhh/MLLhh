#ifndef __OTA_H
#define __OTA_H

#include "./SYSTEM/sys/sys.h"

/************************************************************/
/* 网络数据结构 */

typedef struct __attribute__((packed)) // 客户端->服务器:申请更新app
{
    uint16_t header;        // 帧头
    uint8_t frame_type;     // 帧类型
    uint8_t sub_type;       // 子类型
    uint32_t frame_length;  // 帧长度
    uint16_t machine_type;  // 设备类型
    uint16_t machine_id;    // 设备ID
    uint16_t node_id;       // 采集单元ID
    uint32_t packet_no;     // 当前完成包数(保留)
    uint16_t tail;          // 帧尾
} NET_MSG_DOWNLOAD;

typedef struct __attribute__((packed)) // 通用帧结构:前缀
{
    uint16_t header;        // 帧头
    uint8_t frame_type;     // 帧类型
    uint8_t sub_type;       // 子类型
    uint32_t frame_length;  // 帧长度
    uint16_t machine_type;  // 设备类型
    uint16_t machine_id;    // 设备ID
    uint16_t node_id;       // 采集单元ID
} NET_MSG_HEAD;

typedef struct __attribute__((packed)) // 服务器->客户端:发送app
{
    uint16_t header;
    uint8_t frame_type;     // 帧类型
    uint8_t sub_type;       // 子类型
    uint32_t frame_length;  // 帧长度
    uint16_t machine_type;  // 设备类型
    uint16_t machine_id;    // 设备ID
    uint16_t node_id;       // 采集单元ID
    
    uint32_t app_version;   // 程序版本ID
    uint32_t app_length;    // 程序长度
    uint32_t app_crc;       // 程序CRC(保留)
    uint16_t packet_total;  // 总包数(保留)
    uint16_t packet_size;   // 每包长度(保留)
    uint16_t packet_now;    // 当前包(保留)
} NET_MSG_APP;

/****************************************************************/

typedef struct __attribute__((packed)) // 调试指令:写入缓存
{
    uint16_t header;        // 帧头            0xaa55
    uint8_t frame_type;     // 帧类型          0xdd
    uint8_t sub_type;       // 子类型          0x01
    uint32_t frame_length;  // 帧长度          0x1300      19
    uint16_t machine_type;  // 设备类型        1 门座吊机 2 高空车
    uint16_t machine_id;    // 设备ID          0x6500      101
    uint16_t node_id;       // 采集单元ID      0x4d04      1101
    uint16_t version_id;    // app版本ID       0x0100      1
    uint8_t need_update;    // 需要更新        0x00        0
    uint16_t tail;          // 帧尾            0x0abd
} DEBUG_SET_CACHE;

/************************************************************/

void net_download_app(void);    // 请求更新app
void receive_net_msg(void);     // 接收网络数据
void analyze_net_msg(void);     // 解析网络数据
void analyze_debug_msg(void);   // 解析调试指令
void analyze_at_msg(void);      // 解析AT指令

void net_cmd_reboot(void);      // 网络指令:重启
void net_cmd_app(void);         // 网络指令:更新app

void debug_set_cache(void);     // 调试指令:写入缓存

void at_sync_timestamp(void);   // AT指令:同步基站时间戳
void send_at_cmd(char *str, size_t size);    // 发送AT指令(外网4G模块)

#endif
