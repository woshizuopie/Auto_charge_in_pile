#include "./Peripher_GPIO/peripher_gpio.h"

/**芯片使能管脚GPIO初始化
  */
void CHIP_ENABLE_GPIO_Config(void)
{
	RCC_APB2PeriphClockCmd( CHAR_EN_GPIO_CLK|
                          ACS711_PWR_EN_GPIO_CLK|
                          CHAR_FO_GPIO_CLK|
                          RPU_EN_GPIO_CLK, ENABLE);
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
  
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.GPIO_Pin = CHAR_EN_GPIO_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(CHAR_EN_GPIO_PORT, &GPIO_InitStruct);
	CHAR_DISABLE();
  
  GPIO_InitStruct.GPIO_Pin = ACS711_PWR_EN_GPIO_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(ACS711_PWR_EN_GPIO_PORT, &GPIO_InitStruct);
	ACS711_PWR_DISABLE();
  
  GPIO_InitStruct.GPIO_Pin = CHAR_FO_GPIO_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(CHAR_FO_GPIO_PORT, &GPIO_InitStruct);
  
  GPIO_InitStruct.GPIO_Pin = RPU_EN_GPIO_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(RPU_EN_GPIO_PORT, &GPIO_InitStruct);
  RPU_EN_Low();
}

void DQ_TX_RX_GPIO_Config(void)
{
	RCC_APB2PeriphClockCmd(DQ_TX_GPIO_CLK, ENABLE);
  
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.GPIO_Pin = DQ_TX_GPIO_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DQ_TX_GPIO_PORT, &GPIO_InitStruct);
	DQ_TX_Low();
  
	GPIO_InitStruct.GPIO_Pin = DQ_RX_GPIO_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(DQ_RX_GPIO_PORT, &GPIO_InitStruct);
}
  



