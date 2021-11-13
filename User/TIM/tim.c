#include "./TIM/tim.h"

static void TIM4_Config(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
  
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
	TIM_TimeBaseStruct.TIM_Prescaler = TIM4_Prescaler;
	TIM_TimeBaseStruct.TIM_Period = TIM4_Period;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStruct);
	
	TIM_ClearFlag(TIM4, TIM_FLAG_Update);	      //����жϱ�־λ
	TIM_ITConfig (TIM4, TIM_IT_Update, ENABLE);	//���ö�ʱ���ж�
	TIM_Cmd(TIM4, ENABLE);	                    //ʹ�ܶ�ʱ��
}

// �ж����ȼ�����
static void TIM4_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;	
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	
	NVIC_Init(&NVIC_InitStructure);
}


//������ʱ����ʼ�� 10usһ���ж�
void TIM4_Init(void)
{
	TIM4_NVIC_Config();	//�����ж����ȼ�
	TIM4_Config();				//������ʱ������
}

uint32_t timer4_count=0;
void TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4,TIM_IT_Update)==SET)
	{
    timer4_count++;
		TIM_ClearITPendingBit(TIM4,TIM_IT_Update);		//����жϱ�־λ
	}
}

/* 10us��ʱ���� */
void Tim4_Delay_x10us(uint32_t x10us)
{
  /* ������ʱ��  */
  timer4_count=0;
  TIM4->CNT=0;
  TIM_Cmd(TIM4, ENABLE);	    //�رն�ʱ��
  
  while(timer4_count<x10us);  //�ȴ���ʱ���

  /* �رն�ʱ��  */
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
