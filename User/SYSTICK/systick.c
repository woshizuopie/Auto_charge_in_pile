#include "./SYSTICK/systick.h"

/*  
 *   clock_array[0]  ->  delay_ms()             ms��ʱ��
 *   clock_array[1]  ->  CHARGE_COUNTDOWN_TIMER ��絹��ʱ��ʱ��
 *   clock_array[2]  ->  SLAVE_COMMTASK_TIMER   �ӻ�ͨѶ����ʱ��
 *   clock_array[3]  ->  ADC_CONVERSION_TIMER   ADCת������ʱ��
 *   clock_array[4]  ->  CHARGE_CHECK_TIMER     ���������ʱ��
 *   clock_array[5]  ->  LOW_CURRENT_SAMP_TIMER �͵�����������ʱ��
 *   clock_array[6]  ->  SLAVE_CAN_TIMER        �ӻ�canͨѶ��ʱ��
 *   clock_array[7]  ->  USART_DEBUG_TIMER      ���ڵ��Զ�ʱ��
*/
//ʱ������ �����ڳ�Աÿ10ms��1
uint32_t clock_array[CLOCK_ARRAY_SZIE]={0};

//�δ�ʱ����ʼ��
void Systick_Init(void)
{
	SysTick_Config(36000);	//t=36000*(1/36M)=1ms
}

//ms��ʱ����
void delay_ms(uint32_t ms)
{
	clock_array[0]=ms;
	while(clock_array[0]);
}

//ϵͳ�δ�ʱ���жϷ�����
void SysTick_Handler(void)
{
	uint8_t i=0;
	for(i=0;i<CLOCK_ARRAY_SZIE;i++)
	{
		if(clock_array[i]!=0)
			clock_array[i]--;
	}
}



