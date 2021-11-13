#include "led.h"

void LED_GPIO_Config(void)		//初始化GPIO
{
	RCC_APB2PeriphClockCmd( LED_BOARD_GPIO_CLK|
                          LED_R_GPIO_CLK|
                          LED_G_GPIO_CLK, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.GPIO_Pin = LED_BOARD_GPIO_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LED_BOARD_GPIO_PORT, &GPIO_InitStruct);
  
	GPIO_InitStruct.GPIO_Pin = LED_G_GPIO_PIN;
	GPIO_Init(LED_G_GPIO_PORT, &GPIO_InitStruct);
  
	GPIO_InitStruct.GPIO_Pin = LED_R_GPIO_PIN;
	GPIO_Init(LED_R_GPIO_PORT, &GPIO_InitStruct);
  
	GPIO_InitStruct.GPIO_Pin = LED_B_GPIO_PIN;
	GPIO_Init(LED_B_GPIO_PORT, &GPIO_InitStruct);
  
	LED_BOARD(OFF);
	LED_R(OFF);
	LED_G(OFF);
	LED_B(OFF);
}

/**
  * @brief LED指示控制
  * @param led_indication：led指示状态 @ref Led_Indication
  */
void Indication_LED_Control(Led_Indication led_indication)
{
  switch(led_indication)
  {
    case RED_ON:
      LED_R(ON);
      LED_G(OFF);
      LED_B(OFF);
      break;
    case GREEN_ON:
      LED_R(OFF);
      LED_G(ON);
      LED_B(OFF);
      break;
    case BLUE_ON:
      LED_R(OFF);
      LED_G(OFF);
      LED_B(ON);
      break;
    case ALL_OFF:
      LED_R(OFF);
      LED_G(OFF);
      LED_B(OFF);
    default:
      break;
  }
}

