#include "./Slave/slave.h"
#include "./Oneline/oneline.h"
#include "./SYSTICK/systick.h"
#include "led.h"
#include "./USART/usart.h"

static uint8_t device_id=0;

uint8_t get_device_id(void)
{
    return device_id;
}

enum low_current_shuttime_enum {
  time_200ms = 0,
  time_5000ms = 1,
}low_current_shuttime;

//从机状态标志
uint8_t slave_flag=0x00;

/** 从机检测
  * 成功返回0 失败返回1
  */
static uint8_t Slave_Detect(void)
{
  uint8_t status=0;
  status=Oneline_Master_RST();
//  printf("status=%d\r\n",status);
  if(status==0)   //复位成功
  {
    slave_flag=1;
    printf("设备扫描成功\r\n");
    delay_ms(50);        //延时50ms储能
  }
  else
  {
    slave_flag=0;
//    printf("设备扫描失败\r\n");
    delay_ms(30);
  }
  return 1;
}

/* 从机通讯任务 */
void Slave_Communication_Task(void)
{
  static uint32_t slave_wait_count=0;
  static uint8_t error_count=0;
//  static uint8_t device_id=0;
  uint8_t data=0;
  
  switch(slave_flag)
  {
    case 0:        /*扫描设备*/
      device_id = 0;
      CHAR_DISABLE();                 //关闭充电
      ACS711_PWR_DISABLE();
      RPU_EN_Low();                   //关闭增大电流使能
      Slave_Detect();
      break;
    case 1:        /*读取设备ID*/
      data=Oneline_Master_Read_Reg(REG_DEVICE_ID);
      device_id = data;
      printf("REG_DEVICE_ID=%#x\r\n",device_id);
      if((device_id&0xF0)==DEVICE_ID_8A_5S_1 ||
         (device_id&0xF0)==DEVICE_ID_8A_5S_2 )           //设备ID读取成功
      {
        low_current_shuttime = time_5000ms;
        error_count=0;
        slave_flag=3;
      }
      else if(device_id == DEVICE_ID_8A_200MS)
      {
        low_current_shuttime = time_200ms;
        error_count=0;
        slave_flag=2;
      }
      else if(device_id == DEVICE_ID_3_5A_5S)
      {
        low_current_shuttime = time_5000ms;
        error_count=0;
        slave_flag=2;
      }
      else
      {
        error_count++;
        if(error_count>3)
        {
          error_count=0;
          DQ_TX_0();                  //复位从机
          delay_ms(25);
          DQ_TX_1();
          slave_flag=0;
        }
        SLAVE_COMMTASK_TIMER=100;     //延时等待稳定
      }
      break;
    case 2: /* 往从机发送设备ID进行校验，之后读取校验寄存器查看结果 */
      Oneline_Master_Write_Reg(REG_DEVICE_ID, device_id);
      delay_ms(4);
      if(Oneline_Master_Read_Reg(REG_DEVICE_ID_CHECK) == 0x01)
      {
       printf("check success!\r\n");
       slave_flag = 3;
      }
      else
      {
       printf("check fail!\r\n");
       slave_flag = 1;
      }
      break;
    case 3:       /*REG_CAN_SEND_STATUS寄存器写1 通知从机启动CAN通讯*/
      printf("通知从机启动CAN通讯\r\n");
      Oneline_Master_Write_Reg(REG_CAN_SEND_STATUS,0x01);   //REG_CAN_SEND_STATUS寄存器写1
      SLAVE_CAN_TIMER=1000;           //设置CAN通讯定时器  
      slave_flag=4;
      RPU_EN_High();                  //打开增大电流使能
      break;
    case 4:       /* 打开增大电流使能，提供CAN通讯电流 */
      if(SLAVE_CAN_TIMER==0)
      {
        slave_flag=5;
        slave_wait_count=0;
        RPU_EN_Low();                 //关闭增大电流使能
        delay_ms(5);
      }
      break;
    case 5:     /*读REG_CHARGE_READY寄存器，等待从机就绪，超时时间30ms*/
      printf("等待从机就绪\r\n");
      if(Oneline_Master_Read_Reg(REG_CHARGE_READY)==0x01)
        slave_flag=6;
      else
        slave_wait_count++;
      if(slave_wait_count>=3)         //超时处理30ms
        slave_flag=1;
      break;
    case 6:     /*写REG_CHARGE_STATUS寄存器，通知从机1s后开始充电*/
      printf("通知从机1s后开始充电\r\n");
      Oneline_Master_Write_Reg(REG_CHARGE_STATUS,0x01);
      DQ_TX_1();
      slave_flag=7;
      CHARGE_COUNTDOWN_TIMER=1000;
      break;
    case 7:     /*开启充电*/
      if(CHARGE_COUNTDOWN_TIMER==0)
      {
        DQ_TX_1();
        CHAR_ENABLE();                //开启充电
        printf("开启充电\r\n\r\n");
        switch(low_current_shuttime) {
          case time_5000ms:
            LOW_CURRENT_SAMP_TIMER=5000;  //设置低电流采样定时器倒计时 5s
          break;
          case time_200ms:
            LOW_CURRENT_SAMP_TIMER=200;  //设置低电流采样定时器倒计时 140ms
          break;
        }
        slave_flag=8;
      }
      break;
    case 8:
      break;
  }
}

