#include "stm32f10x.h"
#include "./USART/usart.h"
#include "./SYSTICK/systick.h"
#include "led.h"
#include "./KEY/key.h"
#include "./Peripher_GPIO/peripher_gpio.h"
#include "./TIM/tim.h"
#include "./Oneline/oneline.h"
#include "./Slave/slave.h"
#include "./ADC/adc.h"
#include "math.h"
#include "./APP/app.h"

void MCO_GPIO_Config(void);
void HSI_SetSystemCLK(uint32_t RCC_PLLMul_x);
uint8_t get_device_id(void);

//定义软件版本
const uint8_t soft_version[3]={1,5,1};

float charge_samp_I_ref=0;          //充电电流采样基准电压

int main(void)
{
  uint8_t led_board_count=0;
  
  HSI_SetSystemCLK(RCC_PLLMul_9);   //配置时钟为36MHz
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  Systick_Init();
  USART1_Config();
    
  printf("ChargePile Version : V%d.%d.%d\r\n",soft_version[0],soft_version[1],soft_version[2]);
    
  LED_GPIO_Config();
  CHIP_ENABLE_GPIO_Config();
  DQ_TX_RX_GPIO_Config();
  TIM4_Init();
  ADCx_Mode_Init();
  ACS711_PWR_ENABLE();
  /*测量充电电流采样基准电压*/
  charge_samp_I_ref=Get_Charging_Current_Samp_Refer_Volt();
  ACS711_PWR_DISABLE();  
    
  LED_BOARD(OFF);
  Indication_LED_Control(BLUE_ON);
	while(1) 
	{
    /* 从机通讯任务  */
    if(SLAVE_COMMTASK_TIMER==0)
    {
      SLAVE_COMMTASK_TIMER=10;
      Slave_Communication_Task();
      Handshake_Light_Indication(slave_flag);
    }
    
    /* ADC转化计算任务 */
    if(ADC_CONVERSION_TIMER==0)
    {
      ADC_CONVERSION_TIMER=10;
      Channel_Voltage_Calculate();
    }

    /* 充电检查任务 */
    if(CHARGE_CHECK_TIMER==0)
    {
      CHARGE_CHECK_TIMER=10;
      /* 过流保护 */
      if((slave_flag==8)&&(LOW_CURRENT_SAMP_TIMER==0))  //充电模式且充电一段时间后开启低电流检测
      {
        /* 低电流关断 */
        Low_Current_Shutoff(charge_samp_I_ref,CI_SAMP);
        if(CHAR_FO_READ()==0)         //硬件过流保护
        {
            ASC711_Over_Current_Protect();
        }
        //printf("%d\n", get_device_id());
        /* 如果检测到设备id是火狐0x71，则充电电流大于8A保护，其他设备则充电电流大于12A保护 */
        if(get_device_id() != 0x71)
        {
            Over_Current_Protect(12.0f,charge_samp_I_ref,CI_SAMP);
        }
        else{
            Over_Current_Protect(8.0f,charge_samp_I_ref,CI_SAMP);
        }
      }
      else if((slave_flag==8)&&(LOW_CURRENT_SAMP_TIMER<=50))
      {
        //开始充电后，延时打开ACS711，避免触发硬件过流保护
          ACS711_PWR_ENABLE();    
      }
 
      /* 过温保护 */
      Over_Temperature_Protect();
      /* 过压、欠压保护 */
      Over_Volt_Under_Volt_Protect();
      /* 输出短路保护 */
      Short_circuit_Output();
    }
    
    /* 串口调试任务 */
    if(USART_DEBUG_TIMER==0)
    {
      USART_DEBUG_TIMER=1000;
      if(slave_flag!=0)
        printf("电流采样=%5fA ",(Volt_Channel[0]-1.65)/0.055);
      printf("NTC电压=%5fV 适配器电压采样=%5fV 输出电极片采样=%5fV\r\n",
            Volt_Channel[1],Volt_Channel[2]*23,Volt_Channel[3]*23);
      
      if(led_board_count>=1)
      {
        led_board_count=0;
        LED_BOARD(ON);
      }
      else
      {
        led_board_count++;
        LED_BOARD(OFF);
      }
    }
	}
}



/* 配置系统使用高速的内部时钟HSI
 * 传入参数为PLL倍频系数 参考@arg PLL_multiplication_factor
 */
void HSI_SetSystemCLK(uint32_t RCC_PLLMul_x)
{
	__IO uint32_t HSIStatus = 0;                              /* 定义 */
	
	RCC_DeInit();                                             /* 复位系统时钟 */
	
	RCC_LSICmd(ENABLE);                                       /* 使能HSI */
	HSIStatus = RCC->CR & RCC_CR_HSIRDY;		                  /* 等待HSI就绪 */
	while(HSIStatus != RCC_CR_HSIRDY)
	{
		RCC_LSICmd(ENABLE);
		HSIStatus = RCC->CR & RCC_CR_HSIRDY;		                /* 使能HSI */
	}
	
	if(HSIStatus == RCC_CR_HSIRDY)          	                /* 使能HSI */
	{
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);	  /* 使能flash */
		FLASH_SetLatency(FLASH_Latency_2);	                    /* flash等待 */
		
		RCC_HCLKConfig(RCC_SYSCLK_Div1);//AHBCLK=SYSCLK          /* 配置AHB时钟  */
		RCC_PCLK2Config(RCC_HCLK_Div1);	//APB2CLK=HCLK         /* 配置APB2时钟 36M */
		RCC_PCLK1Config(RCC_HCLK_Div1); //APB1CLK=HCLK             /* 配置APB1时钟 36M */

		
		RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_x);	  /* 配置锁相环时钟 PLL=4*x MHz */
		RCC_PLLCmd(ENABLE);                                   	/* 使能锁相环时钟 */
		
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);     /* 等待锁相环时钟配置成功 */
		
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);	            /* 选择锁相环时钟为系统时钟 */
		while(RCC_GetSYSCLKSource() != 0x08);	                  /* 等待锁相环时钟切换为系统时钟 */
	}
	else
	{
		/* 时钟配置错误之后在这设置 */
	}
}

/* 时钟输出 */
void MCO_GPIO_Config(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_MCO_InitStrut;
	GPIO_MCO_InitStrut.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_MCO_InitStrut.GPIO_Pin = GPIO_Pin_8;
	GPIO_MCO_InitStrut.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOA, &GPIO_MCO_InitStrut);
}
