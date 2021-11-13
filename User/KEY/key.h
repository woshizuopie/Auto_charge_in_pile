#ifndef __KEY_H_
#define __KEY_H_

#include "stm32f10x.h"
#include "./sysTick/sysTick.h"

#define KEY_GPIO_APBx_ClkCmd		RCC_APB2PeriphClockCmd
#define KEY_GPIO_CLK						RCC_APB2Periph_GPIOA

/* KEY1 PA8 */
#define KEY1_GPIO_PORT					GPIOA
#define KEY1_GPIO_PIN						GPIO_Pin_10

/* KEY2 PB3 */
#define KEY2_GPIO_PORT					GPIOA
#define KEY2_GPIO_PIN						GPIO_Pin_11


#define KEY1		GPIO_ReadInputDataBit(KEY1_GPIO_PORT,KEY1_GPIO_PIN)
#define KEY2		GPIO_ReadInputDataBit(KEY2_GPIO_PORT,KEY2_GPIO_PIN)

#define KEY1_ON			1
#define KEY2_ON			2


void KEY_Config(void);
uint8_t KEY_Scan(uint8_t mode);

#endif	/* __KEY_H_ */

