#ifndef APP_IIC_MPU6050_H_
#define APP_IIC_MPU6050_H_
#include "iic.h"
#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"
#include "time.h"
#include "kalman.h"

// MPU6050 structure
typedef struct {

    int16 Accel_X_RAW;//���ٶȼ���������
    int16 Accel_Y_RAW;
    int16 Accel_Z_RAW;
    double Ax;
    double Ay;
    double Az;

    int16 Gyro_X_RAW;//��������������
    int16 Gyro_Y_RAW;
    int16 Gyro_Z_RAW;
    double Gx;
    double Gy;
    double Gz;

    float Temperature;

    double KalmanAngleX;
    double KalmanAngleY;
} MPU6050_t;

typedef struct {    //���ٶȼƽ�������ĸ����Ǻͺ����
    double ne_sqrt;
//    double ac_yaw;
    double ac_roll;
    double ac_pitch;
}ac_angle;

typedef struct {    //�����ǽ�������ĸ����Ǻͺ����
    double gy_yaw;  //��-pi pi��
    double gy_pitch;//��-90�� ��90�㡿
    double gy_roll; //��-pi pi��
    double y_rate;  //yaw������ٶ�(�������ϵ)
    double r_rate;  //roll������ٶ�
    double p_rate;  //pitch������ٶ�
}gy_angle;

void MPU_RUN_CIRCULATION();
void MPU_Initial();
void MPU6050_Init(void);
void MPU6050_Read_All(MPU6050_t *DataStruct);
void acce_calculating(MPU6050_t *DataStruct,ac_angle *a);    //�õ����ٶȼƽ����roll��pitch��
void gyro_calculating(MPU6050_t *DataStruct,gy_angle *b, double dt); //�õ������ǽ����������
void remove_offset(MPU6050_t *DataStruct); //������Ʈƫ����

extern MPU6050_t MPU6050;
extern gy_angle gy_angle1;

//mpu6050�Ĵ���
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


//*---------------------�Լ�Ĵ���----------------------------------------------*/
#define SELF_TEST_X 0x0D
#define SELF_TEST_Y 0x0E
#define SELF_TEST_Z 0x0F
#define SELF_TEST_A 0x10

//*---------------------���ƼĴ���----------------------------------------------*/
#define SMPLRT_DIV  0x19    //*�����ʷ�Ƶ������ֵ��0x07(125Hz) */
#define CONFIG   0x1A       // *��ͨ�˲�Ƶ�ʣ�����ֵ��0x06(5Hz) */
#define GYRO_CONFIG  0x1B   // *�������Լ켰������Χ������ֵ��0x18(���Լ죬2000deg/s) */
#define ACCEL_CONFIG 0x1C  // *���ټ��Լ졢������Χ����ͨ�˲�Ƶ�ʣ�����ֵ��0x01(���Լ죬2G��5Hz) */
#define FIFO_R_W 0X74

#define INT_PIN_CFG 0x37
#define INT_ENALE 0x38
#define INT_STATUS 0x37
#define SIGNAL_PATH_RESET 0x68
#define USER_CTRL 0x6A
#define INT_PIN_CFG 0x37


#define PWR_MGMT_1  0x6B // *��Դ��������ֵ��0x00(��������) */
#define WHO_AM_I  0x75 //*IIC��ַ�Ĵ���(Ĭ����ֵ0x68��ֻ��) */

//*---------------------���ݼĴ���----------------------------------------------*/
#define ACCEL_XOUT_H 0x3B  // �洢�����X�ᡢY�ᡢZ����ٶȸ�Ӧ���Ĳ���ֵ */
#define ACCEL_XOUT_L 0x3C
#define ACCEL_YOUT_H 0x3D
#define ACCEL_YOUT_L 0x3E
#define ACCEL_ZOUT_H 0x3F
#define ACCEL_ZOUT_L 0x40

#define TEMP_OUT_H  0x41   // �洢������¶ȴ������Ĳ���ֵ */
#define TEMP_OUT_L  0x42  //READ ONLY

#define GYRO_XOUT_H  0x43 // �洢�����X�ᡢY�ᡢZ�������Ǹ�Ӧ���Ĳ���ֵ */
#define GYRO_XOUT_L  0x44
#define GYRO_YOUT_H  0x45
#define GYRO_YOUT_L  0x46
#define GYRO_ZOUT_H  0x47
#define GYRO_ZOUT_L  0x48

#define FIFO_COUNTH 0X72  //* �洢FIFO�ļ���ֵ*/
#define FIFO_COUNTL 0X73

//*--------------------DMP    �Ĵ���----------------------------------------------*/



#endif /* APP_IIC_MPU6050_H_ */
