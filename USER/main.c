#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"	 
#include "24l01.h" 	 
#include "nrf_protocol.h"
#include "exti.h"
#include "Encoder.h"
 
 
#define TIM4_PERIOD 8000
#define RX_BUF_MAX 5
#define TX_BUF_MAX 5

#define  FAULT_MODE 0x00
#define START_MODE 0x08
#define MANUAL_MODE 0x10
#define FLIGHT_MODE 0x18
#define TUNING_MODE 0x38 

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
u8 Rx_buf[RX_PLOAD_WIDTH+4] = {0};
u8 Tx_buf[TX_PLOAD_WIDTH+4] = {0,1};							//Tx_buf�����˻���
u8 i, j;
u8 RX_OK_FLAG = 0;
u8 rx_len = 0;
u8 tx_len = 2;			//��ʼ�ϵ��Ĭ�Ϸ���0x00 ��ʾfault mode�������λ��

u8 command_buf_count = 0;


u32 rate_value = 0; 
uint16_t rate_value_new = 0;
uint16_t rate_value_last = 0;
float rate= 0.0;

u16 count =0;
u16 current_count = 32768;
u16 current_count_last = 32768;

int8_t direction = 1;

int8_t cycle_count = 0;
int16_t angle_count = 0;
u8 sta= 0;
u8 Res;
	float angle = 0;
u8 start_flag = 0;//��start_flag =0 ʱ��ʾ��û�н��չ�һ�����ݣ��ոտ�ʼ

 int main(void)
{	
	float angle = 0;
	float angle_init = 0;
	int16_t angle_init_count = 0;
	u8 mode_id = 0;
	delay_init();	    											//��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);					//�жϷ���	
	uart_init(115200);	 											//���ڳ�ʼ��Ϊ115200
	
 	NRF24L01_Init(RX_MODE);    										//��ʼ��NRF24L01Ϊ����ģʽ 
	EXTIx_Init();
	
//	Encoder_init();
	
	NRF24L01_ACK_W_Packet(Tx_buf,2);								//�ʼ���͵�����Ϊ����ģʽ�� 0x00
	

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
				if(start_flag == 0)			//����һ�ν��ܵ�����ʱ		
				{
					Encoder_init();
					mode_id = Rx_buf[0];
					start_flag = 1;			//��ʾ�Ѿ����ܹ�һ������
					switch(mode_id)			//�����ܵĵ��ĵ�һ������������ֵ��Ϊ�������Ƕȵ����
					{
						case MANUAL_MODE:
						{
							angle_init = (float)(*(int16_t *)&Rx_buf[5]);
						}break;
						case FLIGHT_MODE:
						{
							angle_init = (float)(*(int16_t *)&Rx_buf[5]);
						}break;
						case TUNING_MODE:
						{
							angle_init = (float)(*(int16_t *)&Rx_buf[1]);
						}break;
						case START_MODE:
						{
							angle_init = (float)(*(int16_t *)&Rx_buf[5]);
						}break;
						case FAULT_MODE:
						{
							angle_init = (float)(*(int16_t *)&Rx_buf[5]);
						}break;
					}
				}
				
				NRF24L01_ACK_W_Packet(Tx_buf,tx_len);	
				for(i = 0; i < rx_len; i++)
					USART_TX_BUF[i] = Rx_buf[i];
				
				angle_init_count = (int16_t)angle_init / 9; 
				cycle_count = (current_count - 30768 + angle_init_count) / 4000;
				angle_count = (current_count - 30768 + angle_init_count) % 4000;
				if(angle_count > 0 )
					angle = (angle_count -2000) * 0.09;
				else
					if(angle_count == 0)
						angle = -180;
					if(angle_count < 0)
						angle = (angle_count + 2000) *0.09;
				 //angle = ((current_count - 32768) - (((int16_t)current_count - 30768)/4000) * 4000 + (uint16_t)(angle_init / 0.09)) * 0.09;
				if(rate_value != 0)
					rate = 	direction * 36000.0 / (float)rate_value;
				else	
					rate = 0;
				
				*(int16_t *)&USART_TX_BUF[i] = (int16_t)(angle *100);
				*(int16_t *)&USART_TX_BUF[i+2] = (int16_t)(rate * 10);
				
				for(i =0;i < rx_len+4 ; i++)
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


void TIM4_IRQHandler()
{
	if( TIM_GetITStatus(TIM4,TIM_IT_CC2) == 1 )
	{
		rate_value_last = rate_value_new;
		rate_value_new = TIM_GetCapture2(TIM4);
		rate_value = (u32)((int32_t)(rate_value_new - rate_value_last)  + (int32_t)count * 65536);
		count = 0;
	}
	if(TIM_GetITStatus(TIM4,TIM_IT_Update) == 1)
	{
		if(start_flag >0)
		{
			start_flag++;
			if(start_flag > 2)			//˵��2*655.35ms�ڣ�û�н��յ�һ���������ݣ�˵��ͨѶ�ϵ���
			{
				start_flag = 0;			//start_flag = 0 ���յ��������ݺ����¶Ա������ĽǶȸ���ֵ
				Encoder_Deinit();
			}

		}

		if(count == 2)
		{
			count = 0;
			rate_value = 0;	
		}
		else
			count++;
		TIM_ClearFlag(TIM4,TIM_IT_Update);
	}
}


