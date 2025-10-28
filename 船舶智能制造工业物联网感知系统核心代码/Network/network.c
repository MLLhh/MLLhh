#include "network.h"
#include "SYSTEM/sys/sys.h"
#include "SYSTEM/delay/delay.h"
#include "string.h"
#include "BSP/RS485/rs485.h"
#include "SYSTEM/usart/usart.h"
#include "BSP/STMFLASH/stmflash.h"
#include "BSP/RS485/rs485_uart5.h"
#include "Common/utils.h"
#include "stdint.h"
#include "stdlib.h"
#include "Common/timestamp.h"
#include "PLC/plc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stdbool.h"
#include "BSP/RS232/rs232.h"

#define NET_TX_LEN 400

extern uint32_t app_version; // app�汾��(main.c)

uint16_t msg_header = 0x55aa; // ֡ͷ
uint16_t msg_tail = 0xbd0a;   // ֡β

extern uint8_t g_rs485_rx_buf[RS485_REC_LEN];
extern uint32_t g_rs485_rx_cnt;

uint8_t net_tx_buf[NET_TX_LEN];

uint32_t rxlen_485 = 0;

extern STMFLASH_CACHE stmflash_cache; // flash����

char at_msg_timestamp_head[] = "+TIMESTAMP: "; // ʱ���

char g_at_msg_buf[128]; // ATָ���
uint32_t at_len = 0;

bool FLAG_REPORT_IN = true;   // �����ϱ�����(Ĭ�Ͽ���)
bool FLAG_REPORT_OUT = false; // �����ϱ�����(Ĭ�Ϲر�)

bool FLAG_REPORT_SENSOR_STATUS = false; // �ϱ�������״̬
bool FLAG_REPORT_PLC_STATUS = false;    // �ϱ�PLC״̬

NET_MSG_HEAD rx_head; // ͨ��֡�ṹ:ǰ׺

NET_MSG_DATA_YINGLI msg_yingli; // Ӧ������������
NET_MSG_DATA_WENZHEN msg_wenzhen; // ���񴫸�������
NET_MSG_DATA_PLC msg_plc_diaoji; // ����plc����
NET_MSG_REPORT_SENSOR_STATUS msg_s_status_diaoji; // ����������״̬
NET_MSG_REPORT_PLC_STATUS msg_plc_status_diaoji; // ����plc״̬

NET_MSG_DATA_GAOKONGCHE msg_gaokongche;

void network_init(void)
{
    at_sync_timestamp(); // ͬ����վʱ���
}

void receive_net_msg(void) // ������������
{
    rxlen_485 = g_rs485_rx_cnt;

    delay_ms(100);

    if (g_rs485_rx_cnt)
    {
        if (rxlen_485 == g_rs485_rx_cnt)
        {
            printf("�������� <<< ");
            printhex(g_rs485_rx_buf, rxlen_485);

            memcpy(&rx_head, &g_rs485_rx_buf, sizeof(rx_head));

            if (rx_head.header == msg_header) // ���֡ͷ
            {
                printf("����֡���� = %u\n", rxlen_485);
                printf("ָ��֡���� = %u\n", rx_head.frame_length);

                if (rxlen_485 >= rx_head.frame_length) // ���֡����
                {
                    uint16_t tmp_tail;
                    memcpy(&tmp_tail, &g_rs485_rx_buf[rx_head.frame_length - 2], 2);
                    if (tmp_tail == msg_tail) // ���֡β
                    {
                        // ����豸ID��ɼ���ԪID
                        printf("�����豸ID:%u\n", stmflash_cache.machine_id);
                        printf("ָ���豸ID:%u\n", rx_head.machine_id);
                        printf("�����ɼ���ԪID:%u\n", stmflash_cache.node_id);
                        printf("ָ��ɼ���ԪID:%u\n", rx_head.node_id);
                        if ((rx_head.machine_id == stmflash_cache.machine_id) && (rx_head.node_id == stmflash_cache.node_id))
                        {
                            net_cmd_report_rcv(rx_head.frame_type, rx_head.sub_type);

                            if (rx_head.frame_type == 0xdd)
                            {
                                analyze_debug_mode_msg(); // ��������ģʽָ��
                            }
                            else
                            {
                                analyze_net_msg(); // ������������
                            }
                        } // end ����豸ID��ɼ���ԪID
                        else
                        {
                            analyze_debug_msg(); // �������ص���ָ��
                        }
                    }
                    else
                    {
                        printf("֡β����\n");
                    } // end ���֡β
                }
                else
                {
                    printf("֡���ȴ���\n");
                } // end ���֡����
            }
            else
            {
                analyze_at_msg(); // ����ATָ��

            } // end ���֡ͷ

            g_rs485_rx_cnt = 0; // ����
        }
        else
        {
            rxlen_485 = g_rs485_rx_cnt;
        }
    } // end �������
}

