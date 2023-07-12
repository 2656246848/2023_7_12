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
       {PWR_MGMT_1,1}, // 退出睡眠模式，设取样时钟为陀螺仪X轴。
       //{PWR_MGMT_2,0},
       {SMPLRT_DIV,4}, // 取样时钟4分频，1k/4，取样率为250Hz。
       {CONFIG,2    }, // 低通滤波，截止频率100Hz左右。
       {GYRO_CONFIG,0<<3  }, // 陀螺量程 0:250dps;1:500dps;2:1000dps,3:2000dps。
       {ACCEL_CONFIG,0<<3   }, // 加速度计量程0:2g; 1: 4g;    2:8g; 3:16g。
//     {I2C_MST_CTRL,0},    //MPU不作为主设备使用
       {INT_PIN_CFG,0x32  }, // 中断信号为高电平，推挽输出，直到有读取操作才消失，直通辅助I2C。
       {INT_ENALE,1       }, // 使用“数据准备好”中断。
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
      Sys_stat=Recieve_Data;
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
    if(Sys_stat==Recieve_Data)
        {
           I2caRegs.I2CFFRX.bit.RXFFIENA=1; //允许I2C中断

           if(I2cMsgOut1.MsgStatus == I2C_MSGSTAT_SEND_WITHSTOP)
            {
              DELAY_US(100000); //延时约700us,等待总线稳定
              MPU_Initial(); //初始化MPU6050
              }  // end of write section

          ///////////////////////////////////
          // Read data from RTC section //
          ///////////////////////////////////

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
    //             ACCEL_X=Rparams[0];   //give their names.
    //             ACCEL_Y=Rparams[1];
    //             ACCEL_Z=Rparams[2];
    //             TEMP=Rparams[3];
    //             GYRO_X=Rparams[4];
    //             GYRO_Y=Rparams[5];
    //             GYRO_Z=Rparams[6];
                while(I2CA_ReadData(&I2cMsgIn1) != I2C_SUCCESS)
                {
                   // Maybe setup an attempt counter to break an infinite while
                   // loop.
                    if(FailCount>20|PassCount>20){break;}
                }//end of while;
                // Update current message pointer and message status
                CurrentMsgPtr = &I2cMsgIn1;                //指定要读取的数据
                I2cMsgIn1.MsgStatus = I2C_MSGSTAT_READ_BUSY;  //总线修改为读繁忙状态
                Sys_stat=Send_Data;
                Sys_stat=Recieve_Data;
             }

          }
       }
       else if(Sys_stat==Send_Data)
       {
            I2caRegs.I2CFFRX.bit.RXFFIENA=0;
            I2caRegs.I2CFFRX.bit.RXFFINTCLR=0;//
            Sys_stat=Recieve_Data;
       }
       else
       {
         Sys_stat=Recieve_Data;
       }

          // end of for(;;)
}
