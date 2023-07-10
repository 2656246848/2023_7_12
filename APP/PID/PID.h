#ifndef APP_PID_PID_H_
#define APP_PID_PID_H_

#include "eqep.h"
#include "speed.h"

/*参数*/
typedef struct
{
    float P;
    float I;
    float D;
} PIDConstants;

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

extern PID speed_pid1;
extern PID speed_pid2;
extern PIDConstants pidParams1;
extern PIDConstants pidParams2;

void PID_Init(PID *p,PIDConstants *p2,float Set_speed);
float Get_speed(PID *p,POSSPEED *p2);
float Get_speed2(PID *p,POSSPEED *p2);
void PID_Calc(PID *p);

#endif /* APP_PID_PID_H_ */
