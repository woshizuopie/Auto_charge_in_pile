#ifndef _BSP_LED_H
#define _BSP_LED_H

#include "stm32f10x.h"

#define LED_G_GPIO_PIN					GPIO_Pin_0
#define LED_G_GPIO_PORT					GPIOB
#define LED_G_GPIO_CLK					RCC_APB2Periph_GPIOB


#define ON 											1
#define OFF											0

//	\ C������������з������治�����κζ���
#define	LED_G(a)	if(a) \
										GPIO_SetBits(LED_G_GPIO_PORT, LED_G_GPIO_PIN);\
									else \
										GPIO_ResetBits(LED_G_GPIO_PORT, LED_G_GPIO_PIN);

void LED_GPIO_Config(void);		//��ʼ��GPIO

#endif  /* _BSP_LED_H */
