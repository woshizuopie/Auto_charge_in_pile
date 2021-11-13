#include "./APP/app.h"
#include "led.h"
#include "./Slave/slave.h"
#include "./Peripher_GPIO/peripher_gpio.h"
#include "./Oneline/oneline.h"
#include "./SYSTICK/systick.h"
#include "./ADC/adc.h"
#include <math.h>
#include <stdio.h>

#define   LOW_CURRENT_SHOTOFF_VALUE       (0.28f)

/**
  * @brief ���ֹ��̵ƹ�ָʾ
  * @param steps����ǰ�������ڲ���
  * @note �˺���Ҫ��10ms����һ��
  * @retval NULL
  */
void Handshake_Light_Indication(uint8_t steps)
{
  static uint8_t led_count=0;
  switch(steps)
  {
    case 0:
      Indication_LED_Control(BLUE_ON);
      break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
      led_count++;
      if(led_count>50)
        led_count=0;
      else if(led_count>25)
        Indication_LED_Control(ALL_OFF);
      else
        Indication_LED_Control(GREEN_ON);
      break;
    case 7:
      Indication_LED_Control(GREEN_ON);
      break;
    default: break;
  }
}

/**
  * @brief �͵����ض�
  * @param shutoff_I_samp_vlot���ضϵ�����ֵ
  * @param now_I_samp_vlot����ǰASC711�ɼ����ĵ�ѹֵ
  * @retval asc711������=>��ѹ 
  */
void Low_Current_Shutoff(float shutoff_I_samp_vlot,
                          float now_I_samp_vlot)
{
  static uint8_t low_curent_cnt = 0;
  /* ���������� 0.36A�ض� */
  /* ASC711��ѹ������Ӧ��55mv = 1.0A */
//  if(fabs(shutoff_I_samp_vlot-now_I_samp_vlot)<0.02f)   //��������ĳһ�ض�ֵ
  if((fabs(shutoff_I_samp_vlot-now_I_samp_vlot)/0.055f) < LOW_CURRENT_SHOTOFF_VALUE)   //��������ĳһ�ض�ֵ
  {
    low_curent_cnt++;
    if(low_curent_cnt >= 10)
    {
        slave_flag=0;
        printf("�������͹ض�,��׼��ѹ=%f ������ѹ=%f\r\n",shutoff_I_samp_vlot,now_I_samp_vlot);
        CHAR_DISABLE();       //�رճ��
        RPU_EN_Low();         //�ر��������ʹ��
        delay_ms(10);         //��ʱ�ȴ���ѹ���䣬��ֹ�ջ�����orMOS
        DQ_TX_0();            //�������� ��λ��λ��
        delay_ms(1000);
        DQ_TX_1();            //�����������¸���λ���ϵ�
        delay_ms(100); 
        low_curent_cnt = 0;      
        ACS711_PWR_DISABLE();
    }
  }
  else 
  {
    low_curent_cnt = 0;   
  }
  
}

/**
  * @brief ��������
  * @param over_current����������������ֵ���� ��λA
  * @param shutoff_I_samp_vlot���ضϵ�����ֵ
  * @param now_I_samp_vlot����ǰASC711�ɼ����ĵ�ѹֵ
  * @retval NULL 
  */
uint8_t get_device_id(void);

void Over_Current_Protect(float over_current,
                          float shutoff_I_samp_vlot,
                          float now_I_samp_vlot)
{
  static uint32_t over_current_count=0;

  if((fabs(shutoff_I_samp_vlot-now_I_samp_vlot))/0.055f > over_current)
  {  
    over_current_count++;
    if(over_current_count >= 100)
    {
      slave_flag=0;
      printf("�������ض�,��׼��ѹ=%f ������ѹ=%f\r\n",shutoff_I_samp_vlot,now_I_samp_vlot);
      CHAR_DISABLE();     //�رճ��
      RPU_EN_Low();       //�ر��������ʹ��
      Indication_LED_Control(RED_ON);
      delay_ms(100);
      DQ_TX_1();
      Short_circuit_recovery();
      
      DQ_TX_0();          //�������� ��λ��λ��
      delay_ms(3000);
      DQ_TX_1();          //�����������¸���λ���ϵ�
      delay_ms(100);
    }
  }
  else
    over_current_count=0;
}

/**
  * @brief ASC711Ӳ����������
  */
