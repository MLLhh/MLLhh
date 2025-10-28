#include "plc.h"
#include "BSP/RS232/rs232.h"
#include "SYSTEM/sys/sys.h"
#include "SYSTEM/delay/delay.h"
#include "Common/utils.h"
#include "stdio.h"
#include "string.h"
#include "BSP/STMFLASH/stmflash.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stdlib.h"
#include "Network/network.h"
#include "stdbool.h"

uint32_t rxlen_com2 = 0;
uint8_t plc_rx_buf[128];
// STRUCT_PLC_DATA plc_data; // �洢�ɼ���������

extern STMFLASH_CACHE stmflash_cache; // flash����

uint32_t tick = 0; // ����
uint32_t tick_max = 0;
uint32_t tick_interval = 0;

uint32_t tick_cnt = 0;       // ������
uint32_t plc_interval = 500; // ���� ָ��ͼ��

uint8_t plc_reg_total = 8;                           // ��Ҫ��ȡ�ļĴ�������
uint8_t plc_reg_index = 0;                           // ��ǰ��ȡ�Ĵ����������е����
uint8_t plc_reg = 0;                                 // ��ǰ��ȡ�Ĵ�����
uint8_t plc_reg_array[] = {0, 1, 2, 3, 4, 5, 9, 11}; // ��Ҫ��ȡ�ļĴ���������

extern NET_MSG_DATA_PLC msg_plc_diaoji;
extern NET_MSG_REPORT_PLC_STATUS msg_plc_status_diaoji;

int debug_id = 0;

void plc_init(void)
{
    tick_max = stmflash_cache.plc_cycle * (1000 / 100);
    tick_interval = plc_interval / 100;

    reset_plc_data();
}

void read_plc_data(void) // ��������
{
    if (tick % tick_interval == 0)
    {
        tick_cnt = tick / tick_interval;
        if (tick_cnt < plc_reg_total + 1) // �˴�+1����Ϊ�ɼ���ɺ���һ��tick_cnt�����ϱ�
        {
            read_next_plc_reg();
        }
    }
    tick = (tick + 1) % tick_max;
}

void receive_plc_data(void) // ��������
{
    rxlen_com2 = g_rs232_com2_rx_cnt;

    delay_ms(10);

    if (g_rs232_com2_rx_cnt)
    {
        if (rxlen_com2 == g_rs232_com2_rx_cnt)
        {
            // printf("����PLC %u �żĴ���< ", plc_reg);
            printstring((char *)g_rs232_com2_rx_buf, rxlen_com2);

            if (check_plc_data())
            {
                analyze_plc_data();
            }

            g_rs232_com2_rx_cnt = 0;
        }
        else
        {
            rxlen_com2 = g_rs232_com2_rx_cnt;
        }
    }
}

bool check_plc_data(void) // ���PLC�������ݸ�ʽ
{
    memcpy(&plc_rx_buf, &g_rs232_com2_rx_buf, rxlen_com2);

    char *buf = (char *)plc_rx_buf;
    char res[2];
    memcpy(res, buf + 5, 2);

    if (strncmp(res, "00", 2) == 0)
    {
        if (strncmp(res_plc_head, buf, 7) == 0)
        {
            return true;
        }
        else
        {
            printf("��Ӧ��ʽ�쳣\n");
            return false;
        }
    }
    else
    {
        printf("Ӧ�����쳣 = %c%c\n", res[0], res[1]);
        return false;
    }
}

