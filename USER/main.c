#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"	 
#include "24l01.h" 	 
#include "nrf_protocol.h"
 
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

SYS_STATUS SYS_status; 

 int main(void)
 {	
	delay_init();	    																			//��ʱ������ʼ��	  
	
	uart_init(115200);	 																			//���ڳ�ʼ��Ϊ115200
	
 	NRF24L01_Init(RX_MODE);    																		//��ʼ��NRF24L01Ϊ����ģʽ 

	while( NRF24L01_Check() );	
	SYS_status.DTU_NRF_Status |= NRF_ON;															//��NRF����
	
	Tx_buf[0]=2;																					//�����븳ֵ
	NRF24L01_ACK_W_Packet(Tx_buf,1);
	
	do
	{
		if(NRF24L01_RxPacket(Rx_buf) == 0)															//���յ��ź�
		{
			SYS_status.DTU_NRF_Status |= NRF_CONNECTED;												//ͨѶ�ɹ�					
			break;
		}
		SYS_status.DTU_NRF_Status &= NRF_DISCONNECTED;												//ͨѶʧ��
	}
	while(1);

	while(1)
	{

		RX_Result = NRF24L01_Read_Reg(STATUS);  													//��ȡ״̬�Ĵ�����ֵ
		if( RX_Result & RX_OK )																		//���յ�����			
		{
			u8 rx_len = NRF24L01_Read_Reg(R_RX_PL_WID);												//��ȡ���յ������ݳ���
			if((rx_len > 0)&& (rx_len < 33))
			{
				NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,(RX_Result & RX_OK));						//���TX_DS��MAX_RT�жϱ�־
				NRF24L01_Read_Buf(RD_RX_PLOAD,Rx_buf,rx_len);										//��ȡ����
				RX_Result = NRF24L01_Read_Reg(NRF_FIFO_STATUS);										//�鿴RX FIFO��ʱ��״̬
				
				NRF24L01_ACK_W_Packet(Tx_buf,2);													//����Ӧ�����ݰ�
				
				printf("\r\n���յ������ݳ���Ϊ�� %d\r\n", rx_len);								
				printf("STATUS_DATA = %d\r\n",Rx_buf[0]);
				printf("Fly_Pulse = %d\r\n",Rx_buf[1]);
				printf("Climb_Pulse = %d\r\n",Rx_buf[2]);
				printf("Pitch_Pulse = %d\r\n", Rx_buf[3]);
				printf("Roll_Pulse = %d\r\n", Rx_buf[4]);
				printf("Yaw_Pulse = %d\r\n",Rx_buf[5]);
				printf("test = %d\r\n",Rx_buf[6]);
			}
			else//������ݳ����쳣��������		
			{
				NRF24L01_FlushRX();																		
			}
		}

		if( USART_GetFlagStatus(USART1, USART_FLAG_RXNE))											//���ڽ��յ�����
		{
			Tx_buf[0] = ACTUATOR_MODE;     
			Tx_buf[1] = (u8)USART_ReceiveData(USART1); 
		}
 
	}
}



