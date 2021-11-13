#include "./Oneline/oneline.h"

#ifdef ONELINE_HOST
/******************************************************
*********************��������**************************
******************************************************/
//������ʱ��36M��Ƶ�� time*10us
void Oneline_Master_delay(uint32_t time)
{
  uint32_t i=0;
  while((time>0))
  {
    time--;
    for(i=0;i<33;i++);
  }
}

//�������͸�λ�ź�
uint8_t Oneline_Master_RST(void)
{
  uint32_t timeout=0,level_time=0;
  
//  DQ_TX_0();
//  ONELINE_DELAY_10US(1000); //�ϵ�ػ�
  DQ_TX_1();
  ONELINE_DELAY_10US(1000); //����
  /* ������ʼ�ź� */
  DQ_TX_0();
  ONELINE_DELAY_10US(50);
  DQ_TX_1();
  ONELINE_DELAY_10US(10);
  
  /* �ȴ��ӻ���Ӧ */
  timeout=100;   //�ȴ�1000us�ڴӻ�����
  while((DQ_RX_Read()==1)&&(timeout>0))
  {
    timeout--;
    Oneline_Master_delay(1);
  }
  if(timeout==0)
    return 1;
  
  timeout=200;   //����ӻ�����ʱ�� ����ʱ��=level_time*10us ��ʱʱ��2000us
  while((DQ_RX_Read()==0)&&(timeout>0))
  {
    level_time++;
    timeout--;
    Oneline_Master_delay(1);
  }
  
  if(timeout==0)
    return 2;
  
  if(level_time>=7&&level_time<=15) //����ʱ����70us-150us֮�� �ɹ�
    return 0;
  else
    return 3;
}

//����һ���ֽ� ��MSB���У�
void Online_Master_Write_Byte(uint8_t byte)
{
  uint8_t i=0,bit=0,data=0;
  
  data=byte;  
  for(i=0;i<8;i++)
  {
    bit=data&0x80;  //��ȡ���λ
  
    /* ������ʼ�ź� */
    DQ_TX_0();
    ONELINE_DELAY_10US(2);
    
    /* ������Ч����λ */
    if(bit)
    {
      DQ_TX_1();
    }
    else
    {
      DQ_TX_0();
    }
    ONELINE_DELAY_10US(5);
    
    /* �������߽��� */
    DQ_TX_1();
    ONELINE_DELAY_10US(9);
    
    data<<=1;
  }
}

//��ȡһ���ֽ�(MSB����)
uint8_t Online_Master_Read_Byte(void)
{
  uint8_t data=0,i=0;
  for(i=0;i<8;i++)
  {
    /*������ʼ�ź�*/
    DQ_TX_0();
    ONELINE_DELAY_10US(2);
    
    /*�ͷ����� �ɼ�����λ*/
    DQ_TX_1();
    ONELINE_DELAY_10US(4);
    if(DQ_RX_Read()==0)
      data=data<<1;
    else
      data=(data<<1)+1;
    ONELINE_DELAY_10US(2);
    
    /*�������߽���*/
    DQ_TX_1();
    ONELINE_DELAY_10US(8);
  }
  return data;
}

//�������ӻ��Ĵ�������
uint8_t Oneline_Master_Read_Reg(uint8_t reg)
{
  uint8_t data;
  
  Online_Master_Write_Byte(0x01);
  Online_Master_Write_Byte(reg);
  data=Online_Master_Read_Byte();
  
  return data;
}

//�������ӻ��Ĵ�������
void Oneline_Master_Write_Reg(uint8_t reg,uint8_t data)
{
  Online_Master_Write_Byte(0x00);   //���� ����д�ӻ�
  Online_Master_Write_Byte(reg);
  Online_Master_Write_Byte(data);
}

#else //#ifdef ONELINE_SLAVE
/******************************************************
*********************�ӻ�����**************************
******************************************************/

