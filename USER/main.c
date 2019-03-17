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
u8 Rx_buf[RX_PLOAD_WIDTH] = {0};
u8 Tx_buf[TX_PLOAD_WIDTH] = {0};

SYS_STATUS SYS_status; 

 int main(void)
 {	
	delay_init();	    																			//延时函数初始化	  
	
	uart_init(115200);	 																			//串口初始化为115200
	
 	NRF24L01_Init(RX_MODE);    																		//初始化NRF24L01为接收模式 

	while( NRF24L01_Check() );	
	SYS_status.DTU_NRF_Status |= NRF_ON;															//有NRF在线
	
	Tx_buf[0]=2;																					//握手码赋值
	NRF24L01_ACK_W_Packet(Tx_buf,1);
	
	do
	{
		if(NRF24L01_RxPacket(Rx_buf) == 0)															//接收到信号
		{
			SYS_status.DTU_NRF_Status |= NRF_CONNECTED;												//通讯成功					
			break;
		}
		SYS_status.DTU_NRF_Status &= NRF_DISCONNECTED;												//通讯失败
	}
	while(1);

	while(1)
	{

		RX_Result = NRF24L01_Read_Reg(STATUS);  													//读取状态寄存器的值
		if( RX_Result & RX_OK )																		//接收到数据			
		{
			u8 rx_len = NRF24L01_Read_Reg(R_RX_PL_WID);												//读取接收到的数据长度
			if((rx_len > 0)&& (rx_len < 33))
			{
				NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,(RX_Result & RX_OK));						//清除TX_DS或MAX_RT中断标志
				NRF24L01_Read_Buf(RD_RX_PLOAD,Rx_buf,rx_len);										//读取数据
				RX_Result = NRF24L01_Read_Reg(NRF_FIFO_STATUS);										//查看RX FIFO此时的状态
				
				NRF24L01_ACK_W_Packet(Tx_buf,2);													//更新应答数据包
				
				printf("\r\n接收到的数据长度为： %d\r\n", rx_len);								
				printf("STATUS_DATA = %d\r\n",Rx_buf[0]);
				printf("Fly_Pulse = %d\r\n",Rx_buf[1]);
				printf("Climb_Pulse = %d\r\n",Rx_buf[2]);
				printf("Pitch_Pulse = %d\r\n", Rx_buf[3]);
				printf("Roll_Pulse = %d\r\n", Rx_buf[4]);
				printf("Yaw_Pulse = %d\r\n",Rx_buf[5]);
				printf("test = %d\r\n",Rx_buf[6]);
			}
			else//如果数据长度异常则丢弃数据		
			{
				NRF24L01_FlushRX();																		
			}
		}

		if( USART_GetFlagStatus(USART1, USART_FLAG_RXNE))											//串口接收到数据
		{
			Tx_buf[0] = ACTUATOR_MODE;     
			Tx_buf[1] = (u8)USART_ReceiveData(USART1); 
		}
 
	}
}



