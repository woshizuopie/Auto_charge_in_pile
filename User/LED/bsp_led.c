//bsp : board support packer �弶֧�ְ�
#include "bsp_led.h"

void LED_GPIO_Config(void)		//��ʼ��GPIO
{
	RCC_APB2PeriphClockCmd(LED_G_GPIO_CLK, ENABLE);
	
//	//����PB4���ŵ���ӳ�䣬�������ã�PB4����Ĭ��ΪJTAG���ԣ�Ĭ��������������˵͵�ƽ
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

