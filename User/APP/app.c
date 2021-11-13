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
  * @brief 握手过程灯光指示
  * @param steps：当前握手所在步骤
  * @note 此函数要求10ms调用一次
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
  * @brief 低电流关断
  * @param shutoff_I_samp_vlot：关断电流阈值
  * @param now_I_samp_vlot：当前ASC711采集到的电压值
  * @retval asc711将电流=>电压 
  */
void Low_Current_Shutoff(float shutoff_I_samp_vlot,
                          float now_I_samp_vlot)
{
  static uint8_t low_curent_cnt = 0;
  /* 充电电流低于 0.36A关断 */
  /* ASC711电压电流对应：55mv = 1.0A */
//  if(fabs(shutoff_I_samp_vlot-now_I_samp_vlot)<0.02f)   //电流低于某一特定值
  if((fabs(shutoff_I_samp_vlot-now_I_samp_vlot)/0.055f) < LOW_CURRENT_SHOTOFF_VALUE)   //电流低于某一特定值
  {
    low_curent_cnt++;
    if(low_curent_cnt >= 10)
    {
        slave_flag=0;
        printf("电流过低关断,基准电压=%f 采样电压=%f\r\n",shutoff_I_samp_vlot,now_I_samp_vlot);
        CHAR_DISABLE();       //关闭充电
        RPU_EN_Low();         //关闭增大电流使能
        delay_ms(10);         //延时等待电压跌落，防止烧坏电阻orMOS
        DQ_TX_0();            //拉低总线 复位下位机
        delay_ms(1000);
        DQ_TX_1();            //拉高总线重新给下位机上电
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
  * @brief 过流保护
  * @param over_current：过流保护触发阈值电流 单位A
  * @param shutoff_I_samp_vlot：关断电流阈值
  * @param now_I_samp_vlot：当前ASC711采集到的电压值
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
      printf("充电过流关断,基准电压=%f 采样电压=%f\r\n",shutoff_I_samp_vlot,now_I_samp_vlot);
      CHAR_DISABLE();     //关闭充电
      RPU_EN_Low();       //关闭增大电流使能
      Indication_LED_Control(RED_ON);
      delay_ms(100);
      DQ_TX_1();
      Short_circuit_recovery();
      
      DQ_TX_0();          //拉低总线 复位下位机
      delay_ms(3000);
      DQ_TX_1();          //拉高总线重新给下位机上电
      delay_ms(100);
    }
  }
  else
    over_current_count=0;
}

/**
  * @brief ASC711硬件过流保护
  */
void ASC711_Over_Current_Protect(void)
{
  slave_flag=0;
  CHAR_DISABLE();             //关闭充电
  RPU_EN_Low();               //关闭增大电流使能
  DQ_TX_0();                  //从机断电重启
  delay_ms(100);
  Indication_LED_Control(RED_ON);
  printf("过流保护！！！\r\n");
  
  DQ_TX_1();
  delay_ms(100);
  Short_circuit_recovery();
  
  SLAVE_COMMTASK_TIMER=1000;  //1s后重启从机通讯
  ACS711_PWR_DISABLE();       //重启ASC711电流采集芯片
  delay_ms(5);
  //ACS711_PWR_ENABLE();
  
  DQ_TX_0();                  //从机断电重启
  delay_ms(400);
  DQ_TX_1();
  delay_ms(200);
}

/**
  * @brief 短路恢复
  */
void Short_circuit_recovery(void)
{
  do
  {
    delay_ms(200);
    Channel_Voltage_Calculate();
    LED_BOARD(ON);
    Indication_LED_Control(RED_ON);
    printf("电极片短路，等待短路解除，电极片电压=%f \r\n",(CHAR_OUTV_SAMP*23));
  }while((CHAR_OUTV_SAMP*23)<3.0f);   //等待端口短路恢复
}

/**
  * @brief 过温保护
  */
void Over_Temperature_Protect(void)
{
  if(NTC_SAMP<0.35f)            //过热保护
  {
    slave_flag=0;
    CHAR_DISABLE();             //关闭充电
    RPU_EN_Low();               //关闭增大电流使能
    delay_ms(100);
    DQ_TX_0();                  //从机断电重启
    delay_ms(200);
    while(NTC_SAMP<0.65f)        //等待降温
    {
      printf("过热保护！！！NTC电压=%f\r\n",NTC_SAMP);
      Channel_Voltage_Calculate();
      LED_BOARD(ON);
      Indication_LED_Control(RED_ON);
      delay_ms(100);
    }
    Slave_Reset();
//    NVIC_SystemReset();         //系统复位
  }
}

/**
  * @brief 欠压、过压保护
  */
void Over_Volt_Under_Volt_Protect(void)
{
  if((CV_SAMP*23)<ADAPTER_LOW_LIMIT || (CV_SAMP*23)>ADAPTER_UP_LIMIT)  //适配器电源错误
  {
    slave_flag=0;
    CHAR_DISABLE();               //关闭充电
    RPU_EN_Low();                 //关闭增大电流使能
    delay_ms(100);
    DQ_TX_0();                    //从机断电重启
    delay_ms(200);
    while((CV_SAMP*23)<ADAPTER_LOW_LIMIT || (CV_SAMP*23)>ADAPTER_UP_LIMIT)   //红绿灯交替快闪
    {
      printf("适配器电源错误!!! 适配器电源电压=%f \r\n",(CV_SAMP*23));
      Channel_Voltage_Calculate();
      LED_BOARD(ON);
      Indication_LED_Control(RED_ON);
      delay_ms(200);
    }
    Slave_Reset();
//    NVIC_SystemReset();         //系统复位
  }
}

/**
  * @brief 输出短路保护
  */
void Short_circuit_Output(void)
{
  static uint8_t short_count=0;
  if((CHAR_OUTV_SAMP*23)<3.0f && (slave_flag==0 || slave_flag>=6))    //电极片输出异常
  {
    if(slave_flag!=7)
      short_count++;
    else
      short_count=10;
    if(short_count>=10)
    {
      CHAR_DISABLE();             //关闭充电
      RPU_EN_Low();               //关闭增大电流使能
      DQ_TX_0();                  //从机断电重启
      delay_ms(100);
      LED_BOARD(ON);
      Indication_LED_Control(RED_ON);
      
      DQ_TX_1();
      delay_ms(100);
      printf("输出短路保护！slave_flag=%d\r\n",slave_flag);
      Short_circuit_recovery();
      Slave_Reset();
    }
  }
  else
    short_count=0;
}

/**
  * @brief 从机复位
  */
void Slave_Reset(void)
{
  DQ_TX_0();
  delay_ms(100);
  DQ_TX_1();
}
