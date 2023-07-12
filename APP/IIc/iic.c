#include "iic.h"

//��ʼ��һ���������֡ {״̬��MPU��ַ���ֽ������ڲ���ַ}
struct I2CMSG I2cMsgOut1={I2C_MSGSTAT_SEND_WITHSTOP,  //��ʾ������Ϣʱ����ֹͣλ��(���������ݺ����ֹ��ǰ�Ĵ��䣬���ͷ�I2C���ߣ����������豸��ʼ����ͨ��)
                          I2C_SLAVE_ADDR,             //��ʾ���豸�ĵ�ַ��0x68
                          I2C_NUMBYTES,               //��ʾ���͵������ֽ�����
                          SelftestAddr};              //��ʼ��Ϊ�ض����ڲ���ַ��

//��ʼ��һ����������֡ {״̬��MPU��ַ���ֽ������ڲ���ַ}
struct I2CMSG I2cMsgIn1={ I2C_MSGSTAT_SEND_NOSTOP,  //��ʾ������Ϣʱ������ֹͣλ(���������ݺ󲻻���ֹ��ǰ�Ĵ��䣬���Ǳ���I2C���ߵĿ���Ȩ���Ա�������Լ������ͻ���ո��������)
                          I2C_SLAVE_ADDR,           //��ʾ���豸�ĵ�ַ
                          I2C_RNUMBYTES,            //��ʾ���յ������ֽ���
                          ACCEL_XOUT_H};            //��ʼ��Ϊ�ض����ڲ���ַ

struct I2CMSG *CurrentMsgPtr;       // Used in interrupts ��ǰ��Ϣָ��

Uint16 PassCount; //�����Ĵ���ͳ��
Uint16 FailCount; //ʧ��ͳ��
__interrupt void i2c_int1a_isr(void);
Uint8 databuffer[I2C_RNUMBYTES];  //�ݴ洫14���������ݣ�

// I2C ��ʼ��
void I2CA_Init(void)
{
    I2caRegs.I2CMDR.all = 0x0000; //��λIIC ����ʱֹͣ I2C

    EALLOW;

    //Gpio��ʼ��
    GpioCtrlRegs.GPBPUD.bit.GPIO32 = 0;    // ʹ��(SDAA)����
    GpioCtrlRegs.GPBPUD.bit.GPIO33 = 0;    //  ʹ�� (SCLA)����
    GpioCtrlRegs.GPBQSEL1.bit.GPIO32 = 3;  // ͬ�� (SDAA)
    GpioCtrlRegs.GPBQSEL1.bit.GPIO33 = 3;  // ͬ�� (SCLA)
    GpioCtrlRegs.GPBMUX1.bit.GPIO32 = 1;   // ���� GPIO32Ϊ SDAA
    GpioCtrlRegs.GPBMUX1.bit.GPIO33 = 1;   // ����GPIO33 ΪSCLA
    //��λFIFO�Ĵ���
    I2caRegs.I2CFFTX.all = 0x0000;   // Disable FIFO mode and TXFIFO
    I2caRegs.I2CFFRX.all = 0x0040;   // Disable RXFIFO, clear RXFFINT,
    //Ԥ��Ƶ
    #if (CPU_FRQ_150MHZ)            // Default - For 150MHz SYSCLKOUT
        // Prescaler - need 7-12 Mhz on module clk (150/15 = 10MHz)
        I2caRegs.I2CPSC.all = 14;
    #endif
    #if (CPU_FRQ_100MHZ)            // For 100 MHz SYSCLKOUT
        // Prescaler - need 7-12 Mhz on module clk (100/10 = 10MHz)
        I2caRegs.I2CPSC.all = 9;
    #endif
    I2caRegs.I2CCLKL = 10;      //NOTE: must be non zero  ����(ICCL + d)=ʱ�ӵ͵�ƽ����ʱ��
    I2caRegs.I2CCLKH = 5;       // NOTE: must be non zero ����(ICCL + d)=ʱ�Ӹߵ�ƽ����ʱ��

//    I2caRegs.I2CFFTX.all = 0x0000; //Disable FIFO mode and TXFIFO
//    I2caRegs.I2CFFRX.all = 0x0000; //Disable RXFIFO , clear RXFFINT
    I2caRegs.I2CFFTX.all = 0x6000;  // Enable FIFO mode and TXFIFO
    I2caRegs.I2CFFRX.all = 0x2040;  // Enable RXFIFO, clear RXFFINT,
    //I2caRegs.I2CFFRX.all = 0x206F;  //����16λ���жϣ�ʹ��FIFO
    I2caRegs.I2CIER.all = 0x24;       // Enable SCD(��⵽ֹͣ�����ж�) & ARDY interrupts(�Ĵ������ʾ����ж�)
    //I2caRegs.I2CIER.all = 0x00;      // Disable SCD & ARDY interrupts
    I2caRegs.I2CMDR.all = 0x0020; // Take I2C out of reset
                                      // Stop I2C when suspended
    PieCtrlRegs.PIEIER8.bit.INTx1 = 1;//����I2C�ж�
    PieVectTable.I2CINT1A = &i2c_int1a_isr;   //ָ��I2C�жϷ����ӳ���
    IER |= M_INT8; //������CPU�����ж�
    //I2caRegs.I2CSAR = MPU6050_ADDRESS;   // Slave address - mpu6050
    EDIS;
}

