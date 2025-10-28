#ifndef __NETWORK_H
#define __NETWORK_H

#include "./SYSTEM/sys/sys.h"
#include "PLC/plc.h"
#include "stdbool.h"
/*
 * ͨ��ָ��Ŀ¼(����)
 * ee 01: ����app����
 * ee 02: ���漴������
 * ee 03: �ϱ�app�汾��
 *
 * ͨ��ָ��Ŀ¼(����)
 * dd 05: ����app������
 * dd 06: ѯ��app�汾��
 * dd 07: �޸Ĵ������ڲ�����Ϊ9600
 * dd 08: �޸Ĵ������ڲ�����Ϊ115200
 * dd 09: �����ϱ�������״̬
 * dd 0a: �ر��ϱ�������״̬
 * dd 0b: �����ϱ�PLC״̬
 * dd 0c: �ر��ϱ�PLC״̬
 */
/************************************************************/
/* �������ݽṹ */

typedef struct __attribute__((packed))
{
    uint16_t header;       // ֡ͷ       0xaa 0x55
    uint8_t frame_type;    // ֡����
    uint8_t sub_type;      // ������
    uint32_t frame_length; // ֡����
    uint16_t machine_type; // �豸����
    uint16_t machine_id;   // �豸ID
    uint16_t node_id;      // �ɼ���ԪID
} NET_MSG_HEAD;            // ͨ��֡�ṹ:ǰ׺

typedef struct __attribute__((packed))
{
    uint16_t header;       // ֡ͷ        0xaa 0x55
    uint8_t frame_type;    // ֡����      0xa2
    uint8_t sub_type;      // ������
    uint32_t frame_length; // ֡����      0x10 0x00 0x00 0x00
    uint16_t machine_type; // �豸����
    uint16_t machine_id;   // �豸ID
    uint16_t node_id;      // �ɼ���ԪID
    uint16_t tail;         // ֡β        0x0a 0xbd
} NET_MSG;                 // ������->�ͻ��� ͨ��ָ��

typedef struct __attribute__((packed))
{
    float temp;
    float freq;
} STRUCT_YINGLI; // Ӧ�������ݽṹ

typedef struct __attribute__((packed))
{
    uint16_t header;       // ֡ͷ       0xaa 0x55
    uint8_t frame_type;    // ֡����      0x02
    uint8_t sub_type;      // ������      0x01
    uint32_t frame_length; // ֡����
    uint16_t machine_type; // �豸����     0x01
    uint16_t machine_id;   // �豸ID
    uint16_t node_id;      // �ɼ���ԪID

    uint16_t group_type; // �����������   0x01
    uint16_t group_id;   // ��������ID     0x01
    uint32_t timestamp;  // ʱ���

    float temp;               // �¶ȴ�����
    STRUCT_YINGLI yingli[10]; // Ӧ��������

    uint16_t tail;     // ֡β       0x0a 0xbd
} NET_MSG_DATA_YINGLI; // �ͻ���->������:�ϱ�����(��1��) ����-Ӧ��

typedef struct __attribute__((packed))
{
    float temp;
    float acc_x;
    float acc_y;
    float acc_z; // ������ȡ���ֵ
    float freq_z;
} STRUCT_WENZHEN; // ���񴫸������ݽṹ

typedef struct __attribute__((packed))
{
    uint16_t header;       // ֡ͷ       0xaa 0x55
    uint8_t frame_type;    // ֡����      0x02
    uint8_t sub_type;      // ������      0x01
    uint32_t frame_length; // ֡����
    uint16_t machine_type; // �豸����     0x01
    uint16_t machine_id;   // �豸ID
    uint16_t node_id;      // �ɼ���ԪID

    uint16_t group_type; // �����������   0x02
    uint16_t group_id;   // ��������ID     0x01
    uint32_t timestamp;  // ʱ���

    STRUCT_WENZHEN wenzhen[11]; // ���񴫸���

    uint16_t tail;      // ֡β       0x0a 0xbd
} NET_MSG_DATA_WENZHEN; // �ͻ���->������:�ϱ�����(��2��) ����-����

