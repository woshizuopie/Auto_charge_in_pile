#ifndef __SYSTICK_H
#define __SYSTICK_H

#include "stm32f10x.h"

#define CLOCK_ARRAY_SZIE    8
extern uint32_t clock_array[CLOCK_ARRAY_SZIE];

#define CHARGE_COUNTDOWN_TIMER      clock_array[1]
#define SLAVE_COMMTASK_TIMER        clock_array[2]
#define ADC_CONVERSION_TIMER        clock_array[3]
#define CHARGE_CHECK_TIMER          clock_array[4]
#define LOW_CURRENT_SAMP_TIMER      clock_array[5]
#define SLAVE_CAN_TIMER             clock_array[6]
#define USART_DEBUG_TIMER           clock_array[7]

void Systick_Init(void);
void delay_ms(uint32_t ms);

#endif /* __SYSTICK_H */

