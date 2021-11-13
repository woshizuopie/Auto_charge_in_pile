#ifndef __ONE_LINE_H_
#define __ONE_LINE_H_

#include "./Peripher_GPIO/peripher_gpio.h"
#include "./TIM/tim.h"

#define DQ_TX_1()     DQ_TX_Low()
#define DQ_TX_0()     DQ_TX_High()
//#define DQ_RX_Read()  DQ_RX_Read()

#define ONELINE_DELAY_10US(x)     Tim4_Delay_x10us(x)


#define ONELINE_HOST
#ifdef ONELINE_HOST    /*  主机  */
void Oneline_Master_delay(uint32_t time);
uint8_t Oneline_Master_RST(void);
void Online_Master_Write_Byte(uint8_t byte);
uint8_t Online_Master_Read_Byte(void);

uint8_t Oneline_Master_Read_Reg(uint8_t reg);
void Oneline_Master_Write_Reg(uint8_t reg,uint8_t data);
#else   /*  从机  */

uint8_t Oneline_Slave_RST_Ack(void);
uint8_t Oneline_Slave_Wait_Bit_Start(uint32_t timeout);
uint8_t Oneline_Slave_Read_Byte(void);

uint8_t Oneline_Slave_Write_Byte(uint8_t data);
uint8_t Oneline_Slave_Read_Write_Reg_ACK(void);
#endif

#endif /*__ONE_LINE_H_*/

