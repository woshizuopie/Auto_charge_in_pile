#ifndef __USART_H
#define	__USART_H

#include "stm32f10x.h"
#include <stdio.h>
	

#define  	USART1_BAUDRATE					115200
#define 	USART2_BAUDRATE					115200

extern uint8_t USART2_RX_STA;
extern uint8_t USART2_RX_Buf[2];


void USART1_Config(void);
void USART2_Config(void);
void USART2_IRQHandler(void);
#endif /* __USART_H */
