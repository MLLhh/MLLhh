#include "ota.h"
#include "SYSTEM/sys/sys.h"
#include "SYSTEM/usart/usart.h"
#include "SYSTEM/delay/delay.h"
#include "BSP/STMFLASH/stmflash.h"
#include "string.h"
#include "BSP/RS485/rs485.h"
#include "IAP/iap.h"
#include "Common/utils.h"
#include "BSP/LED/led.h"

uint16_t msg_header = 0x55aa;   // ֡ͷ
uint16_t msg_tail = 0xbd0a;     // ֡β

extern uint8_t g_rs485_tx_buf[TXBUFFERSIZE]; // ���ͻ�����
extern uint8_t g_rs485_rx_buf[RS485_REC_LEN];
extern uint32_t g_rs485_rx_cnt;

extern STMFLASH_CACHE stmflash_cache; // flash����

char at_head[] = "\r\n";
char at_tail[] = "\r\n";
char at_msg_timestamp_head[] = "+TIMESTAMP: "; // ʱ���
char at_msg_registered[] = "\r\n+STATUS:NET STATE REGISTERED\r\n";
char at_msg_connected[] = "\r\n+STATUS: 1, CONNECTED\r\n"; // TCP���ӳɹ�
char at_msg_closed[] = "\r\n+STATUS: 1, CLOSED\r\n"; // TCP���ӶϿ�

char g_at_msg_buf[128];  // ATָ���
uint32_t at_len = 0;

uint8_t flag_connected = 0; // TCP���ӳɹ���־
uint8_t flag_update_app = 0; // �յ�����app

NET_MSG_HEAD rx_head; // ͨ��֡�ṹ:ǰ׺

void net_download_app(void) // �������app
{
    printf("�������app\n");
    
    NET_MSG_DOWNLOAD msg;
    msg.header = msg_header;                            // ֡ͷ
    msg.frame_type = 0x01;                              // ֡����
    msg.sub_type = 0x01;                                // ������
    msg.frame_length = sizeof(NET_MSG_DOWNLOAD);        // ֡����
    msg.machine_type = stmflash_cache.machine_type;     // �豸����
    msg.machine_id = stmflash_cache.machine_id;         // �豸ID
    msg.node_id = stmflash_cache.node_id;               // �ɼ���ԪID
    msg.packet_no = 0;                                  // ����
    msg.tail = msg_tail;                                // ֡β
    
    memcpy(&g_rs485_tx_buf, &msg, sizeof(msg));
    
    rs485_send_data(g_rs485_tx_buf, msg.frame_length); // ��������
}

