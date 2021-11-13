#include "./KEY/key.h"

void KEY_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	KEY_GPIO_APBx_ClkCmd(KEY_GPIO_CLK,ENABLE);
	
	/* KEY1 PB1 */
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin=KEY1_GPIO_PIN;
	GPIO_Init(KEY1_GPIO_PORT,&GPIO_InitStruct);
	
	/* KEY2 PA8 */
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin=KEY2_GPIO_PIN;
	GPIO_Init(KEY2_GPIO_PORT,&GPIO_InitStruct);

}

/* 按键扫描函数
 * mode=1：支持连按 mode=0：不支持连按
 * 注意此函数有响应优先级,KEY0>KEY1>KEY2!!
 */
uint8_t KEY_Scan(uint8_t mode)
{
	static uint8_t key_flag=1;
	if(mode==1) key_flag=1;
	if((key_flag==1)&&((KEY1==0)||(KEY2==0)))
	{
		delay_ms(10);
		key_flag=0;
		if(KEY1==0)				return KEY1_ON;
		else if(KEY2==0)	return KEY2_ON;
	}
	else if((KEY1==1)&&(KEY2==1)) key_flag=1;
	return 0;
}




