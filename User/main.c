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

//��������汾
const uint8_t soft_version[3]={1,5,1};

float charge_samp_I_ref=0;          //������������׼��ѹ

int main(void)
{
  uint8_t led_board_count=0;
  
  HSI_SetSystemCLK(RCC_PLLMul_9);   //����ʱ��Ϊ36MHz
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
  /*����������������׼��ѹ*/
  charge_samp_I_ref=Get_Charging_Current_Samp_Refer_Volt();
  ACS711_PWR_DISABLE();  
    
  LED_BOARD(OFF);
  Indication_LED_Control(BLUE_ON);
	while(1) 
	{
    /* �ӻ�ͨѶ����  */
    if(SLAVE_COMMTASK_TIMER==0)
    {
      SLAVE_COMMTASK_TIMER=10;
      Slave_Communication_Task();
      Handshake_Light_Indication(slave_flag);
    }
    
    /* ADCת���������� */
    if(ADC_CONVERSION_TIMER==0)
    {
      ADC_CONVERSION_TIMER=10;
      Channel_Voltage_Calculate();
    }

    /* ��������� */
    if(CHARGE_CHECK_TIMER==0)
    {
      CHARGE_CHECK_TIMER=10;
      /* �������� */
      if((slave_flag==8)&&(LOW_CURRENT_SAMP_TIMER==0))  //���ģʽ�ҳ��һ��ʱ������͵������
      {
        /* �͵����ض� */
        Low_Current_Shutoff(charge_samp_I_ref,CI_SAMP);
        if(CHAR_FO_READ()==0)         //Ӳ����������
        {
            ASC711_Over_Current_Protect();
        }
        //printf("%d\n", get_device_id());
        /* �����⵽�豸id�ǻ��0x71�������������8A�����������豸�����������12A���� */
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
        //��ʼ������ʱ��ACS711�����ⴥ��Ӳ����������
          ACS711_PWR_ENABLE();    
      }
 
      /* ���±��� */
      Over_Temperature_Protect();
      /* ��ѹ��Ƿѹ���� */
      Over_Volt_Under_Volt_Protect();
      /* �����·���� */
      Short_circuit_Output();
    }
    
    /* ���ڵ������� */
    if(USART_DEBUG_TIMER==0)
    {
      USART_DEBUG_TIMER=1000;
      if(slave_flag!=0)
        printf("��������=%5fA ",(Volt_Channel[0]-1.65)/0.055);
      printf("NTC��ѹ=%5fV ��������ѹ����=%5fV ����缫Ƭ����=%5fV\r\n",
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



/* ����ϵͳʹ�ø��ٵ��ڲ�ʱ��HSI
 * �������ΪPLL��Ƶϵ�� �ο�@arg PLL_multiplication_factor
 */
void HSI_SetSystemCLK(uint32_t RCC_PLLMul_x)
{
	__IO uint32_t HSIStatus = 0;                              /* ���� */
	
	RCC_DeInit();                                             /* ��λϵͳʱ�� */
	
	RCC_LSICmd(ENABLE);                                       /* ʹ��HSI */
	HSIStatus = RCC->CR & RCC_CR_HSIRDY;		                  /* �ȴ�HSI���� */
	while(HSIStatus != RCC_CR_HSIRDY)
	{
		RCC_LSICmd(ENABLE);
		HSIStatus = RCC->CR & RCC_CR_HSIRDY;		                /* ʹ��HSI */
	}
	
	if(HSIStatus == RCC_CR_HSIRDY)          	                /* ʹ��HSI */
	{
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);	  /* ʹ��flash */
		FLASH_SetLatency(FLASH_Latency_2);	                    /* flash�ȴ� */
		
		RCC_HCLKConfig(RCC_SYSCLK_Div1);//AHBCLK=SYSCLK          /* ����AHBʱ��  */
		RCC_PCLK2Config(RCC_HCLK_Div1);	//APB2CLK=HCLK         /* ����APB2ʱ�� 36M */
		RCC_PCLK1Config(RCC_HCLK_Div1); //APB1CLK=HCLK             /* ����APB1ʱ�� 36M */

		
		RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_x);	  /* �������໷ʱ�� PLL=4*x MHz */
		RCC_PLLCmd(ENABLE);                                   	/* ʹ�����໷ʱ�� */
		
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);     /* �ȴ����໷ʱ�����óɹ� */
		
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);	            /* ѡ�����໷ʱ��Ϊϵͳʱ�� */
		while(RCC_GetSYSCLKSource() != 0x08);	                  /* �ȴ����໷ʱ���л�Ϊϵͳʱ�� */
	}
	else
	{
		/* ʱ�����ô���֮���������� */
	}
}

/* ʱ����� */
void MCO_GPIO_Config(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_MCO_InitStrut;
	GPIO_MCO_InitStrut.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_MCO_InitStrut.GPIO_Pin = GPIO_Pin_8;
	GPIO_MCO_InitStrut.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOA, &GPIO_MCO_InitStrut);
}
