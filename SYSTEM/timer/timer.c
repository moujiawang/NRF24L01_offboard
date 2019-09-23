#include "timer.h"
#include "stm32f10x_tim.h"


void timer_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_ICInitTypeDef TIM_ICInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	TIM_DeInit(TIM3);
	
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_Period = 65535;		 //������Ϊ1000��
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	
	//��ʱ��������ģʽ �½��� ������ѡ��
	TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_Falling ,TIM_ICPolarity_Falling);
	TIM_ICStructInit(&TIM_ICInitStructure);
	//�������˲� This parameter must be a value between 0x00 and 0x0F.
	TIM_ICInitStructure.TIM_ICFilter = 0x0f;
    TIM_ICInit(TIM3, &TIM_ICInitStructure);
	//�ö�ʱ����ʼֵ
	TIM_SetCounter(TIM3, 32768);
	TIM_ClearFlag(TIM3, TIM_FLAG_Update);
	TIM_Cmd(TIM3,ENABLE);
}