void receive_net_msg(void) // ������������
{
    uint32_t rxlen = g_rs485_rx_cnt;
    delay_ms(100);
    
    if (g_rs485_rx_cnt)
    {
        if (rxlen == g_rs485_rx_cnt)
        {
            printf("���� <<< ");
            printhex(g_rs485_rx_buf, rxlen);
            
            // ����ָ��
            memcpy(&rx_head, &g_rs485_rx_buf, sizeof(rx_head));
            
            printf("����֡���� = %u\n", rxlen);
            printf("ָ��֡���� = %u\n", rx_head.frame_length);
            if (rx_head.header == msg_header) // ���֡ͷ
            {
                if (rxlen >= rx_head.frame_length) // ���֡����
                {
                    uint16_t tmp_tail;
                    memcpy(&tmp_tail, &g_rs485_rx_buf[rx_head.frame_length-2], 2);
                    if (tmp_tail == msg_tail) // ���֡β
                    {
                        // ����豸ID��ɼ���ԪID
                        printf("�����豸ID:%u\n", stmflash_cache.machine_id);
                        printf("ָ���豸ID:%u\n", rx_head.machine_id);
                        printf("�����ɼ���ԪID:%u\n", stmflash_cache.node_id);
                        printf("ָ��ɼ���ԪID:%u\n", rx_head.node_id);
                        if ((rx_head.machine_id == stmflash_cache.machine_id) && (rx_head.node_id == stmflash_cache.node_id))
                        {
                            if (rx_head.frame_type == 0xdd)
                            {
                                analyze_debug_msg(); // ��������ָ��
                            }
                            else
                            {
                                analyze_net_msg(); // ������������
                            }
                        } // end ����豸ID��ɼ���ԪID
                        else
                        {
                            analyze_debug_msg(); // ��������ָ��
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
            rxlen = g_rs485_rx_cnt;
        }
    } // end �������
}

void analyze_net_msg(void) // ������������
{
    printf("֡����:0x%02X\n", rx_head.frame_type);
    printf("������:0x%02X\n", rx_head.sub_type);
    switch (rx_head.frame_type) // ����֡���ͺ�������
    {
        case 0xa1:
            {
                switch (rx_head.sub_type)
                {
                    case 0x01:
                        net_cmd_app(); // ֡����ʶ��:����app
                        break;
                }
            }
            break;
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
    
    if ((0==strncmp(g_at_msg_buf, at_head, sizeof(at_head)-1)) && (0==strncmp(g_at_msg_buf + at_len - sizeof(at_tail) + 1, at_tail, sizeof(at_tail)-1)))
    {
        printf("AT��Ӧ:");
        printstring(g_at_msg_buf, at_len);
        
        if (0==strncmp(g_at_msg_buf, at_msg_registered, sizeof(at_msg_registered)-1)) // ͨ�Ż�վ���ӳɹ�
        {
            printf("ͨ�Ż�վ���ӳɹ�\n");
        }
        else if (0==strncmp(g_at_msg_buf, at_msg_connected, sizeof(at_msg_connected)-1)) // TCP���ӳɹ�
        {
            printf("TCP���ӳɹ�\n");
            LED1(0);
            flag_connected = 1;
        }
        else if (0==strncmp(g_at_msg_buf, at_msg_closed, sizeof(at_msg_closed)-1)) // TCP���ӶϿ�
        {
            printf("TCP���ӶϿ�\n");
            LED1(1);
        }
    }
}

void net_cmd_reboot(void) // ����ָ��:����
{
    printf("����\n");
    
    HAL_NVIC_SystemReset();
}

void net_cmd_app(void) // ����ָ��:����app
{
    printf("����app\n");
    
    flag_update_app = 1;
    
    NET_MSG_APP msg; // ����appָ��ͷ
    memcpy(&msg, &g_rs485_rx_buf, sizeof(NET_MSG_APP));
    
    printf("����汾:%u\n", msg.app_version);
    printf("���򳤶�:%u\n", msg.app_length);
        
    int start = sizeof(NET_MSG_APP);
    
    printf("��ʼ���¹̼�...\n");
    
    iap_write_appbin(FLASH_APP1_ADDR, g_rs485_rx_buf + start, msg.app_length); // ��app���µ�flash
    
    printf("�̼��������!\n");
    
    // ����app�汾��Ϣ
    stmflash_cache.version_id = msg.app_version;
    stmflash_cache.need_update = 0;
    flash_write_cache();
    
    delay_ms(10);
    
    iap_load_app(FLASH_APP1_ADDR); // ִ��app
}

void debug_set_cache(void) // ����ָ��:д�뻺��
{
    printf("����ָ��:д�뻺��\n");
    
    DEBUG_SET_CACHE msg;
    memcpy(&msg, &g_rs485_rx_buf, sizeof(DEBUG_SET_CACHE));
    
    printf("machine_type = %u\n", msg.machine_type);
    printf("machine_id = %u\n", msg.machine_id);
    printf("node_id = %u\n", msg.node_id);
    printf("version_id = %u\n", msg.version_id);
    printf("need_update = %u\n", msg.need_update);
    
    stmflash_cache.machine_type = msg.machine_type;
    stmflash_cache.machine_id = msg.machine_id;
    stmflash_cache.node_id = msg.node_id;
    stmflash_cache.version_id = msg.version_id;
    stmflash_cache.need_update = msg.need_update;
    flash_write_cache();
}

void analyze_debug_msg(void) // ��������ָ��
{
    if (rx_head.frame_type == 0xdd)
    {
        switch (rx_head.sub_type)
        {
            case 0x01:
                debug_set_cache(); // ����ָ��:д�뻺��
                break;
        }
    }
}

void at_sync_timestamp(void) // ATָ��:ͬ����վʱ���
{
    printf("ͬ����վʱ���\n");
    
    char *str = "@DTU:0000:TIMESTAMP";
    send_at_cmd(str, strlen(str));
}

void send_at_cmd(char *str, size_t size) // ����ATָ��(����4Gģ��)
{
    uint8_t array[size];
    string_to_uint8_array(str, array, size);
    
    memcpy(&g_rs485_tx_buf, array, size);
    
    rs485_send_data(g_rs485_tx_buf, size);
}