void analyze_net_msg(void) // ������������
{
    printf("������������ ֡����:0x%02X ������:0x%02X\n", rx_head.frame_type, rx_head.sub_type);

    switch (rx_head.frame_type) // ����֡���ͺ�������
    {
    case 0xa2:
    {
        switch (rx_head.sub_type)
        {
        case 0x01:
            net_cmd_cycle(); // ֡����ʶ��:���²ɼ�����
            break;
        case 0x02:
            net_cmd_update(); // ֡����ʶ��:app������
            break;
        case 0x03:
            net_cmd_reboot_4g_out(); // ֡����ʶ��;��������4Gģ��
            break;
        case 0x04:
            net_cmd_reboot_4g_in(); // ֡����ʶ��:��������4Gģ��
            break;
        case 0x05:
            net_cmd_report_out_on(); // �������������ϱ�
            break;
        case 0x06:
            net_cmd_report_out_off(); // �ر����������ϱ�
            break;
        case 0x07:
            net_cmd_report_in_on(); // �������������ϱ�
            break;
        case 0x08:
            net_cmd_report_in_off(); // �ر����������ϱ�
            break;
        }
        break;
    }
    case 0xfe:
        net_cmd_reboot(); // ֡����ʶ��:����
        break;
    default:
        printf("֡����ʶ��ʧ��\n");
        break;
    }
}

void analyze_at_msg(void) // ����ATָ��
{
    at_len = g_rs485_rx_cnt;
    memcpy(g_at_msg_buf, g_rs485_rx_buf, g_rs485_rx_cnt);

    printf("����AT��Ӧ:");
    printstring(g_at_msg_buf, at_len);

    if (0 == strncmp(g_at_msg_buf, at_msg_timestamp_head, sizeof(at_msg_timestamp_head) - 1))
    {
        uint32_t timestamp = strtoul(g_at_msg_buf + sizeof(at_msg_timestamp_head) - 1, NULL, 10);
        printf("ʱ���: %u\n", timestamp);
        set_boot_timestamp(timestamp);
    }
    //    }
}

void net_cmd_reboot(void) // ����ָ��:����
{
    net_cmd_report_reboot();
    printf("����\n");
    delay_ms(500);
    HAL_NVIC_SystemReset();
}

void net_cmd_cycle(void) // ����ָ��:���²ɼ�����
{
    printf("���²ɼ�����\n");

    NET_MSG_CYCLE msg;
    memcpy(&msg, &g_rs485_rx_buf, sizeof(NET_MSG_CYCLE));

    switch (msg.group_type)
    {
    case 0x01:
    case 0x02:
    case 0x04:
    {
        printf("�޸Ĵ������ɼ����� %u�� -> %u��\n", stmflash_cache.read_cycle, msg.cycle);
        stmflash_cache.read_cycle = msg.cycle;
        flash_write_cache();
        break;
    }
    case 0x03:
    {
        printf("�޸�PLC�ɼ����� %u�� -> %u��\n", stmflash_cache.plc_cycle, msg.cycle);
        stmflash_cache.plc_cycle = msg.cycle;
        flash_write_cache();
        break;
    }
    }
}

void net_cmd_update(void) // ����ָ��:app������
{
    printf("app������\n");
    stmflash_cache.need_update = 1;
    flash_write_cache();
}

void net_cmd_cancel_update(void)
{
    printf("����app������");
    stmflash_cache.need_update = 0;
    flash_write_cache();
}

void debug_set_cache(void) // ����ָ��:д�뻺��
{
//    printf("����ָ��:д�뻺��\n");

//    DEBUG_SET_CACHE msg;
//    memcpy(&msg, &g_rs485_rx_buf, sizeof(DEBUG_SET_CACHE));

//    stmflash_cache.machine_type = msg.machine_type;
//    stmflash_cache.machine_id = msg.machine_id;
//    stmflash_cache.node_id = msg.node_id;
//    stmflash_cache.version_id = msg.version_id;
//    stmflash_cache.need_update = msg.need_update;
//    flash_write_cache();
}

void analyze_debug_msg(void)
{
//    if (rx_head.frame_type == 0xdd)
//    {
//        switch (rx_head.sub_type)
//        {
//        case 0x01:
//            debug_set_cache(); // ����ָ��:д�뻺��
//            break;
//        }
//    }
}

