#include "./TIM/tim.h"

static void TIM4_Config(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
  
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
	TIM_TimeBaseStruct.TIM_Prescaler = TIM4_Prescaler;
	TIM_TimeBaseStruct.TIM_Period = TIM4_Period;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStruct);
	
	TIM_ClearFlag(TIM4, TIM_FLAG_Update);	      //清楚中断标志位
	TIM_ITConfig (TIM4, TIM_IT_Update, ENABLE);	//配置定时器中断
	TIM_Cmd(TIM4, ENABLE);	                    //使能定时器
}

// 中断优先级配置
static void TIM4_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;	
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	
	NVIC_Init(&NVIC_InitStructure);
}


//基础定时器初始化 10us一次中断
void TIM4_Init(void)
{
	TIM4_NVIC_Config();	//配置中断优先级
	TIM4_Config();				//基础定时器配置
}

uint32_t timer4_count=0;
void TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4,TIM_IT_Update)==SET)
	{
    timer4_count++;
		TIM_ClearITPendingBit(TIM4,TIM_IT_Update);		//清楚中断标志位
	}
}

/* 10us延时函数 */
void Tim4_Delay_x10us(uint32_t x10us)
{
  /* 开启定时器  */
  timer4_count=0;
  TIM4->CNT=0;
  TIM_Cmd(TIM4, ENABLE);	    //关闭定时器
  
  while(timer4_count<x10us);  //等待延时完成

  /* 关闭定时器  */
  TIM_Cmd(TIM4, DISABLE);
}

void Tim4_Delay_ms(uint32_t ms)
{
  while(ms)
  {
    Tim4_Delay_x10us(100);
    ms--;
  }
}
