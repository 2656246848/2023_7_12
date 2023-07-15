#include "mpu6050.h"

char DataDispose_mode=0;//���ݴ���ģʽ
char Get_OriginalState=1;//�Ƿ��ǵ�һ�μ���Ƕ�(��һ�εļ��ٶȼƽ�������ڳ�ʼ�������ǽ����)
#define Standard_mode        0//�����Ʈƫ����
#define ReadServices_mode    1//����Ƕ�

MPU6050_t MPU6050;
const double Accel_Z_corrector = 14418.0;
#define RAD_TO_DEG 57.295779513082320876798154814105  //180���Ԧ������ڻ���ת�Ƕ�
float gx_offset = 0.0f;//Ư����
float gy_offset = 0.0f;
float gz_offset = 0.0f;
static int i = 0;
double dt=0.0;
ac_angle ac_angle1={
            .ne_sqrt=0.0,
            .ac_roll=0.0,
            .ac_pitch=0.0,
};//���ٶȼƽ�������ĸ����Ǻͺ����
gy_angle gy_angle1={
            .gy_yaw=0.0,  //��-pi pi��
            .gy_pitch=0.0,//��-90�� ��90�㡿
            .gy_roll=0.0, //��-pi pi��
            .y_rate=0.0,  //yaw������ٶ�(�������ϵ)
            .r_rate=0.0,  //roll������ٶ�
            .p_rate=0.0,  //pitch������ٶ�

};//�����ǽ�������ĸ����Ǻͺ����
Uint32 timer=0;//�洢ʱ�̣����ڼ���dt
void remove_offset(MPU6050_t *DataStruct);
void acce_calculating(MPU6050_t *DataStruct,ac_angle *a);
void gyro_calculating(MPU6050_t *DataStruct,gy_angle *b, double dt);
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

/*--------------------------------------������while�е������Ը���databuffer[i]---------------------------------------*/
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
        //MPU6050_Read_All(&MPU6050);DELAY_US(100000);
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

/*------------------��databuffer[i]������ת��Ϊ��Ҫ����ʽ----------------------*/
void MPU6050_Read_All(MPU6050_t *DataStruct) {
    int16 temp;

    // Read 14 BYTES of data starting from ACCEL_XOUT_H register
    MPU_RUN_CIRCULATION();

    //ƴ�ӵ�λ�͸�λ
    DataStruct->Accel_X_RAW = (int16) (databuffer[0] << 8 | databuffer[1]);
    DataStruct->Accel_Y_RAW = (int16) (databuffer[2] << 8 | databuffer[3]);
    DataStruct->Accel_Z_RAW = (int16) (databuffer[4] << 8 | databuffer[5]);
    temp = (int16) (databuffer[6] << 8 | databuffer[7]);
    DataStruct->Gyro_X_RAW = (int16) (databuffer[8] << 8 | databuffer[9]);
    DataStruct->Gyro_Y_RAW = (int16) (databuffer[10] << 8 | databuffer[11]);
    DataStruct->Gyro_Z_RAW = (int16) (databuffer[12] << 8 | databuffer[13]);

    DataStruct->Ax = DataStruct->Accel_X_RAW / 16384.0f;//������=65536/(2(g)*2)=16384LSB/g ����õ����ٶȼ�����ļ��ٶ�
    DataStruct->Ay = DataStruct->Accel_Y_RAW / 16384.0f;
    DataStruct->Az = DataStruct->Accel_Z_RAW / 16384.0f;
    //DataStruct->Az = DataStruct->Accel_Z_RAW / Accel_Z_corrector;
    DataStruct->Temperature = (float) ((int16) temp / (float) 340.0 + (float) 36.53);
    DataStruct->Gx = DataStruct->Gyro_X_RAW / 16.384f;   //������=65536/(2000(dps)*2)=16.4LSB/(��/S) ����õ�����������Ľ��ٶ�
    DataStruct->Gy = DataStruct->Gyro_Y_RAW / 16.384f;
    DataStruct->Gz = DataStruct->Gyro_Z_RAW / 16.384f;


   dt = ( Systime > timer) ? (double)(Systime - timer) / 1000 : (double)(Systime + 8000000 - timer) / 1000;
   timer = Systime;
//   if ()
   //for (i = 1;i<=2000;i++)
    if (DataDispose_mode == Standard_mode){
        remove_offset(DataStruct);
    }
    else if (DataDispose_mode == ReadServices_mode)
    {
        acce_calculating(DataStruct,&ac_angle1);
        gyro_calculating(DataStruct,&gy_angle1,dt);
    }
}

