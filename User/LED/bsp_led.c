//bsp : board support packer 板级支持包
#include "bsp_led.h"

void LED_GPIO_Config(void)		//初始化GPIO
{
	RCC_APB2PeriphClockCmd(LED_G_GPIO_CLK, ENABLE);
	
//	//设置PB4引脚的重映射，若不设置，PB4引脚默认为JTAG调试，默认上拉，输出不了低电平
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
//	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
//	
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.GPIO_Pin = LED_G_GPIO_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(LED_G_GPIO_PORT, &GPIO_InitStruct);
	GPIO_SetBits(LED_G_GPIO_PORT,LED_G_GPIO_PIN);
}

