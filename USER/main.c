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
u8 Rx_buf[RX_PLOAD_WIDTH];
u8 Tx_buf[TX_PLOAD_WIDTH];

SYS_STATUS SYS_status; 

 int main(void)
 {	 
	u16 i=0;			
	 
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	delay_init();	    																			//��ʱ������ʼ��	 
		
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM4, ENABLE);
	 
	TIM_TimeBaseInitStruct.TIM_Prescaler = (9-1);						//72MHZ/9���ڶ�ʱ��������1���Ӽ����Ĵ�����Ҳ����8MHZ����ÿ����һ��ʱ��Ϊ0.000125ms
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Period = TIM4_PERIOD;				//����8000��,��Ӧʱ����1ms
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;//����ʱ�ӷָ�:TDTS = Tck_tim

	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStruct);
	
   										 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);					//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	NVIC_InitStruct.NVIC_IRQChannel = TIM4_IRQn;						//���ó�ʼ������TIM4���ж�	
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;	//������ռ���ȼ�Ϊ2
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;					//������Ӧ���ȼ��������ȼ���Ϊ0
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;						//ʹ�ܶ�ʱ��4����ж�
	NVIC_Init(&NVIC_InitStruct);
	
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);								//ʹ��TIM4��TIM_IT_Update�ж�
	TIM_Cmd(TIM4, ENABLE);																	//ʹ�ܶ�ʱ������ʼ����	 
	
	uart_init(115200);	 																		//���ڳ�ʼ��Ϊ115200

 	NRF24L01_Init();    																		//��ʼ��NRF24L01 

	while( NRF24L01_Check() );
	SYS_status.DTU_NRF_Status |= NRF_ON;										//��NRF����
	NRF24L01_PowerDown_Mode();
	NRF24L01_RX_Mode();
	do
	{
		if(NRF24L01_RxPacket(Rx_buf) == 0)										//���յ��ź�
		{
			Tx_buf[0] = ~Rx_buf[0];
			NRF24L01_PowerDown_Mode();
			NRF24L01_TX_Mode();
			TX_Result = NRF24L01_TxPacket(Tx_buf);							//�յ��źź󣬷���������
			SYS_status.DTU_NRF_Status |= NRF_CONNECTED;					//������������bug�ģ�Ҳ����offboard�˷����������룬���ǲ�֪�����ֽ���������Ƿ��ͳɹ�Ҳû����֤
			break;
		}
	}
	while(1);
	

	while(1)
	{
		if(Mode == Tx_mode_Flag)
		{
			NRF24L01_PowerDown_Mode();
			NRF24L01_TX_Mode();
			TX_Result = NRF24L01_TxPacket(Tx_buf);
			NRF24L01_PowerDown_Mode();
			NRF24L01_RX_Mode();
			Mode=Rx_mode_Flag;
		}
		RX_Result = NRF24L01_RxPacket(Rx_buf);
		if( RX_Result == 0 )
		{
			printf("\r\n���յ�������Ϊ��\r\n");
//			for(i = 0; i < RX_PLOAD_WIDTH; i++)
//			{
//				while( USART_GetFlagStatus(USART1,USART_FLAG_TXE) == 0);
//				USART_SendData(USART1,Rx_buf[i]);
			printf("STATUS_DATA = %d\r\n",Rx_buf[0]);
			printf("Fly_Pulse = %d\r\n",Rx_buf[1]);
			printf("Climb_Pulse = %d\r\n",Rx_buf[2]);
			printf("Roll_Pulse = %d\r\n", Rx_buf[3]);
			printf("Pitch_Pulse = %d\r\n", Rx_buf[4]);
			printf("Yaw_Pulse = %d\r\n",Rx_buf[5] );
			printf("test = %d\r\n",Rx_buf[6]);
//			}

		}
		else
		{

		}
		Tx_buf[0] = ACTUATOR_MODE;     
		Tx_buf[1] = (u8)USART_ReceiveData(USART1);  
	}
}
 void TIM4_IRQHandler()
{
	if(TIM_GetITStatus(TIM4,TIM_IT_Update) == 1)
	{
		T++;
		if(T == 50)
		{
			Mode = Tx_mode_Flag;
			T = 0;
		}

		TIM_ClearFlag(TIM4,TIM_IT_Update);
	}
}


