#ifndef __TIM_H_
#define __TIM_H_

#include "stm32f10x.h"

//Ԥ��Ƶֵ
#define TIM4_Prescaler				(1-1)
//�Զ���װ��ֵ
#define	TIM4_Period						(360-1)

extern uint32_t timer4_count;

void TIM4_Init(void);
void TIM4_IRQHandler(void);
void Tim4_Delay_x10us(uint32_t x10us);
void Tim4_Delay_ms(uint32_t ms);

#endif /*__TIM_H_*/