void analyze_debug_mode_msg(void) // ��������ģʽָ��
{
    printf("��������ģʽָ��\n");

    if (rx_head.frame_type == 0xdd)
    {
        switch (rx_head.sub_type)
        {
        case 0x04:
            net_cmd_modify_node(); // �޸Ľڵ���Ϣ
            break;
        case 0x05:
            net_cmd_cancel_update(); // ����app������
            break;
        case 0x06:
            net_cmd_report_version(); // �ϱ�app�汾��
            break;
        case 0x07:
            net_cmd_set_baudrate_9600(); // ���ô������ڲ�����Ϊ9600
            break;
        case 0x08:
            net_cmd_set_baudrate_115200(); // ���ô������ڲ�����Ϊ115200
            break;
        case 0x09:
            net_cmd_on_report_sensor_status(); // �����ϱ�������״̬
            break;
        case 0x0a:
            net_cmd_off_report_sensor_status(); // �ر��ϱ�������״̬
            break;
        case 0x0b:
            net_cmd_on_report_plc_status(); // �����ϱ�PLC״̬
            break;
        case 0x0c:
            net_cmd_off_report_plc_status(); // �ر��ϱ�PLC״̬
            break;
        default:
            break;
        }
    }
}

void at_sync_timestamp(void) // ATָ��:ͬ����վʱ���
{
    printf("ͬ����վʱ���\n");

    char *str = "@DTU:0000:TIMESTAMP";
    send_at_cmd_out(str, strlen(str));
}

void send_at_cmd_out(char *str, size_t size) // ����ATָ��(����4Gģ��)
{
    uint8_t array[size];
    string_to_uint8_array(str, array, size);

    rs485_send_data(array, size);
}

void send_at_cmd_in(char *str, size_t size) // ����ATָ��(����4Gģ��)
{
    uint8_t array[size];
    string_to_uint8_array(str, array, size);

    uart5_send_data(array, size);
}

void net_cmd_reboot_4g_in(void) // ����:��������4Gģ��
{
    printf("��������4Gģ��\n");
    char *str = "@DTU:0000:POWEROFF";
    send_at_cmd_in(str, strlen(str));
}

void net_cmd_reboot_4g_out(void) // ����:��������4Gģ��
{
    printf("��������4Gģ��\n");
    char *str = "@DTU:0000:POWEROFF";
    send_at_cmd_out(str, strlen(str));
}

void net_cmd_report_in_on(void) // ����:�������������ϱ�
{
    FLAG_REPORT_IN = true;
    printf("�������������ϱ�\n");
}

void net_cmd_report_in_off(void) // ����:�ر����������ϱ�
{
    FLAG_REPORT_IN = false;
    printf("�ر����������ϱ�\n");
}

void net_cmd_report_out_on(void) // ����:�������������ϱ�
{
    FLAG_REPORT_OUT = true;
    printf("�������������ϱ�\n");
}

void net_cmd_report_out_off(void) // ����:�ر����������ϱ�
{
    FLAG_REPORT_OUT = false;
    printf("�ر����������ϱ�\n");
}

void net_cmd_report_version(void) // �ϱ�app�汾
{
    NET_MSG_REPORT_APP_VERSION msg;
    msg.header = msg_header;
    msg.frame_type = 0xdd;
    msg.sub_type = 0x06;
    msg.frame_length = sizeof(msg);
    msg.machine_type = stmflash_cache.machine_type;
    msg.machine_id = stmflash_cache.machine_id;
    msg.node_id = stmflash_cache.node_id;

    // msg.version_id = stmflash_cache.version_id;
    msg.version_id = app_version; // app�汾��

    msg.tail = msg_tail;

    memcpy(net_tx_buf, &msg, sizeof(msg));
    rs485_send_data(net_tx_buf, msg.frame_length);
}

void net_cmd_report_rcv(uint8_t frame_type, uint8_t sub_type) // �ϱ����յ�
{
    NET_MSG msg;
    msg.header = msg_header;
    msg.frame_type = frame_type;
    msg.sub_type = sub_type;
    msg.frame_length = sizeof(msg);
    msg.machine_type = stmflash_cache.machine_type;
    msg.machine_id = stmflash_cache.machine_id;
    msg.node_id = stmflash_cache.node_id;
    msg.tail = msg_tail;

    memcpy(net_tx_buf, &msg, sizeof(msg));
    rs485_send_data(net_tx_buf, sizeof(msg));
}

