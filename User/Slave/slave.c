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

//�ӻ�״̬��־
uint8_t slave_flag=0x00;

/** �ӻ����
  * �ɹ�����0 ʧ�ܷ���1
  */
static uint8_t Slave_Detect(void)
{
  uint8_t status=0;
  status=Oneline_Master_RST();
//  printf("status=%d\r\n",status);
  if(status==0)   //��λ�ɹ�
  {
    slave_flag=1;
    printf("�豸ɨ��ɹ�\r\n");
    delay_ms(50);        //��ʱ50ms����
  }
  else
  {
    slave_flag=0;
//    printf("�豸ɨ��ʧ��\r\n");
    delay_ms(30);
  }
  return 1;
}

/* �ӻ�ͨѶ���� */
void Slave_Communication_Task(void)
{
  static uint32_t slave_wait_count=0;
  static uint8_t error_count=0;
//  static uint8_t device_id=0;
  uint8_t data=0;
  
  switch(slave_flag)
  {
    case 0:        /*ɨ���豸*/
      device_id = 0;
      CHAR_DISABLE();                 //�رճ��
      ACS711_PWR_DISABLE();
      RPU_EN_Low();                   //�ر��������ʹ��
      Slave_Detect();
      break;
    case 1:        /*��ȡ�豸ID*/
      data=Oneline_Master_Read_Reg(REG_DEVICE_ID);
      device_id = data;
      printf("REG_DEVICE_ID=%#x\r\n",device_id);
      if((device_id&0xF0)==DEVICE_ID_8A_5S_1 ||
         (device_id&0xF0)==DEVICE_ID_8A_5S_2 )           //�豸ID��ȡ�ɹ�
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
          DQ_TX_0();                  //��λ�ӻ�
          delay_ms(25);
          DQ_TX_1();
          slave_flag=0;
        }
        SLAVE_COMMTASK_TIMER=100;     //��ʱ�ȴ��ȶ�
      }
      break;
    case 2: /* ���ӻ������豸ID����У�飬֮���ȡУ��Ĵ����鿴��� */
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
    case 3:       /*REG_CAN_SEND_STATUS�Ĵ���д1 ֪ͨ�ӻ�����CANͨѶ*/
      printf("֪ͨ�ӻ�����CANͨѶ\r\n");
      Oneline_Master_Write_Reg(REG_CAN_SEND_STATUS,0x01);   //REG_CAN_SEND_STATUS�Ĵ���д1
      SLAVE_CAN_TIMER=1000;           //����CANͨѶ��ʱ��  
      slave_flag=4;
      RPU_EN_High();                  //���������ʹ��
      break;
    case 4:       /* ���������ʹ�ܣ��ṩCANͨѶ���� */
      if(SLAVE_CAN_TIMER==0)
      {
        slave_flag=5;
        slave_wait_count=0;
        RPU_EN_Low();                 //�ر��������ʹ��
        delay_ms(5);
      }
      break;
    case 5:     /*��REG_CHARGE_READY�Ĵ������ȴ��ӻ���������ʱʱ��30ms*/
      printf("�ȴ��ӻ�����\r\n");
      if(Oneline_Master_Read_Reg(REG_CHARGE_READY)==0x01)
        slave_flag=6;
      else
        slave_wait_count++;
      if(slave_wait_count>=3)         //��ʱ����30ms
        slave_flag=1;
      break;
    case 6:     /*дREG_CHARGE_STATUS�Ĵ�����֪ͨ�ӻ�1s��ʼ���*/
      printf("֪ͨ�ӻ�1s��ʼ���\r\n");
      Oneline_Master_Write_Reg(REG_CHARGE_STATUS,0x01);
      DQ_TX_1();
      slave_flag=7;
      CHARGE_COUNTDOWN_TIMER=1000;
      break;
    case 7:     /*�������*/
      if(CHARGE_COUNTDOWN_TIMER==0)
      {
        DQ_TX_1();
        CHAR_ENABLE();                //�������
        printf("�������\r\n\r\n");
        switch(low_current_shuttime) {
          case time_5000ms:
            LOW_CURRENT_SAMP_TIMER=5000;  //���õ͵���������ʱ������ʱ 5s
          break;
          case time_200ms:
            LOW_CURRENT_SAMP_TIMER=200;  //���õ͵���������ʱ������ʱ 140ms
          break;
        }
        slave_flag=8;
      }
      break;
    case 8:
      break;
  }
}

