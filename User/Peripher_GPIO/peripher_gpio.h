#ifndef __PERIPHER_GPIO_H_
#define __PERIPHER_GPIO_H_

#include "stm32f10x.h"

#define CHAR_EN_GPIO_CLK          RCC_APB2Periph_GPIOA
#define CHAR_EN_GPIO_PIN          GPIO_Pin_12
#define CHAR_EN_GPIO_PORT         GPIOA
#define CHAR_ENABLE()   	        GPIO_SetBits(CHAR_EN_GPIO_PORT,CHAR_EN_GPIO_PIN);
#define CHAR_DISABLE()   	        GPIO_ResetBits(CHAR_EN_GPIO_PORT,CHAR_EN_GPIO_PIN);

#define ACS711_PWR_EN_GPIO_CLK    RCC_APB2Periph_GPIOB
#define ACS711_PWR_EN_GPIO_PIN    GPIO_Pin_12
#define ACS711_PWR_EN_GPIO_PORT   GPIOB
#define ACS711_PWR_ENABLE()   	  GPIO_ResetBits(ACS711_PWR_EN_GPIO_PORT,ACS711_PWR_EN_GPIO_PIN);
#define ACS711_PWR_DISABLE()   	  GPIO_SetBits(ACS711_PWR_EN_GPIO_PORT,ACS711_PWR_EN_GPIO_PIN);

#define CHAR_FO_GPIO_CLK          RCC_APB2Periph_GPIOA
#define CHAR_FO_GPIO_PIN          GPIO_Pin_15
#define CHAR_FO_GPIO_PORT         GPIOA
#define CHAR_FO_READ()        	  GPIO_ReadInputDataBit(CHAR_FO_GPIO_PORT,CHAR_FO_GPIO_PIN)


#define DQ_TX_GPIO_CLK      RCC_APB2Periph_GPIOA
#define DQ_TX_GPIO_PIN      GPIO_Pin_11
#define DQ_TX_GPIO_PORT     GPIOA

#define DQ_TX_High()   	    GPIO_SetBits(DQ_TX_GPIO_PORT,DQ_TX_GPIO_PIN);
#define DQ_TX_Low()   	    GPIO_ResetBits(DQ_TX_GPIO_PORT,DQ_TX_GPIO_PIN);


#define DQ_RX_GPIO_CLK      RCC_APB2Periph_GPIOA
#define DQ_RX_GPIO_PIN      GPIO_Pin_8
#define DQ_RX_GPIO_PORT     GPIOA

#define DQ_RX_Read()   	    GPIO_ReadInputDataBit(DQ_RX_GPIO_PORT,DQ_RX_GPIO_PIN)

#define RPU_EN_GPIO_CLK      RCC_APB2Periph_GPIOB
#define RPU_EN_GPIO_PIN      GPIO_Pin_13
#define RPU_EN_GPIO_PORT     GPIOB

#define RPU_EN_High()   	   GPIO_SetBits(RPU_EN_GPIO_PORT,RPU_EN_GPIO_PIN);
#define RPU_EN_Low()   	     GPIO_ResetBits(RPU_EN_GPIO_PORT,RPU_EN_GPIO_PIN);

void CHIP_ENABLE_GPIO_Config(void);
void DQ_TX_RX_GPIO_Config(void);

#endif /*__PERIPHER_GPIO_H_*/