void net_cmd_report_data_diaoji_yingli(void)
{
    printf("�ϱ�����Ӧ������\n");
    msg_yingli.header = msg_header;
    msg_yingli.frame_type = 0x02;
    msg_yingli.sub_type = 0x01;
    msg_yingli.frame_length = sizeof(msg_yingli);
    msg_yingli.machine_type = stmflash_cache.machine_type;
    msg_yingli.machine_id = stmflash_cache.machine_id;
    msg_yingli.node_id = stmflash_cache.node_id;
    msg_yingli.group_type = 0x01;
    msg_yingli.group_id = 0x01;
    msg_yingli.timestamp = get_timestamp();
    msg_yingli.tail = msg_tail;

    memcpy(net_tx_buf, &msg_yingli, sizeof(msg_yingli));
    if (FLAG_REPORT_OUT)
        rs485_send_data(net_tx_buf, sizeof(msg_yingli));
    if (FLAG_REPORT_IN)
        uart5_send_data(net_tx_buf, sizeof(msg_yingli));
}

void net_cmd_report_data_diaoji_wenzhen(void)
{
    printf("�ϱ�������������\n");
    msg_wenzhen.header = msg_header;
    msg_wenzhen.frame_type = 0x02;
    msg_wenzhen.sub_type = 0x01;
    msg_wenzhen.frame_length = sizeof(msg_wenzhen);
    msg_wenzhen.machine_type = stmflash_cache.machine_type;
    msg_wenzhen.machine_id = stmflash_cache.machine_id;
    msg_wenzhen.node_id = stmflash_cache.node_id;
    msg_wenzhen.group_type = 0x02;
    msg_wenzhen.group_id = 0x01;
    msg_wenzhen.timestamp = get_timestamp();
    msg_wenzhen.tail = msg_tail;

    memcpy(net_tx_buf, &msg_wenzhen, sizeof(msg_wenzhen));
    if (FLAG_REPORT_OUT)
        rs485_send_data(net_tx_buf, sizeof(msg_wenzhen));
    if (FLAG_REPORT_IN)
        uart5_send_data(net_tx_buf, sizeof(msg_wenzhen));
}

void net_cmd_report_data_diaoji_plc(void)
{
    printf("�ϱ�����PLC����\n");
    msg_plc_diaoji.header = msg_header;
    msg_plc_diaoji.frame_type = 0x02;
    msg_plc_diaoji.sub_type = 0x01;
    msg_plc_diaoji.frame_length = sizeof(msg_plc_diaoji);
    msg_plc_diaoji.machine_type = stmflash_cache.machine_type;
    msg_plc_diaoji.machine_id = stmflash_cache.machine_id;
    msg_plc_diaoji.node_id = stmflash_cache.node_id;
    msg_plc_diaoji.group_type = 0x03;
    msg_plc_diaoji.group_id = 0x01;
    msg_plc_diaoji.timestamp = get_timestamp();
    msg_plc_diaoji.tail = msg_tail;

    memcpy(net_tx_buf, &msg_plc_diaoji, sizeof(msg_plc_diaoji));
    if (FLAG_REPORT_OUT)
        rs485_send_data(net_tx_buf, sizeof(msg_plc_diaoji));
    if (FLAG_REPORT_IN)
        uart5_send_data(net_tx_buf, sizeof(msg_plc_diaoji));
}

void net_cmd_report_data_gaokongche_sensor(void)
{
    printf("�ϱ��߿ճ�����������\n");
    msg_gaokongche.header = msg_header;
    msg_gaokongche.frame_type = 0x02;
    msg_gaokongche.sub_type = 0x01;
    msg_gaokongche.frame_length = sizeof(msg_gaokongche);
    msg_gaokongche.machine_type = stmflash_cache.machine_type;
    msg_gaokongche.machine_id = stmflash_cache.machine_id;
    msg_gaokongche.node_id = stmflash_cache.node_id;
    msg_gaokongche.group_type = 0x04;
    msg_gaokongche.group_id = 0x01;
    msg_gaokongche.timestamp = get_timestamp();
    msg_gaokongche.tail = msg_tail;

    memcpy(net_tx_buf, &msg_gaokongche, sizeof(msg_gaokongche));
    if (FLAG_REPORT_OUT)
        rs485_send_data(net_tx_buf, sizeof(msg_gaokongche)); // ����
    if (FLAG_REPORT_IN)
        uart5_send_data(net_tx_buf, sizeof(msg_gaokongche));     // ����
}

