#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"	 
#include "24l01.h" 	 
#include "nrf_protocol.h"
#include "exti.h"
 
#define TIM4_PERIOD 8000
#define Tx_mode_Flag 0
#define Rx_mode_Flag 0xff
/************************************************
 ALIENTEKս��STM32������ʵ��33
 ����ͨ�� ʵ��
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/

u8 RX_Result;
u8 TX_Result;
u8 Mode = 0xff;
u8 T = 0;
u8 Rx_buf[RX_PLOAD_WIDTH] = {0};
u8 Tx_buf[TX_PLOAD_WIDTH] = {0};
u8 i;
u8 RX_OK_FLAG = 0;
u8 rx_len = 0;

//SYS_STATUS SYS_status; 

 int main(void)
 {	
	delay_init();	    																			//��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);													//�жϷ���	
	uart_init(115200);	 																			//���ڳ�ʼ��Ϊ115200
	
 	NRF24L01_Init(RX_MODE);    																		//��ʼ��NRF24L01Ϊ����ģʽ 
	EXTIx_Init();

//	SYS_status.DTU_NRF_Status |= NRF_ON;															//��NRF����
	NRF24L01_ACK_W_Packet(Tx_buf,1);																//�ʼ���͵�����Ϊ����ģʽ�� 0x00
	
/*	do
	{
		if(NRF24L01_RxPacket(Rx_buf) == 0)															//���յ��ź�
		{
			SYS_status.DTU_NRF_Status |= NRF_CONNECTED;												//ͨѶ�ɹ�					
			break;
		}
		SYS_status.DTU_NRF_Status &= NRF_DISCONNECTED;												//ͨѶʧ��
	}
	while(1);
*/

	while(1)
	{
		if(USART_RX_STA&0x8000)//�������ݽ������.
		{
			for(i = 0; i < (USART_RX_STA&0X3FFF); i++)
				Tx_buf[i] = USART_RX_BUF[i];
			NRF24L01_ACK_W_Packet(Tx_buf,i);			
		}
		
//		RX_Result = NRF24L01_Read_Reg(STATUS);  													//��ȡ״̬�Ĵ�����ֵ
		if( RX_OK_FLAG ) 																			//���յ�����			
		{
			RX_OK_FLAG =0;
			rx_len = NRF24L01_Read_Reg(R_RX_PL_WID);												//��ȡ���յ������ݳ���
			if((rx_len > 0)&& (rx_len < 33))
			{
				NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,(RX_Result & RX_OK));						//���TX_DS��MAX_RT�жϱ�־
				NRF24L01_Read_Buf(RD_RX_PLOAD,Rx_buf,rx_len);										//��ȡ����
//				RX_Result = NRF24L01_Read_Reg(NRF_FIFO_STATUS);										//�鿴RX FIFO��ʱ��״̬
				for(i = 0; i < rx_len; i++)
					USART_TX_BUF[i] = USART_RX_BUF[i];
				NRF24L01_ACK_W_Packet(Tx_buf,i);
				for(i =0;i < rx_len ; i++)
				{
					USART_SendData(USART1, USART_TX_BUF[i]);				//�򴮿�1��������
					while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);	//�ȴ����ͽ���
				}
				USART_SendData(USART1, 0x0d);								//�򴮿�1����0x0d
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);		//�ȴ����ͽ���
				USART_SendData(USART1, 0x0a);								//�򴮿�1����0x0a
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);		//�ȴ����ͽ���
			}
			else//������ݳ����쳣��������		
			{
				NRF24L01_FlushRX();																		
			}
		}
 
	}
}



