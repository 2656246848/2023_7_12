#ifndef APP_IIC_MPU6050_H_
#define APP_IIC_MPU6050_H_
#include "iic.h"
#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"

#define MPU6050_ADDRESS     0xD0

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
void MPU_RUN_CIRCULATION();
void MPU_Initial();
void MPU6050_Init(void);
void MPU6050_GetData(int16 *AccX, int16 *AccY, int16 *AccZ,
                        int16 *GyroX, int16 *GyroY, int16 *GyroZ);

#endif /* APP_IIC_MPU6050_H_ */