void net_cmd_report_status_diaoji_sensor(void)
{
    if (FLAG_REPORT_SENSOR_STATUS == false)
        return;

    printf("�ϱ�����������״̬\n");
    msg_s_status_diaoji.header = msg_header;
    msg_s_status_diaoji.frame_type = 0xdd;
    msg_s_status_diaoji.sub_type = 0x01;
    msg_s_status_diaoji.frame_length = sizeof(msg_s_status_diaoji);
    msg_s_status_diaoji.machine_type = stmflash_cache.machine_type;
    msg_s_status_diaoji.machine_id = stmflash_cache.machine_id;
    msg_s_status_diaoji.node_id = stmflash_cache.node_id;
    msg_s_status_diaoji.tail = msg_tail;

    memcpy(net_tx_buf, &msg_s_status_diaoji, sizeof(msg_s_status_diaoji));
    rs485_send_data(net_tx_buf, sizeof(msg_s_status_diaoji));
}

void net_cmd_report_status_diaoji_plc(void)
{
    if (FLAG_REPORT_PLC_STATUS == false)
        return;

    printf("�ϱ�����PLC״̬\n");
    msg_plc_status_diaoji.header = msg_header;
    msg_plc_status_diaoji.frame_type = 0xdd;
    msg_plc_status_diaoji.sub_type = 0x02;
    msg_plc_status_diaoji.frame_length = sizeof(msg_plc_status_diaoji);
    msg_plc_status_diaoji.machine_type = stmflash_cache.machine_type;
    msg_plc_status_diaoji.machine_id = stmflash_cache.machine_id;
    msg_plc_status_diaoji.node_id = stmflash_cache.node_id;
    msg_plc_status_diaoji.tail = msg_tail;

    memcpy(net_tx_buf, &msg_plc_status_diaoji, sizeof(msg_plc_status_diaoji));
    rs485_send_data(net_tx_buf, sizeof(msg_plc_status_diaoji));
}

void net_cmd_modify_node(void) // �޸Ľڵ���Ϣ
{
    printf("�޸Ľڵ���Ϣ\n");

    NET_MSG_MODIFY_NODE msg;
    memcpy(&msg, &g_rs485_rx_buf, sizeof(msg));

    uint16_t new_machine_type = msg.new_machine_type;
    uint16_t new_machine_id = msg.new_machine_id;
    uint16_t new_node_id = msg.new_node_id;

    printf("machine_type: %u -> %u \n", stmflash_cache.machine_type, new_machine_type);
    printf("machine_id: %u -> %u \n", stmflash_cache.machine_id, new_machine_id);
    printf("node_id: %u -> %u \n", stmflash_cache.node_id, new_node_id);

    stmflash_cache.machine_type = new_machine_type;
    stmflash_cache.machine_id = new_machine_id;
    stmflash_cache.node_id = new_node_id;
    flash_write_cache();
}

void net_cmd_report_common(uint8_t frame_type, uint8_t sub_type) // ͨ��ָ��
{
    NET_MSG msg;
    msg.header = msg_header;
    msg.frame_type = frame_type;
    msg.sub_type = sub_type;
    msg.frame_length = 0x10;
    msg.machine_type = stmflash_cache.machine_type;
    msg.machine_id = stmflash_cache.machine_id;
    msg.node_id = stmflash_cache.node_id;
    msg.tail = msg_tail;
    memcpy(net_tx_buf, &msg, sizeof(msg));
    rs485_send_data(net_tx_buf, sizeof(msg));
}

void net_cmd_report_app_run(void)
{
    net_cmd_report_common(0xee, 0x01);
    printf("����app����\n");
}

void net_cmd_report_reboot(void)
{
    net_cmd_report_common(0xee, 0x02);
    printf("���漴������");
}

void net_cmd_set_baudrate_9600(void)
{
    printf("���ô������ڲ�����Ϊ9600\n");
    stmflash_cache.baudrate = 1;
    flash_write_cache();
}

void net_cmd_set_baudrate_115200(void)
{
    printf("���ô������ڲ�����Ϊ115200\n");
    stmflash_cache.baudrate = 2;
    flash_write_cache();
}

void net_cmd_on_report_sensor_status(void) // �����ϱ�������״̬
{
    printf("�����ϱ�������״̬\n");
    FLAG_REPORT_SENSOR_STATUS = true;
}

void net_cmd_off_report_sensor_status(void) // �ر��ϱ�������״̬
{
    printf("�ر��ϱ�������״̬\n");
    FLAG_REPORT_SENSOR_STATUS = false;
}

void net_cmd_on_report_plc_status(void) // �����ϱ�PLC״̬
{
    printf("�����ϱ�PLC״̬\n");
    FLAG_REPORT_PLC_STATUS = true;
}

void net_cmd_off_report_plc_status(void) // �ر��ϱ�PLC״̬
{
    printf("�ر��ϱ�PLC״̬\n");
    FLAG_REPORT_PLC_STATUS = false;
}
