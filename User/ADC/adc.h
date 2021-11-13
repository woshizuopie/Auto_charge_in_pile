#ifndef __ADC_H
#define __ADC_H

#include "stm32f10x.h"

#define ADCx_GPIO_CLK					RCC_APB2Periph_GPIOA
#define ADCx_DMA_CLk					RCC_AHBPeriph_DMA1

#define ADCx_CLK							RCC_APB2Periph_ADC1
#define ADC_x									ADC1

// ADC1 对应 DMA1 通道 1， ADC3 对应 DMA2 通道 5， ADC2 没有 DMA 功能
#define ADC_DMA_CHANNEL				DMA1_Channel1

#define ADCx_Channel_1_PORT		GPIOA
#define ADCx_Channel_1_PIN		GPIO_Pin_1
#define ADCx_Channel_1				ADC_Channel_1

#define ADCx_Channel_2_PORT		GPIOA
#define ADCx_Channel_2_PIN		GPIO_Pin_4
#define ADCx_Channel_2				ADC_Channel_4

#define ADCx_Channel_3_PORT		GPIOA
#define ADCx_Channel_3_PIN		GPIO_Pin_5
#define ADCx_Channel_3				ADC_Channel_5

#define ADCx_Channel_4_PORT		GPIOA
#define ADCx_Channel_4_PIN		GPIO_Pin_6
#define ADCx_Channel_4				ADC_Channel_6

extern float Volt_Channel[4];

#define CI_SAMP           Volt_Channel[0] //充电电流采样
#define NTC_SAMP          Volt_Channel[1] //温度采样
#define CV_SAMP           Volt_Channel[2] //充电电压采样
#define CHAR_OUTV_SAMP    Volt_Channel[3] //充电接口判断


void ADCx_Mode_Init(void);
void Channel_Voltage_Calculate(void);
float Get_Charging_Current_Samp_Refer_Volt(void);

#endif /* __ADC_H */

