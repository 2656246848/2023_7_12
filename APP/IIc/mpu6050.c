#include "mpu6050.h"
//参数表，通过查表给MPU写入参数值 ：{内部地址，值}
const Uint16 Wparam[][2] =
   {
       // {寄存器地址,寄存器值},
       //------(暂时没用)-----------------
       //{SELF_TEST_X,},
       //{SELF_TEST_Y,},
       //{SELF_TEST_Z,},
       //{SELF_TEST_A,},
       {PWR_MGMT_1,0x01}, // 退出睡眠模式，设取样时钟为陀螺仪X轴。
       //{PWR_MGMT_2,0},
       {SMPLRT_DIV,0x04}, // 取样时钟5分频，1k/5，采样频率为200Hz。采样频率 = 陀螺仪输出频率 / (1+SMPLRT_DIV)。
                           //输出频率是 1Khz 或者 8Khz(只有DLPF即CONFIG寄存器低三位为0或7时，角速度传感器是8kHZ，加速度传感器是固定1kHZ)
       {CONFIG,0x02}, // 低通滤波，截止频率100Hz左右。一般我们设置角速度传感器的带宽为其采样率的一半(200Hz/2,此处设100Hz)
       {GYRO_CONFIG,0x18}, // 陀螺量程 0:250dps;1:500dps;2:1000dps,3:2000dps。一般设置为 3(0x18)
       {ACCEL_CONFIG,0x00}, // 加速度计量程0:2g; 1: 4g;    2:8g; 3:16g。一般设置为 0(0x00)
       /*陀螺仪量程设置：
        *FS_SEL(第3~4位)  0，±250° /S；1，±500° /S； 2，±1000° /S； 3，±2000° /S；我们一般设置为 3，即±2000° /S
        寄存器地址   写入数据    量程
        0x1b          0x00      ±250°/s
        0x1b          0x08      ±500°/s
        0x1b          0x10      ±1000°/s
        0x1b          0x18      ±2000°/s
        加速度计量程设置：
         AFS_SEL(第3~4位)0，±2g； 1，±4g； 2，±8g； 3，±16g；我们一般设置为 0，即±2g
        寄存器地址   写入数据    量程
        0x1c          0x00      ±2G
        0x1c          0x08      ±4G
        0x1c          0x10      ±8G
        0x1c          0x18      ±16G*/
//     {I2C_MST_CTRL,0},    //MPU不作为主设备使用
       {INT_PIN_CFG,0x32}, // 中断信号为高电平，推挽输出，直到有读取操作才消失，直通辅助I2C。
       {INT_ENALE,0x01}, // 使用“数据准备好”中断。
       {USER_CTRL,0x00  },  // 不使用辅助I2C。
       //{FIFO_R_W,0},            //
   };

void MPU6050_Init(void)
{
    I2CA_Init();
    Uint16 i;
    CurrentMsgPtr = &I2cMsgOut1;//当前需发送的数据;
    //清零数据缓冲区
    for(i=0;i<I2C_RNUMBYTES;i+=1)
      {
      databuffer[i]=0;  //clear databuffer
       }
      i=0;
      PassCount=0;
      FailCount=0;
      I2cMsgOut1.MsgStatus = I2C_MSGSTAT_SEND_WITHSTOP;
}

/*-----------------------------------------------初始化函数-------------------------------------------------------*/
void MPU_Initial()
{
    Uint16 Wnum,i,PTR,CONTENT;

    //1. registers configuration
    Wnum=sizeof(Wparam)/2;   //get the size of param_pairs.
   for(i=0;i<Wnum;i++)     //查参数表，给MPU寄存器赋值
      {
         PTR=Wparam[i][1];    //取出需传送的数据值
         CONTENT=Wparam[i][0];   //MPU内部地址
         WriteData(&I2cMsgOut1,&PTR,CONTENT,1);              //write the param to MPU6050 register
 //取一个数，传送一个能否修改为一次传送14个数?
       }
}

void MPU_RUN_CIRCULATION()
{
    Uint16 i;

   if(I2cMsgOut1.MsgStatus == I2C_MSGSTAT_SEND_WITHSTOP)
    {
      DELAY_US(100000); //延时约700us,等待总线稳定
      MPU_Initial(); //初始化MPU6050
      }  // end of write section

  // Read data from RTC section //
  // Check outgoing message status. Bypass read section if status is
  // not inactive.
     if (I2cMsgOut1.MsgStatus == I2C_MSGSTAT_INACTIVE)
    {
     // Check incoming message status.
        if(I2cMsgIn1.MsgStatus == I2C_MSGSTAT_SEND_NOSTOP)//无停止位发送
          {
        // MPU address setup portion
          while(I2CA_ReadData(&I2cMsgIn1) != I2C_SUCCESS) //读数据失败
          {
              if(FailCount>20|PassCount>20){break;}
        }
        // Update current message pointer and message status
        CurrentMsgPtr = &I2cMsgIn1;  //消息指针指向输入数据
        I2cMsgIn1.MsgStatus = I2C_MSGSTAT_SEND_NOSTOP_BUSY;
     }
     else if(I2cMsgIn1.MsgStatus == I2C_MSGSTAT_RESTART)
     {
        DELAY_US(100000);
       //读取MPU6050的14个连续地址数据
        for(i=0;i<I2C_RNUMBYTES;i+=1)
          {
            databuffer[i]= I2cMsgIn1.MsgBuffer[i];  //get the Byte DATA from MPU
           }
        i=0;
        while(I2CA_ReadData(&I2cMsgIn1) != I2C_SUCCESS)
        {
           // Maybe setup an attempt counter to break an infinite while
           // loop.
            if(FailCount>20|PassCount>20){break;}
        }//end of while;
        // Update current message pointer and message status
        CurrentMsgPtr = &I2cMsgIn1;                //指定要读取的数据
        I2cMsgIn1.MsgStatus = I2C_MSGSTAT_READ_BUSY;  //总线修改为读繁忙状态
     }
  }
}
