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
u8 Rx_buf[RX_PLOAD_WIDTH+4] = {0};
u8 Tx_buf[TX_PLOAD_WIDTH+4] = {0,1};							//Tx_buf设置了缓冲
u8 i, j;
u8 RX_OK_FLAG = 0;
u8 rx_len = 0;
u8 tx_len = 2;			//开始上电后，默认发送0x00 表示fault mode的命令到下位机

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
u8 start_flag = 0;//当start_flag =0 时表示还没有接收过一个数据，刚刚开始

 int main(void)
{	
	float angle = 0;
	float angle_init = 0;
	int16_t angle_init_count = 0;
	u8 mode_id = 0;
	delay_init();	    											//延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);					//中断分组	
	uart_init(115200);	 											//串口初始化为115200
	
 	NRF24L01_Init(RX_MODE);    										//初始化NRF24L01为接收模式 
	EXTIx_Init();
	
//	Encoder_init();
	
	NRF24L01_ACK_W_Packet(Tx_buf,2);								//最开始发送的数据为故障模式， 0x00
	

	while(1)
	{
 
		if(USART_RX_STA&0x8000)//串口数据接收完成.
		{
			tx_len = (u8)(USART_RX_STA&0X3FFF);//刷新接收到的指令（也就是后面要无线发送的指令，所以取名tx_len）数据长度，
			for(i = 0; i < tx_len; i++)
				Tx_buf[i] = USART_RX_BUF[i];
			USART_RX_STA = 0;
		}

		if( RX_OK_FLAG ) 																			//接收到数据			
		{
			RX_OK_FLAG =0;
			if((rx_len > 0) && (rx_len < 33))
			{
				if(start_flag == 0)			//当第一次接受到数据时		
				{
					Encoder_init();
					mode_id = Rx_buf[0];
					start_flag = 1;			//表示已经接受过一个数据
					switch(mode_id)			//将接受的到的第一个陀螺仪数据值作为编码器角度的起点
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
					USART_SendData(USART1, USART_TX_BUF[i]);				//向串口1发送数据
					while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);	//等待发送结束
				}
				USART_SendData(USART1, 0x0d);								//向串口1发送0x0d
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);		//等待发送结束
				USART_SendData(USART1, 0x0a);								//向串口1发送0x0a
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);		//等待发送结束
			}
			else//如果数据长度异常则丢弃数据		
			{
				NRF24L01_FlushRX();	
				NRF24L01_FlushTX();	
				NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,RX_OK|TX_OK|MAX_TX);							//清除所有的中断标志
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
			if(start_flag > 2)			//说明2*655.35ms内，没有接收到一次无线数据，说明通讯断掉了
			{
				start_flag = 0;			//start_flag = 0 接收到无线数据后，重新对编码器的角度赋初值
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