void ASC711_Over_Current_Protect(void)
{
  slave_flag=0;
  CHAR_DISABLE();             //�رճ��
  RPU_EN_Low();               //�ر��������ʹ��
  DQ_TX_0();                  //�ӻ��ϵ�����
  delay_ms(100);
  Indication_LED_Control(RED_ON);
  printf("��������������\r\n");
  
  DQ_TX_1();
  delay_ms(100);
  Short_circuit_recovery();
  
  SLAVE_COMMTASK_TIMER=1000;  //1s�������ӻ�ͨѶ
  ACS711_PWR_DISABLE();       //����ASC711�����ɼ�оƬ
  delay_ms(5);
  //ACS711_PWR_ENABLE();
  
  DQ_TX_0();                  //�ӻ��ϵ�����
  delay_ms(400);
  DQ_TX_1();
  delay_ms(200);
}

/**
  * @brief ��·�ָ�
  */
void Short_circuit_recovery(void)
{
  do
  {
    delay_ms(200);
    Channel_Voltage_Calculate();
    LED_BOARD(ON);
    Indication_LED_Control(RED_ON);
    printf("�缫Ƭ��·���ȴ���·������缫Ƭ��ѹ=%f \r\n",(CHAR_OUTV_SAMP*23));
  }while((CHAR_OUTV_SAMP*23)<3.0f);   //�ȴ��˿ڶ�·�ָ�
}

/**
  * @brief ���±���
  */
void Over_Temperature_Protect(void)
{
  if(NTC_SAMP<0.35f)            //���ȱ���
  {
    slave_flag=0;
    CHAR_DISABLE();             //�رճ��
    RPU_EN_Low();               //�ر��������ʹ��
    delay_ms(100);
    DQ_TX_0();                  //�ӻ��ϵ�����
    delay_ms(200);
    while(NTC_SAMP<0.65f)        //�ȴ�����
    {
      printf("���ȱ���������NTC��ѹ=%f\r\n",NTC_SAMP);
      Channel_Voltage_Calculate();
      LED_BOARD(ON);
      Indication_LED_Control(RED_ON);
      delay_ms(100);
    }
    Slave_Reset();
//    NVIC_SystemReset();         //ϵͳ��λ
  }
}

/**
  * @brief Ƿѹ����ѹ����
  */
void Over_Volt_Under_Volt_Protect(void)
{
  if((CV_SAMP*23)<ADAPTER_LOW_LIMIT || (CV_SAMP*23)>ADAPTER_UP_LIMIT)  //��������Դ����
  {
    slave_flag=0;
    CHAR_DISABLE();               //�رճ��
    RPU_EN_Low();                 //�ر��������ʹ��
    delay_ms(100);
    DQ_TX_0();                    //�ӻ��ϵ�����
    delay_ms(200);
    while((CV_SAMP*23)<ADAPTER_LOW_LIMIT || (CV_SAMP*23)>ADAPTER_UP_LIMIT)   //���̵ƽ������
    {
      printf("��������Դ����!!! ��������Դ��ѹ=%f \r\n",(CV_SAMP*23));
      Channel_Voltage_Calculate();
      LED_BOARD(ON);
      Indication_LED_Control(RED_ON);
      delay_ms(200);
    }
    Slave_Reset();
//    NVIC_SystemReset();         //ϵͳ��λ
  }
}

/**
  * @brief �����·����
  */
void Short_circuit_Output(void)
{
  static uint8_t short_count=0;
  if((CHAR_OUTV_SAMP*23)<3.0f && (slave_flag==0 || slave_flag>=6))    //�缫Ƭ����쳣
  {
    if(slave_flag!=7)
      short_count++;
    else
      short_count=10;
    if(short_count>=10)
    {
      CHAR_DISABLE();             //�رճ��
      RPU_EN_Low();               //�ر��������ʹ��
      DQ_TX_0();                  //�ӻ��ϵ�����
      delay_ms(100);
      LED_BOARD(ON);
      Indication_LED_Control(RED_ON);
      
      DQ_TX_1();
      delay_ms(100);
      printf("�����·������slave_flag=%d\r\n",slave_flag);
      Short_circuit_recovery();
      Slave_Reset();
    }
  }
  else
    short_count=0;
}

/**
  * @brief �ӻ���λ
  */
void Slave_Reset(void)
{
  DQ_TX_0();
  delay_ms(100);
  DQ_TX_1();
}
