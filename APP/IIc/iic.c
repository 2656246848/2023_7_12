#include "iic.h"

//初始化一个输出数据帧 {状态、MPU地址、字节数，内部地址}
struct I2CMSG I2cMsgOut1={I2C_MSGSTAT_SEND_WITHSTOP,  //表示发送消息时带有停止位。(发送完数据后会终止当前的传输，并释放I2C总线，允许其他设备开始进行通信)
                          I2C_SLAVE_ADDR,             //表示从设备的地址。0x68
                          I2C_NUMBYTES,               //表示发送的数据字节数。
                          SelftestAddr};              //初始化为特定的内部地址。

//初始化一个读入数据帧 {状态、MPU地址、字节数，内部地址}
struct I2CMSG I2cMsgIn1={ I2C_MSGSTAT_SEND_NOSTOP,  //表示发送消息时不带有停止位(发送完数据后不会终止当前的传输，而是保持I2C总线的控制权，以便后续可以继续发送或接收更多的数据)
                          I2C_SLAVE_ADDR,           //表示从设备的地址
                          I2C_RNUMBYTES,            //表示接收的数据字节数
                          ACCEL_XOUT_H};            //初始化为特定的内部地址

struct I2CMSG *CurrentMsgPtr;       // Used in interrupts 当前消息指针

Uint16 PassCount; //跳过的次数统计
Uint16 FailCount; //失败统计
__interrupt void i2c_int1a_isr(void);
Uint8 databuffer[I2C_RNUMBYTES];  //暂存传14个感器数据；

// I2C 初始化
void I2CA_Init(void)
{
    I2caRegs.I2CMDR.all = 0x0000; //复位IIC 挂起时停止 I2C

    EALLOW;

    //Gpio初始化
    GpioCtrlRegs.GPBPUD.bit.GPIO32 = 0;    // 使能(SDAA)上拉
    GpioCtrlRegs.GPBPUD.bit.GPIO33 = 0;    //  使能 (SCLA)上拉
    GpioCtrlRegs.GPBQSEL1.bit.GPIO32 = 3;  // 同步 (SDAA)
    GpioCtrlRegs.GPBQSEL1.bit.GPIO33 = 3;  // 同步 (SCLA)
    GpioCtrlRegs.GPBMUX1.bit.GPIO32 = 1;   // 配置 GPIO32为 SDAA
    GpioCtrlRegs.GPBMUX1.bit.GPIO33 = 1;   // 配置GPIO33 为SCLA
    //复位FIFO寄存器
    I2caRegs.I2CFFTX.all = 0x0000;   // Disable FIFO mode and TXFIFO
    I2caRegs.I2CFFRX.all = 0x0040;   // Disable RXFIFO, clear RXFFINT,
    //预分频
    #if (CPU_FRQ_150MHZ)            // Default - For 150MHz SYSCLKOUT
        // Prescaler - need 7-12 Mhz on module clk (150/15 = 10MHz)
        I2caRegs.I2CPSC.all = 14;
    #endif
    #if (CPU_FRQ_100MHZ)            // For 100 MHz SYSCLKOUT
        // Prescaler - need 7-12 Mhz on module clk (100/10 = 10MHz)
        I2caRegs.I2CPSC.all = 9;
    #endif
    I2caRegs.I2CCLKL = 10;      //NOTE: must be non zero  乘上(ICCL + d)=时钟低电平持续时间
    I2caRegs.I2CCLKH = 5;       // NOTE: must be non zero 乘上(ICCL + d)=时钟高电平持续时间

//    I2caRegs.I2CFFTX.all = 0x0000; //Disable FIFO mode and TXFIFO
//    I2caRegs.I2CFFRX.all = 0x0000; //Disable RXFIFO , clear RXFFINT
    I2caRegs.I2CFFTX.all = 0x6000;  // Enable FIFO mode and TXFIFO
    I2caRegs.I2CFFRX.all = 0x2040;  // Enable RXFIFO, clear RXFFINT,
    //I2caRegs.I2CFFRX.all = 0x206F;  //接收16位进中断，使能FIFO
    I2caRegs.I2CIER.all = 0x24;       // Enable SCD(检测到停止条件中断) & ARDY interrupts(寄存器访问就绪中断)
    //I2caRegs.I2CIER.all = 0x00;      // Disable SCD & ARDY interrupts
    I2caRegs.I2CMDR.all = 0x0020; // Take I2C out of reset
                                      // Stop I2C when suspended
    PieCtrlRegs.PIEIER8.bit.INTx1 = 1;//允许I2C中断
    PieVectTable.I2CINT1A = &i2c_int1a_isr;   //指定I2C中断服务子程序
    IER |= M_INT8; //允许向CPU发送中断
    //I2caRegs.I2CSAR = MPU6050_ADDRESS;   // Slave address - mpu6050
    EDIS;
}

