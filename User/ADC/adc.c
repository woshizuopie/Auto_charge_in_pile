#include "./ADC/adc.h"
#include "./SYSTICK/systick.h"

//存放4个adc通道的数据
static uint16_t ADC_DataBuf[10][4]={0};
//存放各通道电压值
float Volt_Channel[4]={0};

//ADCx引脚配置
static void ADCx_GPIO_Config()
{
	RCC_APB2PeriphClockCmd(ADCx_GPIO_CLK,ENABLE);
  
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AIN;
	GPIO_InitStruct.GPIO_Pin=ADCx_Channel_1_PIN;
	GPIO_Init(ADCx_Channel_1_PORT,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AIN;
	GPIO_InitStruct.GPIO_Pin=ADCx_Channel_2_PIN;
	GPIO_Init(ADCx_Channel_2_PORT,&GPIO_InitStruct);
 	
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AIN;
	GPIO_InitStruct.GPIO_Pin=ADCx_Channel_3_PIN;
	GPIO_Init(ADCx_Channel_3_PORT,&GPIO_InitStruct);
 	
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AIN;
	GPIO_InitStruct.GPIO_Pin=ADCx_Channel_4_PIN;
	GPIO_Init(ADCx_Channel_4_PORT,&GPIO_InitStruct);
}

//ADC的DMA配置
static void ADCx_DMA_Config(void)
{
	DMA_InitTypeDef DMA_InitStruct;
	
	RCC_AHBPeriphClockCmd(ADCx_DMA_CLk,ENABLE);
	
	DMA_InitStruct.DMA_BufferSize=40;																//DMA传输数据数目
	DMA_InitStruct.DMA_DIR=DMA_DIR_PeripheralSRC;										//DMA方向 外设是源
	DMA_InitStruct.DMA_M2M=DMA_M2M_Disable;													//关闭内存到内存
	DMA_InitStruct.DMA_MemoryBaseAddr=(uint32_t)ADC_DataBuf;				//内存数据地址
	DMA_InitStruct.DMA_MemoryDataSize=DMA_MemoryDataSize_HalfWord;	//内存数据大小 半字
	DMA_InitStruct.DMA_MemoryInc=DMA_MemoryInc_Enable;							//内存地址自加
	DMA_InitStruct.DMA_Mode=DMA_Mode_Circular;											//DMA模式是循环模式
	DMA_InitStruct.DMA_PeripheralBaseAddr=(uint32_t)&(ADC_x->DR);		//外设地址
	DMA_InitStruct.DMA_PeripheralDataSize=DMA_PeripheralDataSize_HalfWord;	//外设数据大小 半字
	DMA_InitStruct.DMA_PeripheralInc=DMA_PeripheralInc_Disable;			//外设地址不自加
	DMA_InitStruct.DMA_Priority=DMA_Priority_High;									//DMA优先级
	DMA_Init(ADC_DMA_CHANNEL, &DMA_InitStruct);
	//使能DMA通道
	DMA_Cmd(ADC_DMA_CHANNEL,ENABLE);
}


//ADC模式初始化
void ADCx_Mode_Init(void)
{
	ADCx_GPIO_Config();	//配置ADC的引脚
	RCC_APB2PeriphClockCmd(ADCx_CLK,ENABLE);
	
	ADC_InitTypeDef ADC_InitStruct;
	ADC_InitStruct.ADC_ContinuousConvMode=ENABLE;			//使能连续转换
	ADC_InitStruct.ADC_DataAlign=ADC_DataAlign_Right;	//数据右对齐
	ADC_InitStruct.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None;	//软件触发
	ADC_InitStruct.ADC_Mode=ADC_Mode_Independent;			//独立模式
	ADC_InitStruct.ADC_NbrOfChannel=4;								//ADC通道数
	ADC_InitStruct.ADC_ScanConvMode=ENABLE;						//打开ADC扫描模式
	ADC_Init(ADC_x,&ADC_InitStruct);
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div4);		//设置ADC时钟为 ADCCLK=PCLK2/4=36M/4=9M
	
	ADC_RegularChannelConfig(ADC_x,ADCx_Channel_1,1,ADC_SampleTime_71Cycles5);	//设置ADC规则通道的顺序和采样时间
	ADC_RegularChannelConfig(ADC_x,ADCx_Channel_2,2,ADC_SampleTime_71Cycles5);	//设置ADC规则通道的顺序和采样时间
	ADC_RegularChannelConfig(ADC_x,ADCx_Channel_3,3,ADC_SampleTime_71Cycles5);	//设置ADC规则通道的顺序和采样时间
  ADC_RegularChannelConfig(ADC_x,ADCx_Channel_4,4,ADC_SampleTime_71Cycles5);	//设置ADC规则通道的顺序和采样时间
  
	ADCx_DMA_Config();					  //配置ADC的DMA功能
	ADC_DMACmd(ADC_x,ENABLE);		  //使能DMA
	
	ADC_Cmd(ADC_x,ENABLE);			  //使能AD转换
	
	ADC_ResetCalibration(ADC_x);	//复位校准
	while(ADC_GetResetCalibrationStatus(ADC_x)==SET);
	
	ADC_StartCalibration(ADC_x);	//A/D校准
	while(ADC_GetCalibrationStatus(ADC_x)==SET);
	
	ADC_SoftwareStartConvCmd(ADC_x,ENABLE);	//软件转化开始
}

/*计算各个通道电压*/
void Channel_Voltage_Calculate(void)
{
	uint8_t i=0,j=0;
	float volt[4]={0};
	uint16_t volt_max[4]={0};  //记录采集到的电压的最大值和最小值
	uint16_t volt_min[4]={0};
  
  volt_max[0]=ADC_DataBuf[0][0]; volt_min[0]=ADC_DataBuf[0][0];
  volt_max[1]=ADC_DataBuf[0][1]; volt_min[1]=ADC_DataBuf[0][1];
  volt_max[2]=ADC_DataBuf[0][2]; volt_min[2]=ADC_DataBuf[0][2];
  volt_max[3]=ADC_DataBuf[0][3]; volt_min[3]=ADC_DataBuf[0][3];
	
	for(i=0;i<10;i++)
	{
    for(j=0;j<4;j++)
    {
      volt[j]+=ADC_DataBuf[i][j];
      if(volt_max[j]<ADC_DataBuf[i][j])	volt_max[j]=ADC_DataBuf[i][j];//获取最大值
      if(volt_min[j]>ADC_DataBuf[i][j])	volt_min[j]=ADC_DataBuf[i][j];//获取最小值
    }
	}
  
  for(i=0;i<4;i++)  	//滤除最大值最小值 取平均值
    volt[i]=(volt[i]-volt_max[i]-volt_min[i])/8;
  
  for(i=0;i<4;i++)    //计算各通道电压值 并进行相应的转化
    Volt_Channel[i]=volt[i]*3.3/4096;
}

/**
  * @brief 获取充电电流采样基准
  * @note 需要在充电未开始时调用，否则获取到的为错误值
  * @retval 对应的充电电流采样基准值
  */
float Get_Charging_Current_Samp_Refer_Volt(void)
{
  float charge_samp_I_ref=0;
  delay_ms(100);
  Channel_Voltage_Calculate();
  charge_samp_I_ref=CI_SAMP;
  
  return charge_samp_I_ref;
}