typedef struct __attribute__((packed))
{
    uint16_t header;       // ֡ͷ
    uint8_t frame_type;    // ֡����
    uint8_t sub_type;      // ������
    uint32_t frame_length; // ֡����
    uint16_t machine_type; // �豸���� 0x01
    uint16_t machine_id;   // �豸ID
    uint16_t node_id;      // �ɼ���ԪID

    uint16_t group_type; // ����������� 0x03
    uint16_t group_id;   // ��������ID 0x01
    uint32_t timestamp;  // ʱ���

    uint8_t data[20]; // PLC�Ĵ�������

    uint16_t tail;  // ֡β
} NET_MSG_DATA_PLC; // �ͻ���->������:�ϱ�����(��3��) ����-PLC

typedef struct __attribute__((packed))
{
    float angle_x;
    float angle_y;
    float angle_z;
    float acc_x;
    float acc_y;
    float acc_z;
} STRUCT_QINGJIAO; // ��Ǵ��������ݽṹ

typedef struct __attribute__((packed))
{
    uint16_t header;       // ֡ͷ
    uint8_t frame_type;    // ֡����
    uint8_t sub_type;      // ������
    uint32_t frame_length; // ֡����
    uint16_t machine_type; // �豸���� 0x02
    uint16_t machine_id;   // �豸ID
    uint16_t node_id;      // �ɼ���ԪID

    uint16_t group_type; // ����������� 0x04
    uint16_t group_id;   // ��������ID 0x01
    uint32_t timestamp;  // ʱ���

    STRUCT_QINGJIAO qingjiao[2]; // ��Ǵ�����
    STRUCT_YINGLI yingli;        // Ӧ��������
    STRUCT_WENZHEN wenzhen;      // ���񴫸���

    uint16_t tail; // ֡β

} NET_MSG_DATA_GAOKONGCHE; // �ͻ���->������:�ϱ�����(��4��) �߿ճ�-������

typedef struct __attribute__((packed))
{
    uint16_t header;       // ֡ͷ
    uint8_t frame_type;    // ֡����
    uint8_t sub_type;      // ������
    uint32_t frame_length; // ֡����
    uint16_t machine_type; // �豸����
    uint16_t machine_id;   // �豸ID
    uint16_t node_id;      // �ɼ���ԪID

    uint16_t group_type; // �����������
    uint16_t group_id;   // ��������ID
    uint16_t cycle;      // �ɼ�����

    uint16_t tail; // ֡β
} NET_MSG_CYCLE;   // ������->�ͻ���:���òɼ�����

/************************************************************/

// ����ָ��:д�뻺��
typedef struct __attribute__((packed))
{
    uint16_t header;       // ֡ͷ            aa 55
    uint8_t frame_type;    // ֡����          dd
    uint8_t sub_type;      // ������          01
    uint32_t frame_length; // ֡����          13 00 00 00
    uint16_t machine_type; // �豸����        01 00 (��������)  02 00 (�߿ճ�)
    uint16_t machine_id;   // �豸ID          65 00 (101)  66 00 (102)  67 00 (103)  C9 00 (201)  CA 00 (202)  CB 00 (203)
    uint16_t node_id;      // �ɼ���ԪID      4d 04 (1101)  4e 04 (1102)  4f 04 (1103)  b1 04 (1201)  b2 04 (1202)  b3 04 (1203)
    uint16_t version_id;   // app�汾ID       09 00
    uint8_t need_update;   // ��Ҫ����        00 ����Ҫ 01 ��Ҫ
    uint16_t tail;         // ֡β            0a bd
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
} NET_MSG_REPORT_SENSOR_STATUS; // �ϱ�������״̬

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
} NET_MSG_REPORT_PLC_STATUS; // �ϱ�PLC״̬

