#ifndef _BSP_LED_H
#define _BSP_LED_H

#include "stm32f10x.h"

#define LED_R_GPIO_PIN					GPIO_Pin_8
#define LED_R_GPIO_PORT					GPIOB
#define LED_R_GPIO_CLK					RCC_APB2Periph_GPIOB

#define LED_G_GPIO_PIN					GPIO_Pin_7
#define LED_G_GPIO_PORT					GPIOB
#define LED_G_GPIO_CLK					RCC_APB2Periph_GPIOB

#define LED_B_GPIO_PIN					GPIO_Pin_6
#define LED_B_GPIO_PORT					GPIOB
#define LED_B_GPIO_CLK					RCC_APB2Periph_GPIOB

#define LED_BOARD_GPIO_PIN			GPIO_Pin_15
#define LED_BOARD_GPIO_PORT			GPIOC
#define LED_BOARD_GPIO_CLK			RCC_APB2Periph_GPIOC


#define ON 											1
#define OFF											0

#define	LED_BOARD(a)	if(a) \
                        GPIO_SetBits(LED_BOARD_GPIO_PORT, LED_BOARD_GPIO_PIN);\
                      else \
                        GPIO_ResetBits(LED_BOARD_GPIO_PORT, LED_BOARD_GPIO_PIN);

#define	LED_R(a)	   if(a) \
                        GPIO_SetBits(LED_R_GPIO_PORT, LED_R_GPIO_PIN);\
                      else \
                        GPIO_ResetBits(LED_R_GPIO_PORT, LED_R_GPIO_PIN);
              
#define	LED_G(a)	   if(a) \
                        GPIO_SetBits(LED_G_GPIO_PORT, LED_G_GPIO_PIN);\
                      else \
                        GPIO_ResetBits(LED_G_GPIO_PORT, LED_G_GPIO_PIN);
                      
#define	LED_B(a)	   if(a) \
                        GPIO_SetBits(LED_B_GPIO_PORT, LED_B_GPIO_PIN);\
                      else \
                        GPIO_ResetBits(LED_B_GPIO_PORT, LED_B_GPIO_PIN);
                      
typedef enum Led_Indication_Enum
{
  RED_ON=0,
  GREEN_ON,
  BLUE_ON,
  ALL_OFF
}Led_Indication;

void LED_GPIO_Config(void);		//≥ı ºªØGPIO
void Indication_LED_Control(Led_Indication led_indication);

#endif  /* _BSP_LED_H */

