#ifndef __OTA_H
#define __OTA_H

#include "./SYSTEM/sys/sys.h"

/************************************************************/
/* �������ݽṹ */

typedef struct __attribute__((packed)) // �ͻ���->������:�������app
{
    uint16_t header;        // ֡ͷ
    uint8_t frame_type;     // ֡����
    uint8_t sub_type;       // ������
    uint32_t frame_length;  // ֡����
    uint16_t machine_type;  // �豸����
    uint16_t machine_id;    // �豸ID
    uint16_t node_id;       // �ɼ���ԪID
    uint32_t packet_no;     // ��ǰ��ɰ���(����)
    uint16_t tail;          // ֡β
} NET_MSG_DOWNLOAD;

typedef struct __attribute__((packed)) // ͨ��֡�ṹ:ǰ׺
{
    uint16_t header;        // ֡ͷ
    uint8_t frame_type;     // ֡����
    uint8_t sub_type;       // ������
    uint32_t frame_length;  // ֡����
    uint16_t machine_type;  // �豸����
    uint16_t machine_id;    // �豸ID
    uint16_t node_id;       // �ɼ���ԪID
} NET_MSG_HEAD;

typedef struct __attribute__((packed)) // ������->�ͻ���:����app
{
    uint16_t header;
    uint8_t frame_type;     // ֡����
    uint8_t sub_type;       // ������
    uint32_t frame_length;  // ֡����
    uint16_t machine_type;  // �豸����
    uint16_t machine_id;    // �豸ID
    uint16_t node_id;       // �ɼ���ԪID
    
    uint32_t app_version;   // ����汾ID
    uint32_t app_length;    // ���򳤶�
    uint32_t app_crc;       // ����CRC(����)
    uint16_t packet_total;  // �ܰ���(����)
    uint16_t packet_size;   // ÿ������(����)
    uint16_t packet_now;    // ��ǰ��(����)
} NET_MSG_APP;

/****************************************************************/

typedef struct __attribute__((packed)) // ����ָ��:д�뻺��
{
    uint16_t header;        // ֡ͷ            0xaa55
    uint8_t frame_type;     // ֡����          0xdd
    uint8_t sub_type;       // ������          0x01
    uint32_t frame_length;  // ֡����          0x1300      19
    uint16_t machine_type;  // �豸����        1 �������� 2 �߿ճ�
    uint16_t machine_id;    // �豸ID          0x6500      101
    uint16_t node_id;       // �ɼ���ԪID      0x4d04      1101
    uint16_t version_id;    // app�汾ID       0x0100      1
    uint8_t need_update;    // ��Ҫ����        0x00        0
    uint16_t tail;          // ֡β            0x0abd
} DEBUG_SET_CACHE;

/************************************************************/

void net_download_app(void);    // �������app
void receive_net_msg(void);     // ������������
void analyze_net_msg(void);     // ������������
void analyze_debug_msg(void);   // ��������ָ��
void analyze_at_msg(void);      // ����ATָ��

void net_cmd_reboot(void);      // ����ָ��:����
void net_cmd_app(void);         // ����ָ��:����app

void debug_set_cache(void);     // ����ָ��:д�뻺��

void at_sync_timestamp(void);   // ATָ��:ͬ����վʱ���
void send_at_cmd(char *str, size_t size);    // ����ATָ��(����4Gģ��)

#endif