/*������I2C��������д�����ݡ�������һ��ָ��I2CMSG�ṹ��ָ����Ϊ������
 * ���а�����Ҫд������ݺ�Ŀ���豸�ĵ�ַ����Ϣ��
 * �����ڲ������ô��豸��ַ�����͵��ֽ�����Ҫ���͵����ݣ���ͨ��д��Ĵ����ķ�ʽ�����ݷ��͵�I2C�����ϡ�*/
Uint16 I2CA_WriteData(struct I2CMSG *msg)
{
   Uint16 i;

   // Wait until the STP bit is cleared from any previous master communication.
   // Clearing of this bit by the module is delayed until after the SCD bit is
   // set. If this bit is not checked prior to initiating a new message, the
   // I2C could get confused.
   if (I2caRegs.I2CMDR.bit.STP == 1)  //���ֹͣλΪ1
   {
      return I2C_STP_NOT_READY_ERROR;  //���ش�����Ϣ
   }

   // Setup slave address
   I2caRegs.I2CSAR = msg->SlaveAddress;  //���ô��豸��ַ

   // Check if bus busy
   if (I2caRegs.I2CSTR.bit.BB == 1)  //���߷�æ��־����STOP�ź�֮�����һ��START�ź�֮ǰ��IIC���ߴ��ڿ���״̬(BB=0)
   {
      return I2C_BUS_BUSY_ERROR;  //���ش�����Ϣ
   }

   // Setup number of bytes to send
   // MsgBuffer + Address
   I2caRegs.I2CCNT = msg->NumOfBytes+1; //������ϴ��豸��ַ 1���ֽ�

   // Setup data to send
//   I2caRegs.I2CDXR = msg->MemoryHighAddr;
   I2caRegs.I2CDXR = msg->MPUAddr; //��ַ���ݶ���Ĵ���
// for (i=0; i<msg->NumOfBytes-2; i++)
   for (i=0; i<msg->NumOfBytes; i++) //ͨ��I2C��FIFO������ȡ14������

   {
      I2caRegs.I2CDXR = *(msg->MsgBuffer+i); //д���ݴ浽���ͼĴ���
   }

   // Send start as master transmitter
   I2caRegs.I2CMDR.all = 0x6E20;  //�趨I2C����ģʽ

   return I2C_SUCCESS;  //���سɹ��ź�
}

