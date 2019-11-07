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


#define FOUR_WINGS

#ifdef TWO_WINGS

//上行数据长度
#define FAULT_MODE_UPDATA_LENGTH  21
#define START_MODE_UPDATA_LENGTH  23
#define MANUAL_MODE_UPDATA_LENGTH 23
#define FLIGHT_MODE_UPDATA_LENGTH 30
#define TUNING_MODE_UPDATA_LENGTH 31

//下行数据长度
#define FAULT_MODE_DOWNDATA_LENGTH 1
#define START_MODE_DOWNDATA_LENGTH  5
#define MANUAL_MODE_DOWNDATA_LENGTH 6
#define FLIGHT_MODE_DOWNDATA_LENGTH 6
#define TUNING_MODE_DOWNDATA_LENGTH 24

#endif

#ifdef FOUR_WINGS

//上行数据长度
#define FAULT_MODE_UPDATA_LENGTH  20
#define START_MODE_UPDATA_LENGTH  24
#define MANUAL_MODE_UPDATA_LENGTH 24
#define FLIGHT_MODE_UPDATA_LENGTH 24
#define TUNING_MODE_UPDATA_LENGTH 31

//下行数据长度
#define FAULT_MODE_DOWNDATA_LENGTH  1
#define START_MODE_DOWNDATA_LENGTH  7
#define MANUAL_MODE_DOWNDATA_LENGTH 7
#define FLIGHT_MODE_DOWNDATA_LENGTH 7
#define TUNING_MODE_DOWNDATA_LENGTH 27

#endif


typedef enum{ 	
				FAULT_MODE = 0x00, 
				START_MODE = 0x08, 
				MANUAL_MODE = 0x10,
				FLIGHT_MODE = 0x18,
				TUNING_MODE = 0x38 
			} MODE_ID;									//FAULT_MODE = 0x00, START_MODE = 0x08, MANUAL_MODE = 0x10,FLIGHT_MODE = 0x18,TUNING_MODE = 0x38

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

u8 T = 0;
u8 Rx_buf[RX_PLOAD_WIDTH] = {0};
u8 Tx_buf[TX_PLOAD_WIDTH] = {0,1};							//Tx_buf设置了缓冲
u8 i, j;
u8 RX_OK_FLAG = 0;
u8 rx_len = 0;
u8 tx_len = 2;			//开始上电后，默认发送0x00 表示fault mode的命令到下位机

u8 command_buf_count = 0;

u8 sta= 0;
u8 Res;
u8 mode = 0;

 int main(void)
 {	
	delay_init();	    																			//延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);													//中断分组	
	uart_init(460800);	 																			//串口初始化为460800
	
 	NRF24L01_Init(RX_MODE);    																		//初始化NRF24L01为接收模式 
	EXTIx_Init();

	NRF24L01_ACK_W_Packet(Tx_buf,2);											//最开始发送的数据为故障模式， 0x00
	

	while(1)
	{
		
		if(USART_RX_STA&0x8000)//串口数据接收完成.
		{
			tx_len = (u8)(USART_RX_STA&0X3FFF);//刷新接收到的指令（也就是后面要无线发送的指令，所以取名tx_len）数据长度，
			for(i = 0; i < tx_len; i++)
				Tx_buf[i] = USART_RX_BUF[i];
			mode = Tx_buf[0] & 0xfe;
			switch(mode)
			{
				case FAULT_MODE:
				{
					if(tx_len == FAULT_MODE_DOWNDATA_LENGTH)
						NRF24L01_ACK_W_Packet(Tx_buf,tx_len);	
				}break;
				case START_MODE:
				{
					if(tx_len == START_MODE_DOWNDATA_LENGTH)
						NRF24L01_ACK_W_Packet(Tx_buf,tx_len);					
				}break;
				case MANUAL_MODE:
				{
					if(tx_len == MANUAL_MODE_DOWNDATA_LENGTH)
						NRF24L01_ACK_W_Packet(Tx_buf,tx_len);						
				}break;
				case FLIGHT_MODE:
				{
					if(tx_len == FLIGHT_MODE_DOWNDATA_LENGTH)
						NRF24L01_ACK_W_Packet(Tx_buf,tx_len);					
				}break;
				case TUNING_MODE:
				{
					if(tx_len == TUNING_MODE_DOWNDATA_LENGTH)
						NRF24L01_ACK_W_Packet(Tx_buf,tx_len);							
				}break;	
				default:;break;
			}
			USART_RX_STA = 0;
		}


		if( RX_OK_FLAG ) 																			//接收到数据			
		{
			RX_OK_FLAG =0;
			if((rx_len > 0) && (rx_len < 33))
			{
				for(i = 0; i < rx_len; i++)
					USART_TX_BUF[i] = Rx_buf[i];
				for(i =0;i < rx_len ; i++)
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



