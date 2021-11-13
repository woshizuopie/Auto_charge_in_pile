#include "./Oneline/oneline.h"

#ifdef ONELINE_HOST
/******************************************************
*********************主机操作**************************
******************************************************/
//主机延时（36M主频） time*10us
void Oneline_Master_delay(uint32_t time)
{
  uint32_t i=0;
  while((time>0))
  {
    time--;
    for(i=0;i<33;i++);
  }
}

//主机发送复位信号
uint8_t Oneline_Master_RST(void)
{
  uint32_t timeout=0,level_time=0;
  
//  DQ_TX_0();
//  ONELINE_DELAY_10US(1000); //断电关机
  DQ_TX_1();
  ONELINE_DELAY_10US(1000); //蓄能
  /* 发送起始信号 */
  DQ_TX_0();
  ONELINE_DELAY_10US(50);
  DQ_TX_1();
  ONELINE_DELAY_10US(10);
  
  /* 等待从机响应 */
  timeout=100;   //等待1000us内从机拉低
  while((DQ_RX_Read()==1)&&(timeout>0))
  {
    timeout--;
    Oneline_Master_delay(1);
  }
  if(timeout==0)
    return 1;
  
  timeout=200;   //计算从机拉低时间 拉低时间=level_time*10us 超时时间2000us
  while((DQ_RX_Read()==0)&&(timeout>0))
  {
    level_time++;
    timeout--;
    Oneline_Master_delay(1);
  }
  
  if(timeout==0)
    return 2;
  
  if(level_time>=7&&level_time<=15) //拉低时间在70us-150us之间 成功
    return 0;
  else
    return 3;
}

//发送一个字节 （MSB先行）
void Online_Master_Write_Byte(uint8_t byte)
{
  uint8_t i=0,bit=0,data=0;
  
  data=byte;  
  for(i=0;i<8;i++)
  {
    bit=data&0x80;  //读取最高位
  
    /* 发送起始信号 */
    DQ_TX_0();
    ONELINE_DELAY_10US(2);
    
    /* 发送有效数据位 */
    if(bit)
    {
      DQ_TX_1();
    }
    else
    {
      DQ_TX_0();
    }
    ONELINE_DELAY_10US(5);
    
    /* 拉高总线结束 */
    DQ_TX_1();
    ONELINE_DELAY_10US(9);
    
    data<<=1;
  }
}

//读取一个字节(MSB先行)
uint8_t Online_Master_Read_Byte(void)
{
  uint8_t data=0,i=0;
  for(i=0;i<8;i++)
  {
    /*发送起始信号*/
    DQ_TX_0();
    ONELINE_DELAY_10US(2);
    
    /*释放总线 采集数据位*/
    DQ_TX_1();
    ONELINE_DELAY_10US(4);
    if(DQ_RX_Read()==0)
      data=data<<1;
    else
      data=(data<<1)+1;
    ONELINE_DELAY_10US(2);
    
    /*拉高总线结束*/
    DQ_TX_1();
    ONELINE_DELAY_10US(8);
  }
  return data;
}

//主机读从机寄存器数据
uint8_t Oneline_Master_Read_Reg(uint8_t reg)
{
  uint8_t data;
  
  Online_Master_Write_Byte(0x01);
  Online_Master_Write_Byte(reg);
  data=Online_Master_Read_Byte();
  
  return data;
}

//主机读从机寄存器数据
void Oneline_Master_Write_Reg(uint8_t reg,uint8_t data)
{
  Online_Master_Write_Byte(0x00);   //方向 主机写从机
  Online_Master_Write_Byte(reg);
  Online_Master_Write_Byte(data);
}

#else //#ifdef ONELINE_SLAVE
/******************************************************
*********************从机操作**************************
******************************************************/

//从机复位应答
//成功返回0 失败返回非0值
uint8_t Oneline_Slave_RST_Ack(void)
{
  uint32_t timeout=100000;  //延时1s
  uint8_t ret=0;
  if(DQ_RX_Read()==0)
  {
    while((DQ_RX_Read()==0)&&(timeout!=0))  //等待总线拉高
    {
      ONELINE_DELAY_10US(1);
      timeout--;
    }
    if(timeout==0)
    {
      ret=2;
      goto oneline_slave_rst_ack_error;
    }
    ONELINE_DELAY_10US(20);    //蓄能
    DQ_TX_0();                 //发送ack信号
    ONELINE_DELAY_10US(10);
    DQ_TX_1();
    ret=0;
  }
  else
    ret=1;
  
oneline_slave_rst_ack_error:
  return ret;
}

//从机等待一个bit开始
//超时时间=(timeout*10)us
uint8_t Oneline_Slave_Wait_Bit_Start(uint32_t timeout)
{
  uint32_t i=0;
  while((DQ_RX_Read()==1)&&(timeout>0))
  {
    timeout--;
    for(i=0;i<114;i++);
  }
  if(timeout==0)
    return 1;   //超时返回1
  else
    return 0;   //成功扫描到
}

//从机读取一个字节数据(MSB先行)
uint8_t Oneline_Slave_Read_Byte(void)
{
  uint8_t data=0,i=0;
  
  for(i=0;i<8;i++)
  {
    /* 等待起始信号 超时时间10ms */
    if(Oneline_Slave_Wait_Bit_Start(100)!=0)
      return 0xFF;
    ONELINE_DELAY_10US(2+4);
    
    /* 读取有效数据位 */
    if(DQ_RX_Read()==0)
      data=data<<1;
    else
      data=(data<<1)+1;
    ONELINE_DELAY_10US(2);
    
    /* 拉高总线结束 */
    DQ_TX_1();
    ONELINE_DELAY_10US(1);
  }
  return data;
}

//从机发送一个字节数据（MSB先行）
uint8_t Oneline_Slave_Write_Byte(uint8_t data)
{
  uint8_t i=0,bit=0;
  
  for(i=0;i<8;i++)
  {
    bit=data&0x80;  //读取最高位
    data<<=1;
    
    /* 等待起始信号 */
    if(Oneline_Slave_Wait_Bit_Start(100)!=0)
      return 0xFF;
    ONELINE_DELAY_10US(2);
    
    /* 发送有效数据位 */
    if(bit)
    {
      DQ_TX_1();
    }
    else
    {
      DQ_TX_0();
    }
    ONELINE_DELAY_10US(5);
    
    /* 拉高总线结束 */
    DQ_TX_1();
    ONELINE_DELAY_10US(2);  //延时确保电平拉高，否则会直接触发下一位发送
  }
  return 0;
}

//从机应答主机读写寄存器
uint8_t Oneline_Slave_Read_Write_Reg_ACK(void)
{
  uint8_t direction=0,data=0,reg=0;
  direction=Oneline_Slave_Read_Byte();    //获取方向
  if(direction!=0x01&&direction!=0x00)
    return 1;
  
  reg=Oneline_Slave_Read_Byte();          //获取寄存器地址
  if(reg==0xff)
    return 2;
  
  if(direction==0x01)   //主机读寄存器
  {
    if(Oneline_Slave_Write_Byte(Get_Slave_Reg_Value(reg))!=0)  //发送对应寄存器数据
      return 3;
  }
  else                  //主机写寄存器
  {
    data=Oneline_Slave_Read_Byte();       //接收对应寄存器数据
    if(data==0xff)
      return 4;
    Set_Slave_Reg_Value(reg,data);
  }
  return 0;
}


#endif







