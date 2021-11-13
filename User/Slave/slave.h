#ifndef __SLAVE_H_
#define __SLAVE_H_

#include "stm32f10x.h"

#define REG_WHO_AM_I          0x00
#define REG_DEVICE_ID         0x01
#define REG_CAN_SEND_STATUS   0x02
#define REG_CHARGE_READY      0x03
#define REG_CHARGE_STATUS     0x04
#define REG_DEVICE_ID_CHECK   0x05

#define DEVICE_ID_8A_5S_1     0x40
#define DEVICE_ID_8A_5S_2     0x50
#define DEVICE_ID_3_5A_5S     0x71
#define DEVICE_ID_8A_200MS    0x33

//从机状态标志
extern uint8_t slave_flag;
void Slave_Communication_Task(void);

#endif 

