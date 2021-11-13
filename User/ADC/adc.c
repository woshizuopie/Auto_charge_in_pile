#include "./ADC/adc.h"
#include "./SYSTICK/systick.h"

//���4��adcͨ��������
static uint16_t ADC_DataBuf[10][4]={0};
//��Ÿ�ͨ����ѹֵ
float Volt_Channel[4]={0};

//ADCx��������
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

//ADC��DMA����
static void ADCx_DMA_Config(void)
{
	DMA_InitTypeDef DMA_InitStruct;
	
	RCC_AHBPeriphClockCmd(ADCx_DMA_CLk,ENABLE);
	
	DMA_InitStruct.DMA_BufferSize=40;																//DMA����������Ŀ
	DMA_InitStruct.DMA_DIR=DMA_DIR_PeripheralSRC;										//DMA���� ������Դ
	DMA_InitStruct.DMA_M2M=DMA_M2M_Disable;													//�ر��ڴ浽�ڴ�
	DMA_InitStruct.DMA_MemoryBaseAddr=(uint32_t)ADC_DataBuf;				//�ڴ����ݵ�ַ
	DMA_InitStruct.DMA_MemoryDataSize=DMA_MemoryDataSize_HalfWord;	//�ڴ����ݴ�С ����
	DMA_InitStruct.DMA_MemoryInc=DMA_MemoryInc_Enable;							//�ڴ��ַ�Լ�
	DMA_InitStruct.DMA_Mode=DMA_Mode_Circular;											//DMAģʽ��ѭ��ģʽ
	DMA_InitStruct.DMA_PeripheralBaseAddr=(uint32_t)&(ADC_x->DR);		//�����ַ
	DMA_InitStruct.DMA_PeripheralDataSize=DMA_PeripheralDataSize_HalfWord;	//�������ݴ�С ����
	DMA_InitStruct.DMA_PeripheralInc=DMA_PeripheralInc_Disable;			//�����ַ���Լ�
	DMA_InitStruct.DMA_Priority=DMA_Priority_High;									//DMA���ȼ�
	DMA_Init(ADC_DMA_CHANNEL, &DMA_InitStruct);
	//ʹ��DMAͨ��
	DMA_Cmd(ADC_DMA_CHANNEL,ENABLE);
}


//ADCģʽ��ʼ��
void ADCx_Mode_Init(void)
{
	ADCx_GPIO_Config();	//����ADC������
	RCC_APB2PeriphClockCmd(ADCx_CLK,ENABLE);
	
	ADC_InitTypeDef ADC_InitStruct;
	ADC_InitStruct.ADC_ContinuousConvMode=ENABLE;			//ʹ������ת��
	ADC_InitStruct.ADC_DataAlign=ADC_DataAlign_Right;	//�����Ҷ���
	ADC_InitStruct.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None;	//�������
	ADC_InitStruct.ADC_Mode=ADC_Mode_Independent;			//����ģʽ
	ADC_InitStruct.ADC_NbrOfChannel=4;								//ADCͨ����
	ADC_InitStruct.ADC_ScanConvMode=ENABLE;						//��ADCɨ��ģʽ
	ADC_Init(ADC_x,&ADC_InitStruct);
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div4);		//����ADCʱ��Ϊ ADCCLK=PCLK2/4=36M/4=9M
	
	ADC_RegularChannelConfig(ADC_x,ADCx_Channel_1,1,ADC_SampleTime_71Cycles5);	//����ADC����ͨ����˳��Ͳ���ʱ��
	ADC_RegularChannelConfig(ADC_x,ADCx_Channel_2,2,ADC_SampleTime_71Cycles5);	//����ADC����ͨ����˳��Ͳ���ʱ��
	ADC_RegularChannelConfig(ADC_x,ADCx_Channel_3,3,ADC_SampleTime_71Cycles5);	//����ADC����ͨ����˳��Ͳ���ʱ��
  ADC_RegularChannelConfig(ADC_x,ADCx_Channel_4,4,ADC_SampleTime_71Cycles5);	//����ADC����ͨ����˳��Ͳ���ʱ��
  
	ADCx_DMA_Config();					  //����ADC��DMA����
	ADC_DMACmd(ADC_x,ENABLE);		  //ʹ��DMA
	
	ADC_Cmd(ADC_x,ENABLE);			  //ʹ��ADת��
	
	ADC_ResetCalibration(ADC_x);	//��λУ׼
	while(ADC_GetResetCalibrationStatus(ADC_x)==SET);
	
	ADC_StartCalibration(ADC_x);	//A/DУ׼
	while(ADC_GetCalibrationStatus(ADC_x)==SET);
	
	ADC_SoftwareStartConvCmd(ADC_x,ENABLE);	//���ת����ʼ
}

/*�������ͨ����ѹ*/
void Channel_Voltage_Calculate(void)
{
	uint8_t i=0,j=0;
	float volt[4]={0};
	uint16_t volt_max[4]={0};  //��¼�ɼ����ĵ�ѹ�����ֵ����Сֵ
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
      if(volt_max[j]<ADC_DataBuf[i][j])	volt_max[j]=ADC_DataBuf[i][j];//��ȡ���ֵ
      if(volt_min[j]>ADC_DataBuf[i][j])	volt_min[j]=ADC_DataBuf[i][j];//��ȡ��Сֵ
    }
	}
  
  for(i=0;i<4;i++)  	//�˳����ֵ��Сֵ ȡƽ��ֵ
    volt[i]=(volt[i]-volt_max[i]-volt_min[i])/8;
  
  for(i=0;i<4;i++)    //�����ͨ����ѹֵ ��������Ӧ��ת��
    Volt_Channel[i]=volt[i]*3.3/4096;
}

/**
  * @brief ��ȡ������������׼
  * @note ��Ҫ�ڳ��δ��ʼʱ���ã������ȡ����Ϊ����ֵ
  * @retval ��Ӧ�ĳ�����������׼ֵ
  */
float Get_Charging_Current_Samp_Refer_Volt(void)
{
  float charge_samp_I_ref=0;
  delay_ms(100);
  Channel_Voltage_Calculate();
  charge_samp_I_ref=CI_SAMP;
  
  return charge_samp_I_ref;
}

