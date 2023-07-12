/*
 * mpu6050.h
 *
 *  Created on: 2023��7��10��
 *      Author: GY
 */

#ifndef APP_IIC_IIC_H_
#define APP_IIC_IIC_H_

#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"
#include "mpu6050.h"

#define I2C_SLAVE_ADDR       0x68  //MPU Address
#define I2C_NUMBYTES          1
#define I2C_RNUMBYTES         14  //14 Bytes  Basic params according to 7 varities.
#define Data_NUM            7
#define SelftestAddr    0x0d   //���Լ��������ַ
#define Recieve_Data    0
#define Send_Data     1

Uint16  I2C_rrdy();
Uint16  I2C_xrdy();
Uint16 I2CA_ReadData(struct I2CMSG *msg);
void WriteData(struct I2CMSG *msg,Uint16 *MsgBuffer,Uint16 MemoryAdd,Uint16 NumOfBytes);
void I2CA_Init(void);
__interrupt void i2c_int1a_isr(void);

extern struct I2CMSG *CurrentMsgPtr;
extern struct I2CMSG I2cMsgOut1;
extern struct I2CMSG I2cMsgIn1;
extern Uint8 databuffer[I2C_RNUMBYTES];  //�ݴ洫����8λ���ݣ�
extern Uint16 PassCount; //�����Ĵ���ͳ��
extern Uint16 FailCount; //ʧ��ͳ��
extern Uint16 Sys_stat;

#endif /* APP_IIC_IIC_H_ */