typedef struct __attribute__((packed))
{
    uint16_t header;       // ֡ͷ        0xaa 0x55
    uint8_t frame_type;    // ֡����      0xa2
    uint8_t sub_type;      // ������
    uint32_t frame_length; // ֡����      0x12 0x00 0x00 0x00
    uint16_t machine_type; // �豸����
    uint16_t machine_id;   // �豸ID
    uint16_t node_id;      // �ɼ���ԪID

    uint16_t version_id; // app�汾��

    uint16_t tail;       // ֡β        0x0a 0xbd
} NET_MSG_REPORT_APP_VERSION; // �ͻ���->������ �ϱ�app�汾

typedef struct __attribute__((packed))
{
    uint16_t header;       // ֡ͷ        0xaa 0x55
    uint8_t frame_type;    // ֡����      0xdd
    uint8_t sub_type;      // ������      0x04
    uint32_t frame_length; // ֡����      0x16 0x00 0x00 0x00
    uint16_t machine_type; // �豸����
    uint16_t machine_id;   // �豸ID
    uint16_t node_id;      // �ɼ���ԪID

    uint16_t new_machine_type; // ���豸����
    uint16_t new_machine_id;   // ���豸ID
    uint16_t new_node_id;      // �²ɼ���ԪID

    uint16_t tail; // ֡β      0x0a 0xbd

} NET_MSG_MODIFY_NODE;

/************************************************************/

extern char at_msg_timestamp_head[];

/************************************************************/

void network_init(void); // �����ʼ��

void receive_net_msg(void);        // ������������
void analyze_net_msg(void);        // ������������
void analyze_debug_msg(void);      // ��������ָ��
void analyze_at_msg(void);         // ����ATָ��
void analyze_debug_mode_msg(void); // ��������ģʽָ��

void net_cmd_reboot(void);        // ����:����
void net_cmd_cycle(void);         // ����:���²ɼ�����
void net_cmd_update(void);        // ����:app������
void net_cmd_cancel_update(void); // ����:����app������

void net_cmd_reboot_4g_in(void);   // ����:��������4Gģ��
void net_cmd_reboot_4g_out(void);  // ����:��������4Gģ��
void net_cmd_report_in_on(void);   // ����:�������������ϱ�
void net_cmd_report_in_off(void);  // ����:�ر����������ϱ�
void net_cmd_report_out_on(void);  // ����:�������������ϱ�
void net_cmd_report_out_off(void); // ����:�ر����������ϱ�

void net_cmd_report_version(void); // ����:����app�汾��

void net_cmd_report_data_diaoji_yingli(void);     // ����:�������Ӧ������������
void net_cmd_report_data_diaoji_wenzhen(void);    // ����:����������񴫸�������
void net_cmd_report_data_diaoji_plc(void);        // ����:�������plc����
void net_cmd_report_data_gaokongche_sensor(void); // ����:����߿ճ�����������

void net_cmd_report_status_diaoji_sensor(void); // ����:�������������״̬
void net_cmd_report_status_diaoji_plc(void);    // ����:�������plc״̬

void net_cmd_modify_node(void); // ����:�޸Ľڵ���Ϣ

void net_cmd_report_rcv(uint8_t frame_type, uint8_t sub_type);

void net_cmd_report_common(uint8_t frame_type, uint8_t sub_type); // ����:ͨ��ָ��

void net_cmd_report_app_run(void);           // ����:����app����
void net_cmd_report_reboot(void);            // ����:���漴������
void net_cmd_set_baudrate_9600(void);        // ����:���ô������ڲ�����Ϊ9600
void net_cmd_set_baudrate_115200(void);      // ����:���ô������ڲ�����Ϊ115200
void net_cmd_on_report_sensor_status(void);  // ����:�����ϱ�������״̬
void net_cmd_off_report_sensor_status(void); // ����:�ر��ϱ�������״̬
void net_cmd_on_report_plc_status(void);     // ����:�����ϱ�PLC״̬
void net_cmd_off_report_plc_status(void);    // ����:�ر��ϱ�PLC״̬

void debug_set_cache(void); // ����ָ��:д�뻺��

void at_sync_timestamp(void);                 // ATָ��:ͬ����վʱ���
void send_at_cmd_out(char *str, size_t size); // ����ATָ��(����4Gģ��)
void send_at_cmd_in(char *str, size_t size);  // ����ATָ��(����4Gģ��)

#endif