void analyze_plc_data(void) // ����PLC��������
{
    char *buf = (char *)plc_rx_buf;
    // char buf[] = "@00RR00010100*"; // ����
    char ascii[4];              // ����(ascii��ʽ��ʮ��������)
    memcpy(ascii, buf + 7, 4);  // ��PLC����������ȡ������
    uint16_t val;               // ��ֵ
    sscanf(ascii, "%hx", &val); // ��ascii��ʽ��ʮ��������ת��Ϊ��ֵ

    msg_plc_status_diaoji.reg[plc_reg] = 0x01; // ����PLC״̬

    switch (plc_reg)
    {
    case 0:
    {
        msg_plc_diaoji.data[1] = get_bit(val, 11); // 1       0.11    ������������λ
        msg_plc_diaoji.data[3] = get_bit(val, 12); // 3       0.12    �������½���λ
        msg_plc_diaoji.data[15] = get_bit(val, 6); // 15      0.06    ��/��������Ƶ������
        break;
    }
    case 1:
    {
        msg_plc_diaoji.data[4] = get_bit(val, 11); // 4       1.11    ������������λ
        msg_plc_diaoji.data[6] = get_bit(val, 12); // 6       1.12    �������½���λ
        break;
    }
    case 2:
    {
        msg_plc_diaoji.data[7] = get_bit(val, 11);  // 7       2.11    ���������λ
        msg_plc_diaoji.data[9] = get_bit(val, 12);  // 9       2.12    ���������λ
        msg_plc_diaoji.data[10] = get_bit(val, 13); // 10      2.13    ����ŵ���λ
        msg_plc_diaoji.data[16] = get_bit(val, 5);  // 16      2.05    �����Ƶ������
        break;
    }
    case 3:
    {
        msg_plc_diaoji.data[17] = get_bit(val, 5);  // 17      3.05    ��ת��Ƶ������
        msg_plc_diaoji.data[18] = get_bit(val, 13); // 18      3.13    ���߱�Ƶ������
        break;
    }
    case 4:
    {
        msg_plc_diaoji.data[11] = get_bit(val, 9); // 11      4.09    ����ê����λ
        msg_plc_diaoji.data[12] = get_bit(val, 5); // 12      4.05    ������λ
        msg_plc_diaoji.data[13] = get_bit(val, 6); // 13      4.06    ������λ
        msg_plc_diaoji.data[19] = get_bit(val, 7); // 19      4.07    �����ն���λ
        break;
    }
    case 5:
    {
        msg_plc_diaoji.data[14] = get_bit(val, 7); // 14      5.07    �����ź�
        break;
    }
    case 9:
    {
        msg_plc_diaoji.data[2] = get_bit(val, 9);  // 2       9.09    ��������������λ
        msg_plc_diaoji.data[5] = get_bit(val, 10); // 5       9.10    ��������������λ
        msg_plc_diaoji.data[8] = get_bit(val, 11); // 8       9.11    �������������λ
        break;
    }
    case 11:
    {
        msg_plc_diaoji.data[0] = get_bit(val, 7); // 0       11.07   ���Ӵ����м�
        break;
    }
    }
}

// ��ȡ��һ���Ĵ���
void read_next_plc_reg(void)
{
    if (plc_reg_index < plc_reg_total)
    {
        plc_reg = plc_reg_array[plc_reg_index];
        read_reg(plc_reg);
        plc_reg_index++;
    }
    else if (plc_reg_index == plc_reg_total)
    {
        refine_plc_data();
        
        net_cmd_report_data_diaoji_plc();
        net_cmd_report_status_diaoji_plc();
        
        plc_reg_index++;
    }
    else
    {
        reset_plc_data(); // �������ݻ���
        plc_reg_index = 0;
    }
}

// ��ָ���Ĵ���
void read_reg(uint8_t reg)
{
    // printf("��ȡPLC %u �żĴ���: ", reg);
    int len = 19;
    uint8_t buf[len];

    switch (reg)
    {
    case 0:
        memcpy(buf, read_plc_0, len);
        break;
    case 1:
        memcpy(buf, read_plc_1, len);
        break;
    case 2:
        memcpy(buf, read_plc_2, len);
        break;
    case 3:
        memcpy(buf, read_plc_3, len);
        break;
    case 4:
        memcpy(buf, read_plc_4, len);
        break;
    case 5:
        memcpy(buf, read_plc_5, len);
        break;
    case 9:
        memcpy(buf, read_plc_9, len);
        break;
    case 11:
        memcpy(buf, read_plc_11, len);
        break;
    default:
        printf("��ͼ��ȡδ����Ĵ��� %u\n", reg);
        return;
    }

    rs232_com2_send_data(buf, sizeof(buf));
}

void reset_plc_data(void) // ����PLC���ݻ���
{
    printf("�������ݻ���(PLC)\n");

    for (int i = 0; i < 20; i++)
    {
        msg_plc_diaoji.data[i] = 0;
    }

    for (int i = 0; i < 12; i++)
    {
        msg_plc_status_diaoji.reg[i] = 0xff;
    }
}

void refine_plc_data(void)
{
    // ��������
    if (stmflash_cache.machine_id == 101)
    {
        msg_plc_diaoji.data[0] = 1; // ���Ӵ����м�(�����豸����)
    }
    else if (stmflash_cache.machine_id == 102 || stmflash_cache.machine_id == 103)
    {
        for (int i =0; i < 20; i++)
        {
            msg_plc_diaoji.data[i] = 2;
        }
    }
}
