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
 ALIENTEK战舰STM32开发板实验33
 无线通信 实验
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
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
	delay_init();	    																			//延时函数初始化	 
		
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM4, ENABLE);
	 
	TIM_TimeBaseInitStruct.TIM_Prescaler = (9-1);						//72MHZ/9等于定时器计数器1秒钟计数的次数，也就是8MHZ，那每计数一次时间为0.000125ms
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Period = TIM4_PERIOD;				//计数8000次,对应时间是1ms
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;//设置时钟分割:TDTS = Tck_tim

	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStruct);
	
   										 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);					//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	NVIC_InitStruct.NVIC_IRQChannel = TIM4_IRQn;						//设置初始化的是TIM4的中断	
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;	//设置抢占优先级为2
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;					//设置响应优先级（子优先级）为0
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;						//使能定时器4这个中断
	NVIC_Init(&NVIC_InitStruct);
	
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);								//使能TIM4的TIM_IT_Update中断
	TIM_Cmd(TIM4, ENABLE);																	//使能定时器，开始计数	 
	
	uart_init(115200);	 																		//串口初始化为115200

 	NRF24L01_Init();    																		//初始化NRF24L01 

	while( NRF24L01_Check() );
	SYS_status.DTU_NRF_Status |= NRF_ON;										//有NRF在线
	NRF24L01_PowerDown_Mode();
	NRF24L01_RX_Mode();
	do
	{
		if(NRF24L01_RxPacket(Rx_buf) == 0)										//接收到信号
		{
			Tx_buf[0] = ~Rx_buf[0];
			NRF24L01_PowerDown_Mode();
			NRF24L01_TX_Mode();
			TX_Result = NRF24L01_TxPacket(Tx_buf);							//收到信号后，发送握手码
			SYS_status.DTU_NRF_Status |= NRF_CONNECTED;					//程序这里是由bug的，也就是offboard端发送了握手码，但是不知道握手结果，而且是否发送成功也没法验证
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
			printf("\r\n接收到的数据为：\r\n");
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


