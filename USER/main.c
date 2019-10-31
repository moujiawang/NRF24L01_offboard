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
#define RX_BUF_MAX 5
#define TX_BUF_MAX 5

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
u8 Tx_buf[TX_PLOAD_WIDTH] = {0,1};							//Tx_buf�����˻���
u8 i, j;
u8 RX_OK_FLAG = 0;
u8 rx_len = 0;
u8 tx_len = 2;			//��ʼ�ϵ��Ĭ�Ϸ���0x00 ��ʾfault mode�������λ��

u8 command_buf_count = 0;

u8 sta= 0;
u8 Res;
 int main(void)
 {	
	delay_init();	    																			//��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);													//�жϷ���	
	uart_init(460800);	 																			//���ڳ�ʼ��Ϊ115200
	
 	NRF24L01_Init(RX_MODE);    																		//��ʼ��NRF24L01Ϊ����ģʽ 
	EXTIx_Init();

	NRF24L01_ACK_W_Packet(Tx_buf,2);											//�ʼ���͵�����Ϊ����ģʽ�� 0x00
	

	while(1)
	{
 
		if(USART_RX_STA&0x8000)//�������ݽ������.
		{
			tx_len = (u8)(USART_RX_STA&0X3FFF);//ˢ�½��յ���ָ�Ҳ���Ǻ���Ҫ���߷��͵�ָ�����ȡ��tx_len�����ݳ��ȣ�
			for(i = 0; i < tx_len; i++)
				Tx_buf[i] = USART_RX_BUF[i];
			USART_RX_STA = 0;
		}

		if( RX_OK_FLAG ) 																			//���յ�����			
		{
			RX_OK_FLAG =0;
			if((rx_len > 0) && (rx_len < 33))
			{
				NRF24L01_ACK_W_Packet(Tx_buf,tx_len);	
				for(i = 0; i < rx_len; i++)
					USART_TX_BUF[i] = Rx_buf[i];
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
				NRF24L01_FlushTX();	
				NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,RX_OK|TX_OK|MAX_TX);							//������е��жϱ�־
			}
		}
 
	}
}