/*用于向I2C总线连续写入数据。它接受一个指向I2CMSG结构的指针作为参数，
 * 其中包含了要写入的数据和目标设备的地址等信息。
 * 函数内部会设置从设备地址、发送的字节数和要发送的数据，并通过写入寄存器的方式将数据发送到I2C总线上。*/
Uint16 I2CA_WriteData(struct I2CMSG *msg)
{
   Uint16 i;

   // Wait until the STP bit is cleared from any previous master communication.
   // Clearing of this bit by the module is delayed until after the SCD bit is
   // set. If this bit is not checked prior to initiating a new message, the
   // I2C could get confused.
   if (I2caRegs.I2CMDR.bit.STP == 1)  //如果停止位为1
   {
      return I2C_STP_NOT_READY_ERROR;  //返回错误信息
   }

   // Setup slave address
   I2caRegs.I2CSAR = msg->SlaveAddress;  //设置从设备地址

   // Check if bus busy
   if (I2caRegs.I2CSTR.bit.BB == 1)  //总线繁忙标志，在STOP信号之后和下一个START信号之前，IIC总线处于空闲状态(BB=0)
   {
      return I2C_BUS_BUSY_ERROR;  //返回错误信息
   }

   // Setup number of bytes to send
   // MsgBuffer + Address
   I2caRegs.I2CCNT = msg->NumOfBytes+1; //还需加上从设备地址 1个字节

   // Setup data to send
//   I2caRegs.I2CDXR = msg->MemoryHighAddr;
   I2caRegs.I2CDXR = msg->MPUAddr; //地址数据读入寄存器
// for (i=0; i<msg->NumOfBytes-2; i++)
   for (i=0; i<msg->NumOfBytes; i++) //通过I2C的FIFO连续读取14个参数

   {
      I2caRegs.I2CDXR = *(msg->MsgBuffer+i); //写数据存到发送寄存器
   }

   // Send start as master transmitter
   I2caRegs.I2CMDR.all = 0x6E20;  //设定I2C工作模式

   return I2C_SUCCESS;  //返回成功信号
}

/*该函数用于从I2C总线读取数据。与I2CA_WriteData函数类似，它也接受一个指向I2CMSG结构的指针作为参数，
 * 其中包含了要读取的数据长度和目标设备的地址等信息。
 * 函数内部会设置从设备地址和要读取的字节数，并通过读取寄存器的方式从I2C总线上接收数据。*/
Uint16 I2CA_ReadData(struct I2CMSG *msg)
{
   // Wait until the STP bit is cleared from any previous master communication.
   // Clearing of this bit by the module is delayed until after the SCD bit is
   // set. If this bit is not checked prior to initiating a new message, the
   // I2C could get confused.
   if (I2caRegs.I2CMDR.bit.STP == 1)
   {
      return I2C_STP_NOT_READY_ERROR;   //如果STP位为1，则报错
   }

   I2caRegs.I2CSAR = msg->SlaveAddress; //设定从设备地址

   if(msg->MsgStatus == I2C_MSGSTAT_SEND_NOSTOP)
   {
      // Check if bus busy
      if (I2caRegs.I2CSTR.bit.BB == 1)//现在繁忙
      {
         return I2C_BUS_BUSY_ERROR;  //返回错误信息
      }
      I2caRegs.I2CCNT = 1;//传几字节的地址
//      I2caRegs.I2CDXR = msg->MemoryHighAddr;
      I2caRegs.I2CDXR = msg->MPUAddr; //地址发往寄存器
      I2caRegs.I2CMDR.all = 0x2620;         // Send data to setup MPU address 发送地址
   }  //发送从设备地址
   else if(msg->MsgStatus == I2C_MSGSTAT_RESTART)
   {
      I2caRegs.I2CCNT = msg->NumOfBytes;    // Setup how many bytes to expect //数据长度
      I2caRegs.I2CMDR.all = 0x2C20;         // Send restart as master receiver //发送数据
   }

   return I2C_SUCCESS;  //返回传输成功状态信号
}


