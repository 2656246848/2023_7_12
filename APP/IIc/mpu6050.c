#include "mpu6050.h"

char DataDispose_mode=0;//数据处理模式
char Get_OriginalState=1;//是否是第一次计算角度(第一次的加速度计解算角用于初始化陀螺仪解算角)
#define Standard_mode        0//获得零飘偏移量
#define ReadServices_mode    1//计算角度

MPU6050_t MPU6050;
const double Accel_Z_corrector = 14418.0;
#define RAD_TO_DEG 57.295779513082320876798154814105  //180除以Π，用于弧度转角度
float gx_offset = 0.0f;//漂移量
float gy_offset = 0.0f;
float gz_offset = 0.0f;
static int i = 0;
double dt=0.0;
ac_angle ac_angle1={
            .ne_sqrt=0.0,
            .ac_roll=0.0,
            .ac_pitch=0.0,
};//加速度计解算出来的俯仰角和横滚角
gy_angle gy_angle1={
            .gy_yaw=0.0,  //【-pi pi】
            .gy_pitch=0.0,//【-90° ～90°】
            .gy_roll=0.0, //【-pi pi】
            .y_rate=0.0,  //yaw方向角速度(大地坐标系)
            .r_rate=0.0,  //roll方向角速度
            .p_rate=0.0,  //pitch方向角速度

};//陀螺仪解算出来的俯仰角和横滚角
Uint32 timer=0;//存储时刻，用于计算dt
void remove_offset(MPU6050_t *DataStruct);
void acce_calculating(MPU6050_t *DataStruct,ac_angle *a);
void gyro_calculating(MPU6050_t *DataStruct,gy_angle *b, double dt);
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

/*--------------------------------------主函数while中调用用以更新databuffer[i]---------------------------------------*/
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
        //MPU6050_Read_All(&MPU6050);DELAY_US(100000);
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

/*------------------将databuffer[i]中数据转换为需要的形式----------------------*/
void MPU6050_Read_All(MPU6050_t *DataStruct) {
    int16 temp;

    // Read 14 BYTES of data starting from ACCEL_XOUT_H register
    MPU_RUN_CIRCULATION();

    //拼接低位和高位
    DataStruct->Accel_X_RAW = (int16) (databuffer[0] << 8 | databuffer[1]);
    DataStruct->Accel_Y_RAW = (int16) (databuffer[2] << 8 | databuffer[3]);
    DataStruct->Accel_Z_RAW = (int16) (databuffer[4] << 8 | databuffer[5]);
    temp = (int16) (databuffer[6] << 8 | databuffer[7]);
    DataStruct->Gyro_X_RAW = (int16) (databuffer[8] << 8 | databuffer[9]);
    DataStruct->Gyro_Y_RAW = (int16) (databuffer[10] << 8 | databuffer[11]);
    DataStruct->Gyro_Z_RAW = (int16) (databuffer[12] << 8 | databuffer[13]);

    DataStruct->Ax = DataStruct->Accel_X_RAW / 16384.0f;//灵敏度=65536/(2(g)*2)=16384LSB/g 计算得到加速度计输出的加速度
    DataStruct->Ay = DataStruct->Accel_Y_RAW / 16384.0f;
    DataStruct->Az = DataStruct->Accel_Z_RAW / 16384.0f;
    //DataStruct->Az = DataStruct->Accel_Z_RAW / Accel_Z_corrector;
    DataStruct->Temperature = (float) ((int16) temp / (float) 340.0 + (float) 36.53);
    DataStruct->Gx = DataStruct->Gyro_X_RAW / 16.384f;   //灵敏度=65536/(2000(dps)*2)=16.4LSB/(°/S) 计算得到陀螺仪输出的角速度
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
    //static float dT = 0.005;//5ms一次
    static int times = 200;//采样200次去零漂,共1s时间
    if(i<times){
        gx_offset = gx_offset + DataStruct->Gx;//。陀螺仪去零飘；累积200次。静止时Gx,Gy,Gz理论上为0
        gy_offset = gy_offset + DataStruct->Gy;
        gz_offset = gz_offset + DataStruct->Gz;
        i++;
    }
    else{
        gx_offset/=times;gy_offset/=times;gz_offset/=times;
        DataDispose_mode = ReadServices_mode;//零漂偏移量计算完成，切换模式
    }
}

void acce_calculating(MPU6050_t *DataStruct,ac_angle *a)    //得到加速度计解算的roll和pitch角
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
    //a->ac_yaw = atan2(DataStruct->Accel_X_RAW, DataStruct->Accel_Z_RAW) * RAD_TO_DEG;//没有太大参考价值
    //atan，输入参数是斜率：y/x，返回角度范围是 [-π/2, π/2];atan2，数入参数是y，x，返回完整的角度范围 [-π, π]。一般知道参数象限用atan不知道时用atan2
    //滚转角(rool)的范围通常为 -180 度到 +180 度，俯仰角(pitch)的范围通常为 -90 度到 +90 度。计算roll用atan2好，计算pitch用atan好
    if(Get_OriginalState==1)
    {
        gy_angle1.gy_yaw = 0;
        gy_angle1.gy_roll = a->ac_roll;
        gy_angle1.gy_pitch = a->ac_pitch;
        Get_OriginalState = 0;
    }
}

void gyro_calculating(MPU6050_t *DataStruct,gy_angle *b, double dt) //得到陀螺仪解算的三个角
{
    //去零飘
//    b->r_rate = (DataStruct->Gx - gx_offset) + (sin(b->gy_pitch )*sin(b->gy_roll)/cos(b->gy_pitch))*(DataStruct->Gy - gy_offset)+
//                   (cos(b->gy_roll)*sin(b->gy_pitch)/cos(b->gy_pitch))*(DataStruct->Gz- gz_offset);
//    b->p_rate = cos(b->gy_roll)*(DataStruct->Gy - gy_offset) - sin(b->gy_roll)*(DataStruct->Gz - gz_offset);
//    b->y_rate = (sin(b->gy_roll)/cos(b->gy_pitch))*(DataStruct->Gy - gy_offset) + (cos(b->gy_roll)/cos(b->gy_pitch))*(DataStruct->Gz - gz_offset);
    b->r_rate = DataStruct->Gx - gx_offset;
    b->p_rate = DataStruct->Gy - gy_offset;
    b->y_rate = DataStruct->Gz- gz_offset;

    b->gy_yaw += b->y_rate * dt;//角度积分，大地坐标系
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