/*�ú������ڴ�I2C���߶�ȡ���ݡ���I2CA_WriteData�������ƣ���Ҳ����һ��ָ��I2CMSG�ṹ��ָ����Ϊ������
 * ���а�����Ҫ��ȡ�����ݳ��Ⱥ�Ŀ���豸�ĵ�ַ����Ϣ��
 * �����ڲ������ô��豸��ַ��Ҫ��ȡ���ֽ�������ͨ����ȡ�Ĵ����ķ�ʽ��I2C�����Ͻ������ݡ�*/
Uint16 I2CA_ReadData(struct I2CMSG *msg)
{
   // Wait until the STP bit is cleared from any previous master communication.
   // Clearing of this bit by the module is delayed until after the SCD bit is
   // set. If this bit is not checked prior to initiating a new message, the
   // I2C could get confused.
   if (I2caRegs.I2CMDR.bit.STP == 1)
   {
      return I2C_STP_NOT_READY_ERROR;   //���STPλΪ1���򱨴�
   }

   I2caRegs.I2CSAR = msg->SlaveAddress; //�趨���豸��ַ

   if(msg->MsgStatus == I2C_MSGSTAT_SEND_NOSTOP)
   {
      // Check if bus busy
      if (I2caRegs.I2CSTR.bit.BB == 1)//���ڷ�æ
      {
         return I2C_BUS_BUSY_ERROR;  //���ش�����Ϣ
      }
      I2caRegs.I2CCNT = 1;//�����ֽڵĵ�ַ
//      I2caRegs.I2CDXR = msg->MemoryHighAddr;
      I2caRegs.I2CDXR = msg->MPUAddr; //��ַ�����Ĵ���
      I2caRegs.I2CMDR.all = 0x2620;         // Send data to setup MPU address ���͵�ַ
   }  //���ʹ��豸��ַ
   else if(msg->MsgStatus == I2C_MSGSTAT_RESTART)
   {
      I2caRegs.I2CCNT = msg->NumOfBytes;    // Setup how many bytes to expect //���ݳ���
      I2caRegs.I2CMDR.all = 0x2C20;         // Send restart as master receiver //��������
   }

   return I2C_SUCCESS;  //���ش���ɹ�״̬�ź�
}


/*�ú�������������I2C����д�����ݡ�
 * ������һ��ָ��I2CMSG�ṹ��ָ�롢һ��ָ�����ݻ�������ָ�롢Ҫд����ڲ���ַ��Ҫд����ֽ�����Ϊ������
 * �����ڲ��Ὣ����д��I2CMSG�ṹ��MsgBuffer�����У�������I2CA_WriteData���������ݷ��͵�I2C�����ϡ�*/
void WriteData(struct I2CMSG *msg,Uint16 *MsgBuffer,Uint16 MemoryAdd,Uint16 NumOfBytes)
{
    Uint16 i,Error;
    for(i = 0; i < I2C_RNUMBYTES; i++)
    {
        msg->MsgBuffer[i] = MsgBuffer[i];       //�����ݴ������ṹ��ĸ�������
    }
    //msg->MemoryHighAddr = MemoryAdd >> 8;
    msg->MPUAddr = MemoryAdd & 0xff;  //ȡ��8λ
    msg->NumOfBytes = NumOfBytes;  //д�����ֽ�
    Error = I2CA_WriteData(&I2cMsgOut1); //����I2CA_WriteData

    if (Error == I2C_SUCCESS)
    {
        CurrentMsgPtr = &I2cMsgOut1;    //�贫�͵�����ָ��ָ��I2cMsgOut1,I2cMsgOut1���������֡
        I2cMsgOut1.MsgStatus = I2C_MSGSTAT_WRITE_BUSY;
    }
    while(I2cMsgOut1.MsgStatus != I2C_MSGSTAT_INACTIVE);
    DELAY_US(1000);
}

void pass()
{
    asm("  ESTOP0");  //������ֹͣ����
    PassCount++;
   // for(;;);
}

