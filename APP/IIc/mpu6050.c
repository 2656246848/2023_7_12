#include "mpu6050.h"
//������ͨ������MPUд�����ֵ ��{�ڲ���ַ��ֵ}
const Uint16 Wparam[][2] =
   {
       // {�Ĵ�����ַ,�Ĵ���ֵ},
       //------(��ʱû��)-----------------
       //{SELF_TEST_X,},
       //{SELF_TEST_Y,},
       //{SELF_TEST_Z,},
       //{SELF_TEST_A,},
       {PWR_MGMT_1,1}, // �˳�˯��ģʽ����ȡ��ʱ��Ϊ������X�ᡣ
       //{PWR_MGMT_2,0},
       {SMPLRT_DIV,4}, // ȡ��ʱ��4��Ƶ��1k/4��ȡ����Ϊ250Hz��
       {CONFIG,2    }, // ��ͨ�˲�����ֹƵ��100Hz���ҡ�
       {GYRO_CONFIG,0<<3  }, // �������� 0:250dps;1:500dps;2:1000dps,3:2000dps��
       {ACCEL_CONFIG,0<<3   }, // ���ٶȼ�����0:2g; 1: 4g;    2:8g; 3:16g��
//     {I2C_MST_CTRL,0},    //MPU����Ϊ���豸ʹ��
       {INT_PIN_CFG,0x32  }, // �ж��ź�Ϊ�ߵ�ƽ�����������ֱ���ж�ȡ��������ʧ��ֱͨ����I2C��
       {INT_ENALE,1       }, // ʹ�á�����׼���á��жϡ�
       {USER_CTRL,0x00  },  // ��ʹ�ø���I2C��
       //{FIFO_R_W,0},            //
   };

void MPU6050_Init(void)
{
    I2CA_Init();
    Uint16 i;
    CurrentMsgPtr = &I2cMsgOut1;//��ǰ�跢�͵�����;
    //�������ݻ�����
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

/*-----------------------------------------------��ʼ������-------------------------------------------------------*/
void MPU_Initial()
{
    Uint16 Wnum,i,PTR,CONTENT;

    //1. registers configuration
    Wnum=sizeof(Wparam)/2;   //get the size of param_pairs.
   for(i=0;i<Wnum;i++)     //���������MPU�Ĵ�����ֵ
      {
         PTR=Wparam[i][1];    //ȡ���贫�͵�����ֵ
         CONTENT=Wparam[i][0];   //MPU�ڲ���ַ
         WriteData(&I2cMsgOut1,&PTR,CONTENT,1);              //write the param to MPU6050 register
 //ȡһ����������һ���ܷ��޸�Ϊһ�δ���14����?
       }
}

void MPU_RUN_CIRCULATION()
{
    Uint16 i;
    if(Sys_stat==Recieve_Data)
        {
           I2caRegs.I2CFFRX.bit.RXFFIENA=1; //����I2C�ж�

           if(I2cMsgOut1.MsgStatus == I2C_MSGSTAT_SEND_WITHSTOP)
            {
              DELAY_US(100000); //��ʱԼ700us,�ȴ������ȶ�
              MPU_Initial(); //��ʼ��MPU6050
              }  // end of write section

          ///////////////////////////////////
          // Read data from RTC section //
          ///////////////////////////////////

          // Check outgoing message status. Bypass read section if status is
          // not inactive.
             if (I2cMsgOut1.MsgStatus == I2C_MSGSTAT_INACTIVE)
            {
             // Check incoming message status.
                if(I2cMsgIn1.MsgStatus == I2C_MSGSTAT_SEND_NOSTOP)//��ֹͣλ����
                  {
                // MPU address setup portion
                  while(I2CA_ReadData(&I2cMsgIn1) != I2C_SUCCESS) //������ʧ��
                  {
                      if(FailCount>20|PassCount>20){break;}
                }
                // Update current message pointer and message status
                CurrentMsgPtr = &I2cMsgIn1;  //��Ϣָ��ָ����������
                I2cMsgIn1.MsgStatus = I2C_MSGSTAT_SEND_NOSTOP_BUSY;
             }
             else if(I2cMsgIn1.MsgStatus == I2C_MSGSTAT_RESTART)
             {
                DELAY_US(100000);
               //��ȡMPU6050��14��������ַ����
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
                CurrentMsgPtr = &I2cMsgIn1;                //ָ��Ҫ��ȡ������
                I2cMsgIn1.MsgStatus = I2C_MSGSTAT_READ_BUSY;  //�����޸�Ϊ����æ״̬
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