void remove_offset(MPU6050_t *DataStruct)
{
    //static float dT = 0.005;//5msһ��
    static int times = 200;//����200��ȥ��Ư,��1sʱ��
    if(i<times){
        gx_offset = gx_offset + DataStruct->Gx;//��������ȥ��Ʈ���ۻ�200�Ρ���ֹʱGx,Gy,Gz������Ϊ0
        gy_offset = gy_offset + DataStruct->Gy;
        gz_offset = gz_offset + DataStruct->Gz;
        i++;
    }
    else{
        gx_offset/=times;gy_offset/=times;gz_offset/=times;
        DataDispose_mode = ReadServices_mode;//��Ưƫ����������ɣ��л�ģʽ
    }
}

void acce_calculating(MPU6050_t *DataStruct,ac_angle *a)    //�õ����ٶȼƽ����roll��pitch��
{

    //a->ne_sqrt = sqrt((double)(DataStruct->Accel_Y_RAW) * (double)(DataStruct->Accel_Y_RAW) + (double)(DataStruct->Accel_Z_RAW) * (double)(DataStruct->Accel_Z_RAW));
    a->ne_sqrt = sqrt((double)DataStruct->Accel_Y_RAW * DataStruct->Accel_Y_RAW + (double)DataStruct->Accel_Z_RAW * DataStruct->Accel_Z_RAW);

    if (a->ne_sqrt != 0){
        a->ac_pitch = atan(-DataStruct->Accel_X_RAW/a->ne_sqrt) * RAD_TO_DEG;
    }
    else{
        a->ac_pitch = 0;
    }
    a->ac_roll = atan2(DataStruct->Accel_Y_RAW, DataStruct->Accel_Z_RAW) * RAD_TO_DEG;
    //a->ac_yaw = atan2(DataStruct->Accel_X_RAW, DataStruct->Accel_Z_RAW) * RAD_TO_DEG;//û��̫��ο���ֵ
    //atan�����������б�ʣ�y/x�����ؽǶȷ�Χ�� [-��/2, ��/2];atan2�����������y��x�����������ĽǶȷ�Χ [-��, ��]��һ��֪������������atan��֪��ʱ��atan2
    //��ת��(rool)�ķ�Χͨ��Ϊ -180 �ȵ� +180 �ȣ�������(pitch)�ķ�Χͨ��Ϊ -90 �ȵ� +90 �ȡ�����roll��atan2�ã�����pitch��atan��
    if(Get_OriginalState==1)
    {
        gy_angle1.gy_yaw = 0;
        gy_angle1.gy_roll = a->ac_roll;
        gy_angle1.gy_pitch = a->ac_pitch;
        Get_OriginalState = 0;
    }
}

void gyro_calculating(MPU6050_t *DataStruct,gy_angle *b, double dt) //�õ������ǽ����������
{
    //ȥ��Ʈ
//    b->r_rate = (DataStruct->Gx - gx_offset) + (sin(b->gy_pitch )*sin(b->gy_roll)/cos(b->gy_pitch))*(DataStruct->Gy - gy_offset)+
//                   (cos(b->gy_roll)*sin(b->gy_pitch)/cos(b->gy_pitch))*(DataStruct->Gz- gz_offset);
//    b->p_rate = cos(b->gy_roll)*(DataStruct->Gy - gy_offset) - sin(b->gy_roll)*(DataStruct->Gz - gz_offset);
//    b->y_rate = (sin(b->gy_roll)/cos(b->gy_pitch))*(DataStruct->Gy - gy_offset) + (cos(b->gy_roll)/cos(b->gy_pitch))*(DataStruct->Gz - gz_offset);
    b->r_rate = DataStruct->Gx - gx_offset;
    b->p_rate = DataStruct->Gy - gy_offset;
    b->y_rate = DataStruct->Gz- gz_offset;

    b->gy_yaw += b->y_rate * dt;//�ǶȻ��֣��������ϵ
    b->gy_roll += b->r_rate * dt;
    b->gy_pitch += b->p_rate * dt;
    if(b->gy_yaw>180){
        b->gy_yaw-=360;
    }
    if(b->gy_yaw<-180){
       b->gy_yaw+=360;
   }
    if(b->gy_roll>180){
        b->gy_roll-=360;
    }
    if(b->gy_roll<-180){
       b->gy_roll+=360;
   }
    if(b->gy_pitch>90){
        b->gy_pitch-=180;
    }
    if(b->gy_pitch<-90){
       b->gy_pitch+=180;
   }

}
