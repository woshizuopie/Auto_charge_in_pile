#include "./SYSTICK/systick.h"

/*  
 *   clock_array[0]  ->  delay_ms()             ms定时器
 *   clock_array[1]  ->  CHARGE_COUNTDOWN_TIMER 充电倒计时定时器
 *   clock_array[2]  ->  SLAVE_COMMTASK_TIMER   从机通讯任务定时器
 *   clock_array[3]  ->  ADC_CONVERSION_TIMER   ADC转化任务定时器
 *   clock_array[4]  ->  CHARGE_CHECK_TIMER     充电检测任务定时器
 *   clock_array[5]  ->  LOW_CURRENT_SAMP_TIMER 低电流采样任务定时器
 *   clock_array[6]  ->  SLAVE_CAN_TIMER        从机can通讯定时器
 *   clock_array[7]  ->  USART_DEBUG_TIMER      串口调试定时器
*/
//时钟数组 数组内成员每10ms减1
uint32_t clock_array[CLOCK_ARRAY_SZIE]={0};

//滴答定时器初始化
void Systick_Init(void)
{
	SysTick_Config(36000);	//t=36000*(1/36M)=1ms
}

//ms延时函数
void delay_ms(uint32_t ms)
{
	clock_array[0]=ms;
	while(clock_array[0]);
}

//系统滴答定时器中断服务函数
void SysTick_Handler(void)
{
	uint8_t i=0;
	for(i=0;i<CLOCK_ARRAY_SZIE;i++)
	{
		if(clock_array[i]!=0)
			clock_array[i]--;
	}
}



