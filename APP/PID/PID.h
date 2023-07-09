#ifndef APP_PID_PID_H_
#define APP_PID_PID_H_

#include "eqep.h"
#include "speed.h"

/*PID参数相关宏*/
#define     P               0.06f        //2.5
#define     I               0.0f        //0
#define     D               0.0f        //0

#define     P2               0.06f        //2.5
#define     I2               0.0f        //0
#define     D2               0.0f        //0

/*PID结构体*/
typedef struct
{
    float sv;//设定值
    float pv;//实际值
    float Kp;
    float Kd;
    float Ki;
    float EK;//本次偏差
    float Last_EK;//上次偏差
    long SEK;//历史偏差之和
    float OUT;
    float system_scale;

}PID;

extern PID speed_pid;
extern PID speed_pid2;

void PID_Init();
void PID2_Init();
float Get_speed(PID *p,POSSPEED *p2);
float Get_speed2(PID *p,POSSPEED *p2);
void PID_Calc(PID *p);

#endif /* APP_PID_PID_H_ */