void fail()
{
    asm("  ESTOP0");  //������passͬ��������
    FailCount++;
   // for(;;);
}

/*��7�ֻ����жϣ����Բ�ѯI2caRegs.I2CISRC.allֵ�ж�
 *0��none ���¼�
 *1�� Arbitration lost �ٲð���
 *2�� No-acknowledgment condition detected �ӻ�δӦ��
 *3�� Registers ready to be accessed �Ĵ���׼���ñ����� ��
 *4�� Receive data ready �������ݾ�����FIFOģʽ�±�FIFO�жϴ���
 *5�� Transmit data ready �������ݾ�����FIFOģʽ�±�FIFO�жϴ���
 *6�� Stop condition detected ��⵽ֹͣ״̬ ��
 *7�� Addressed as slave �ӻ��ѱ�ռ��
 *���⣺��������FIFO�ж�
 * */

//*--------------------------------�жϷ����ӳ���--------------------------------*/
__interrupt void i2c_int1a_isr(void)     // I2CA �жϷ����ӳ���
{
   Uint16 IntSource, i;
   // Read interrupt source
   IntSource = I2caRegs.I2CISRC.all;
  DINT;
   // Interrupt source = stop condition detected
   if(IntSource == I2C_SCD_ISRC)//ֹͣ������SCLΪ�ߵ�ƽ��SDA�ɸߵ�ƽ��Ϊ�͵�ƽ������⵽ʱ����������ѯSCDλ����
   {
      // If completed message was writing data, reset msg to inactive state
      if (CurrentMsgPtr->MsgStatus == I2C_MSGSTAT_WRITE_BUSY)
      {
         CurrentMsgPtr->MsgStatus = I2C_MSGSTAT_INACTIVE;  //����Ϣ״̬�޸�Ϊ��æ
      }
      else
      {
          // If completed message was writing data
         if(CurrentMsgPtr->MsgStatus == I2C_MSGSTAT_SEND_NOSTOP_BUSY)
         {
            CurrentMsgPtr->MsgStatus = I2C_MSGSTAT_SEND_NOSTOP;
         }
         // If completed message was reading RTC data, reset msg to inactive state
         // and read data from FIFO.
         else if (CurrentMsgPtr->MsgStatus == I2C_MSGSTAT_READ_BUSY)
         {
            CurrentMsgPtr->MsgStatus = I2C_MSGSTAT_SEND_NOSTOP;//I2C_MSGSTAT_INACTIVE;
            for(i=0; i < CurrentMsgPtr->NumOfBytes; i++)  //read 14data from MPU and save them in MsgBuffer
            {
              CurrentMsgPtr->MsgBuffer[i] = I2caRegs.I2CDRR;  //������ȡFIFO�Ĵ���������
            }
         }
      }
   }
   else if(IntSource == I2C_ARDY_ISRC)//��I2Cģ��׼���÷��ʼĴ���ʱ����������ж�(֮ǰ��̵ĵ�ַ�����ݡ�����ֵ�Ѿ���ʹ��)����������ѯARDYλ����
   {
      if(I2caRegs.I2CSTR.bit.NACK == 1)
      {
         I2caRegs.I2CMDR.bit.STP = 1;
         I2caRegs.I2CSTR.all = I2C_CLR_NACK_BIT;
      }
      else if(CurrentMsgPtr->MsgStatus == I2C_MSGSTAT_SEND_NOSTOP_BUSY)
      {
         CurrentMsgPtr->MsgStatus = I2C_MSGSTAT_RESTART;  //��λ��Ϣ״̬
      }
   }  // end of register access ready

   else
   {
      // Generate some error due to invalid interrupt source
      asm("   ESTOP0");  //�д���Ļ���ֹͣ���棬
   }

   // Enable future I2C (PIE Group 8) interrupts

   PieCtrlRegs.PIEACK.all = PIEACK_GROUP8 | PIEACK_GROUP9; //ʹ�������ж� ��8��
   EINT;
}
