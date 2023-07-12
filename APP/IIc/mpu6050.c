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
       {PWR_MGMT_1,0x01}, // �˳�˯��ģʽ����ȡ��ʱ��Ϊ������X�ᡣ
       //{PWR_MGMT_2,0},
       {SMPLRT_DIV,0x04}, // ȡ��ʱ��5��Ƶ��1k/5������Ƶ��Ϊ200Hz������Ƶ�� = ���������Ƶ�� / (1+SMPLRT_DIV)��
                           //���Ƶ���� 1Khz ���� 8Khz(ֻ��DLPF��CONFIG�Ĵ�������λΪ0��7ʱ�����ٶȴ�������8kHZ�����ٶȴ������ǹ̶�1kHZ)
       {CONFIG,0x02}, // ��ͨ�˲�����ֹƵ��100Hz���ҡ�һ���������ý��ٶȴ������Ĵ���Ϊ������ʵ�һ��(200Hz/2,�˴���100Hz)
       {GYRO_CONFIG,0x18}, // �������� 0:250dps;1:500dps;2:1000dps,3:2000dps��һ������Ϊ 3(0x18)
       {ACCEL_CONFIG,0x00}, // ���ٶȼ�����0:2g; 1: 4g;    2:8g; 3:16g��һ������Ϊ 0(0x00)
       /*�������������ã�
        *FS_SEL(��3~4λ)  0����250�� /S��1����500�� /S�� 2����1000�� /S�� 3����2000�� /S������һ������Ϊ 3������2000�� /S
        �Ĵ�����ַ   д������    ����
        0x1b          0x00      ��250��/s
        0x1b          0x08      ��500��/s
        0x1b          0x10      ��1000��/s
        0x1b          0x18      ��2000��/s
        ���ٶȼ��������ã�
         AFS_SEL(��3~4λ)0����2g�� 1����4g�� 2����8g�� 3����16g������һ������Ϊ 0������2g
        �Ĵ�����ַ   д������    ����
        0x1c          0x00      ��2G
        0x1c          0x08      ��4G
        0x1c          0x10      ��8G
        0x1c          0x18      ��16G*/
//     {I2C_MST_CTRL,0},    //MPU����Ϊ���豸ʹ��
       {INT_PIN_CFG,0x32}, // �ж��ź�Ϊ�ߵ�ƽ�����������ֱ���ж�ȡ��������ʧ��ֱͨ����I2C��
       {INT_ENALE,0x01}, // ʹ�á�����׼���á��жϡ�
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

   if(I2cMsgOut1.MsgStatus == I2C_MSGSTAT_SEND_WITHSTOP)
    {
      DELAY_US(100000); //��ʱԼ700us,�ȴ������ȶ�
      MPU_Initial(); //��ʼ��MPU6050
      }  // end of write section

  // Read data from RTC section //
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
        while(I2CA_ReadData(&I2cMsgIn1) != I2C_SUCCESS)
        {
           // Maybe setup an attempt counter to break an infinite while
           // loop.
            if(FailCount>20|PassCount>20){break;}
        }//end of while;
        // Update current message pointer and message status
        CurrentMsgPtr = &I2cMsgIn1;                //ָ��Ҫ��ȡ������
        I2cMsgIn1.MsgStatus = I2C_MSGSTAT_READ_BUSY;  //�����޸�Ϊ����æ״̬
     }
  }
}
