#ifndef APP_IIC_MPU6050_H_
#define APP_IIC_MPU6050_H_
#include "iic.h"
#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"
#include "time.h"
#include "kalman.h"

// MPU6050 structure
typedef struct {

    int16 Accel_X_RAW;//加速度计三轴数据
    int16 Accel_Y_RAW;
    int16 Accel_Z_RAW;
    double Ax;
    double Ay;
    double Az;

    int16 Gyro_X_RAW;//陀螺仪三轴数据
    int16 Gyro_Y_RAW;
    int16 Gyro_Z_RAW;
    double Gx;
    double Gy;
    double Gz;

    float Temperature;

    double KalmanAngleX;
    double KalmanAngleY;
} MPU6050_t;

typedef struct {    //加速度计解算出来的俯仰角和横滚角
    double ne_sqrt;
//    double ac_yaw;
    double ac_roll;
    double ac_pitch;
}ac_angle;

typedef struct {    //陀螺仪解算出来的俯仰角和横滚角
    double gy_yaw;  //【-pi pi】
    double gy_pitch;//【-90° ～90°】
    double gy_roll; //【-pi pi】
    double y_rate;  //yaw方向角速度(大地坐标系)
    double r_rate;  //roll方向角速度
    double p_rate;  //pitch方向角速度
}gy_angle;

void MPU_RUN_CIRCULATION();
void MPU_Initial();
void MPU6050_Init(void);
void MPU6050_Read_All(MPU6050_t *DataStruct);
void acce_calculating(MPU6050_t *DataStruct,ac_angle *a);    //得到加速度计解算的roll和pitch角
void gyro_calculating(MPU6050_t *DataStruct,gy_angle *b, double dt); //得到陀螺仪解算的三个角
void remove_offset(MPU6050_t *DataStruct); //计算零飘偏移量

extern MPU6050_t MPU6050;
extern gy_angle gy_angle1;

//mpu6050寄存器
#define MPU6050_SMPLRT_DIV      0x19
#define MPU6050_CONFIG          0x1A
#define MPU6050_GYRO_CONFIG     0x1B
#define MPU6050_ACCEL_CONFIG    0x1C

#define MPU6050_ACCEL_XOUT_H    0x3B
#define MPU6050_ACCEL_XOUT_L    0x3C
#define MPU6050_ACCEL_YOUT_H    0x3D
#define MPU6050_ACCEL_YOUT_L    0x3E
#define MPU6050_ACCEL_ZOUT_H    0x3F
#define MPU6050_ACCEL_ZOUT_L    0x40
#define MPU6050_TEMP_OUT_H      0x41
#define MPU6050_TEMP_OUT_L      0x42
#define MPU6050_GYRO_XOUT_H     0x43
#define MPU6050_GYRO_XOUT_L     0x44
#define MPU6050_GYRO_YOUT_H     0x45
#define MPU6050_GYRO_YOUT_L     0x46
#define MPU6050_GYRO_ZOUT_H     0x47
#define MPU6050_GYRO_ZOUT_L     0x48

#define MPU6050_PWR_MGMT_1      0x6B
#define MPU6050_PWR_MGMT_2      0x6C
#define MPU6050_WHO_AM_I        0x75


//*---------------------自检寄存器----------------------------------------------*/
#define SELF_TEST_X 0x0D
#define SELF_TEST_Y 0x0E
#define SELF_TEST_Z 0x0F
#define SELF_TEST_A 0x10

//*---------------------控制寄存器----------------------------------------------*/
#define SMPLRT_DIV  0x19    //*采样率分频，典型值：0x07(125Hz) */
#define CONFIG   0x1A       // *低通滤波频率，典型值：0x06(5Hz) */
#define GYRO_CONFIG  0x1B   // *陀螺仪自检及测量范围，典型值：0x18(不自检，2000deg/s) */
#define ACCEL_CONFIG 0x1C  // *加速计自检、测量范围及高通滤波频率，典型值：0x01(不自检，2G，5Hz) */
#define FIFO_R_W 0X74

#define INT_PIN_CFG 0x37
#define INT_ENALE 0x38
#define INT_STATUS 0x37
#define SIGNAL_PATH_RESET 0x68
#define USER_CTRL 0x6A
#define INT_PIN_CFG 0x37


#define PWR_MGMT_1  0x6B // *电源管理，典型值：0x00(正常启用) */
#define WHO_AM_I  0x75 //*IIC地址寄存器(默认数值0x68，只读) */

//*---------------------数据寄存器----------------------------------------------*/
#define ACCEL_XOUT_H 0x3B  // 存储最近的X轴、Y轴、Z轴加速度感应器的测量值 */
#define ACCEL_XOUT_L 0x3C
#define ACCEL_YOUT_H 0x3D
#define ACCEL_YOUT_L 0x3E
#define ACCEL_ZOUT_H 0x3F
#define ACCEL_ZOUT_L 0x40

#define TEMP_OUT_H  0x41   // 存储的最近温度传感器的测量值 */
#define TEMP_OUT_L  0x42  //READ ONLY

#define GYRO_XOUT_H  0x43 // 存储最近的X轴、Y轴、Z轴陀螺仪感应器的测量值 */
#define GYRO_XOUT_L  0x44
#define GYRO_YOUT_H  0x45
#define GYRO_YOUT_L  0x46
#define GYRO_ZOUT_H  0x47
#define GYRO_ZOUT_L  0x48

#define FIFO_COUNTH 0X72  //* 存储FIFO的计数值*/
#define FIFO_COUNTL 0X73

//*--------------------DMP    寄存器----------------------------------------------*/



#endif /* APP_IIC_MPU6050_H_ */
