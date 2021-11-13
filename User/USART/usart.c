#include "./USART/usart.h"

 /**
  * @brief  USART GPIO 配置,工作参数配置
  * @param  无
  * @retval 无
  */
void USART1_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	// 打开串口GPIO的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	// 打开串口外设的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	// 将USART Tx的GPIO配置为推挽复用模式 USART1_Tx=PA9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

  // 将USART Rx的GPIO配置为浮空输入模式 USART1_Rx=PA10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// 配置串口的工作参数
	// 配置波特率
	USART_InitStructure.USART_BaudRate = USART1_BAUDRATE;
	// 配置 针数据字长
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	// 配置停止位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	// 配置校验位
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	// 配置硬件流控制
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	// 配置工作模式，收发一起
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	// 完成串口的初始化配置
	USART_Init(USART1, &USART_InitStructure);

	// 使能串口
	USART_Cmd(USART1, ENABLE);
}


///重定向c库函数printf到串口，重定向后可使用printf函数
int fputc(int ch, FILE *f)
{
		/* 发送一个字节数据到串口 */
		USART_SendData(USART1, (uint8_t) ch);
		
		/* 等待发送完毕 */
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);		
	
		return (ch);
}

///重定向c库函数scanf到串口，重写向后可使用scanf、getchar等函数
int fgetc(FILE *f)
{
		/* 等待串口输入数据 */
		while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);

		return (int)USART_ReceiveData(USART1);
}

//USART2的NVIC中断配置
static void NVIC_USART2_Config(void)
{
	NVIC_InitTypeDef USART2_NVIV_InitStruct;
	
	USART2_NVIV_InitStruct.NVIC_IRQChannel=USART2_IRQn;
	USART2_NVIV_InitStruct.NVIC_IRQChannelPreemptionPriority=1;
	USART2_NVIV_InitStruct.NVIC_IRQChannelSubPriority=2;
	USART2_NVIV_InitStruct.NVIC_IRQChannelCmd=ENABLE;
	
	NVIC_Init(&USART2_NVIV_InitStruct);
}

void USART2_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	// 打开串口GPIO的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	// 打开串口外设的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	// 将USART Tx的GPIO配置为推挽复用模式  USART2_Tx=PA2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

  // 将USART Rx的GPIO配置为浮空输入模式  USART2_Rx=PA3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// 配置串口的工作参数
	// 配置波特率
	USART_InitStructure.USART_BaudRate = USART2_BAUDRATE;
	// 配置 针数据字长
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	// 配置停止位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	// 配置校验位
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	// 配置硬件流控制
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	// 配置工作模式，收发一起
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	// 完成串口的初始化配置
	USART_Init(USART2, &USART_InitStructure);

	//NVIC中断配置
	NVIC_USART2_Config();
	//接收中断使能
	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
	// 使能串口
	USART_Cmd(USART2, ENABLE);
}

//串口2接收标志位
//第7位置1：接收完成  第6置1：接收到包头 
//第5位置1：接收到次包头 第0-4位：记录接收到的字节数
uint8_t USART2_RX_STA=0;
//串口2接收数据缓存区
uint8_t USART2_RX_Buf[2]={0};
//USART2中断服务函数
void USART2_IRQHandler(void)
{
	uint8_t data;
	
	if(USART_GetFlagStatus(USART2, USART_FLAG_ORE) != RESET)//溢出中断
	{
		USART_ReceiveData(USART2);		//清除溢出中断 先读SR再读DR清除溢出中断
	}
	
	if(USART_GetITStatus(USART2,USART_IT_RXNE)==SET)	//如果获取到的是接收中断
	{
		data=USART_ReceiveData(USART2);
		if((USART2_RX_STA&0x80)==0)		//未接受完成
		{
			USART2_RX_STA|=0x80;
			USART2_RX_Buf[0]=data;
			//==============================
			//========		待开发   =========
			//==============================
		}
	}
}