//�ӻ���λӦ��
//�ɹ�����0 ʧ�ܷ��ط�0ֵ
uint8_t Oneline_Slave_RST_Ack(void)
{
  uint32_t timeout=100000;  //��ʱ1s
  uint8_t ret=0;
  if(DQ_RX_Read()==0)
  {
    while((DQ_RX_Read()==0)&&(timeout!=0))  //�ȴ���������
    {
      ONELINE_DELAY_10US(1);
      timeout--;
    }
    if(timeout==0)
    {
      ret=2;
      goto oneline_slave_rst_ack_error;
    }
    ONELINE_DELAY_10US(20);    //����
    DQ_TX_0();                 //����ack�ź�
    ONELINE_DELAY_10US(10);
    DQ_TX_1();
    ret=0;
  }
  else
    ret=1;
  
oneline_slave_rst_ack_error:
  return ret;
}

//�ӻ��ȴ�һ��bit��ʼ
//��ʱʱ��=(timeout*10)us
uint8_t Oneline_Slave_Wait_Bit_Start(uint32_t timeout)
{
  uint32_t i=0;
  while((DQ_RX_Read()==1)&&(timeout>0))
  {
    timeout--;
    for(i=0;i<114;i++);
  }
  if(timeout==0)
    return 1;   //��ʱ����1
  else
    return 0;   //�ɹ�ɨ�赽
}

//�ӻ���ȡһ���ֽ�����(MSB����)
uint8_t Oneline_Slave_Read_Byte(void)
{
  uint8_t data=0,i=0;
  
  for(i=0;i<8;i++)
  {
    /* �ȴ���ʼ�ź� ��ʱʱ��10ms */
    if(Oneline_Slave_Wait_Bit_Start(100)!=0)
      return 0xFF;
    ONELINE_DELAY_10US(2+4);
    
    /* ��ȡ��Ч����λ */
    if(DQ_RX_Read()==0)
      data=data<<1;
    else
      data=(data<<1)+1;
    ONELINE_DELAY_10US(2);
    
    /* �������߽��� */
    DQ_TX_1();
    ONELINE_DELAY_10US(1);
  }
  return data;
}

//�ӻ�����һ���ֽ����ݣ�MSB���У�
uint8_t Oneline_Slave_Write_Byte(uint8_t data)
{
  uint8_t i=0,bit=0;
  
  for(i=0;i<8;i++)
  {
    bit=data&0x80;  //��ȡ���λ
    data<<=1;
    
    /* �ȴ���ʼ�ź� */
    if(Oneline_Slave_Wait_Bit_Start(100)!=0)
      return 0xFF;
    ONELINE_DELAY_10US(2);
    
    /* ������Ч����λ */
    if(bit)
    {
      DQ_TX_1();
    }
    else
    {
      DQ_TX_0();
    }
    ONELINE_DELAY_10US(5);
    
    /* �������߽��� */
    DQ_TX_1();
    ONELINE_DELAY_10US(2);  //��ʱȷ����ƽ���ߣ������ֱ�Ӵ�����һλ����
  }
  return 0;
}

//�ӻ�Ӧ��������д�Ĵ���
uint8_t Oneline_Slave_Read_Write_Reg_ACK(void)
{
  uint8_t direction=0,data=0,reg=0;
  direction=Oneline_Slave_Read_Byte();    //��ȡ����
  if(direction!=0x01&&direction!=0x00)
    return 1;
  
  reg=Oneline_Slave_Read_Byte();          //��ȡ�Ĵ�����ַ
  if(reg==0xff)
    return 2;
  
  if(direction==0x01)   //�������Ĵ���
  {
    if(Oneline_Slave_Write_Byte(Get_Slave_Reg_Value(reg))!=0)  //���Ͷ�Ӧ�Ĵ�������
      return 3;
  }
  else                  //����д�Ĵ���
  {
    data=Oneline_Slave_Read_Byte();       //���ն�Ӧ�Ĵ�������
    if(data==0xff)
      return 4;
    Set_Slave_Reg_Value(reg,data);
  }
  return 0;
}


#endif