/*该函数用于连续向I2C总线写入数据。
 * 它接受一个指向I2CMSG结构的指针、一个指向数据缓冲区的指针、要写入的内部地址和要写入的字节数作为参数。
 * 函数内部会将数据写入I2CMSG结构的MsgBuffer数组中，并调用I2CA_WriteData函数将数据发送到I2C总线上。*/
void WriteData(struct I2CMSG *msg,Uint16 *MsgBuffer,Uint16 MemoryAdd,Uint16 NumOfBytes)
{
    Uint16 i,Error;
    for(i = 0; i < I2C_RNUMBYTES; i++)
    {
        msg->MsgBuffer[i] = MsgBuffer[i];       //将数据传送至结构体的附加数组
    }
    //msg->MemoryHighAddr = MemoryAdd >> 8;
    msg->MPUAddr = MemoryAdd & 0xff;  //取低8位
    msg->NumOfBytes = NumOfBytes;  //写几个字节
    Error = I2CA_WriteData(&I2cMsgOut1); //调用I2CA_WriteData

    if (Error == I2C_SUCCESS)
    {
        CurrentMsgPtr = &I2cMsgOut1;    //需传送的数据指针指向I2cMsgOut1,I2cMsgOut1是输出数据帧
        I2cMsgOut1.MsgStatus = I2C_MSGSTAT_WRITE_BUSY;
    }
    while(I2cMsgOut1.MsgStatus != I2C_MSGSTAT_INACTIVE);
    DELAY_US(1000);
}

void pass()
{
    asm("  ESTOP0");  //跳过，停止仿真
    PassCount++;
   // for(;;);
}

void fail()
{
    asm("  ESTOP0");  //出错，跟pass同样的作用
    FailCount++;
   // for(;;);
}

/*共7种基本中断：可以查询I2caRegs.I2CISRC.all值判断
 *0：none 无事件
 *1： Arbitration lost 仲裁败诉
 *2： No-acknowledgment condition detected 从机未应答
 *3： Registers ready to be accessed 寄存器准备好被访问 √
 *4： Receive data ready 接收数据就绪，FIFO模式下被FIFO中断代替
 *5： Transmit data ready 传输数据就绪，FIFO模式下被FIFO中断代替
 *6： Stop condition detected 检测到停止状态 √
 *7： Addressed as slave 从机已被占用
 *另外：可以配置FIFO中断
 * */

//*--------------------------------中断服务子程序--------------------------------*/
__interrupt void i2c_int1a_isr(void)     // I2CA 中断服务子程序
{
   Uint16 IntSource, i;
   // Read interrupt source
   IntSource = I2caRegs.I2CISRC.all;
  DINT;
   // Interrupt source = stop condition detected
   if(IntSource == I2C_SCD_ISRC)//停止条件（SCL为高电平，SDA由高电平变为低电平）被检测到时，考虑用轮询SCD位代替
   {
      // If completed message was writing data, reset msg to inactive state
      if (CurrentMsgPtr->MsgStatus == I2C_MSGSTAT_WRITE_BUSY)
      {
         CurrentMsgPtr->MsgStatus = I2C_MSGSTAT_INACTIVE;  //将信息状态修改为繁忙
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
              CurrentMsgPtr->MsgBuffer[i] = I2caRegs.I2CDRR;  //连续读取FIFO寄存器的数据
            }
         }
      }
   }
   else if(IntSource == I2C_ARDY_ISRC)//当I2C模块准备好访问寄存器时，会产生此中断(之前编程的地址、数据、命令值已经被使用)。考虑用轮询ARDY位代替
   {
      if(I2caRegs.I2CSTR.bit.NACK == 1)
      {
         I2caRegs.I2CMDR.bit.STP = 1;
         I2caRegs.I2CSTR.all = I2C_CLR_NACK_BIT;
      }
      else if(CurrentMsgPtr->MsgStatus == I2C_MSGSTAT_SEND_NOSTOP_BUSY)
      {
         CurrentMsgPtr->MsgStatus = I2C_MSGSTAT_RESTART;  //复位消息状态
      }
   }  // end of register access ready

   else
   {
      // Generate some error due to invalid interrupt source
      asm("   ESTOP0");  //有错误的话，停止仿真，
   }

   // Enable future I2C (PIE Group 8) interrupts

   PieCtrlRegs.PIEACK.all = PIEACK_GROUP8 | PIEACK_GROUP9; //使能外设中断 第8组
   EINT;
}
