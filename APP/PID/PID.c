/*
 * PID.c
 *
 *  Created on: 2023年5月29日
 *      Author: gy
 */

#include "PID.h"

PID speed_pid1;
PID speed_pid2;
//初始化参数
PIDConstants pidParams1 = {0.06f, 0.0f, 0.0f};
PIDConstants pidParams2 = {0.06f, 0.0f, 0.0f};

void PID_Init(PID *p,PIDConstants *p2,float Set_speed)
{
    p->sv=Set_speed;    //150r/min
    p->pv=0.0f;
    p->Kp=p2->P;
    p->Ki=p2->I;
    p->Kd=p2->D;
    p->EK=0.0f;
    p->Last_EK=0.0f;
    p->SEK=0;
}

float Get_speed(PID *p,POSSPEED *p2)
{
    #if speed_method == speed_method_M
        if(p2->Speed_fr<750||p2->Speed_fr>-750)
            p->pv = p2->Speed_fr*10;
        else
            p->pv = 0;
    #endif
    #if speed_method == speed_method_T
        p->pv = p2->Speed_pr*10;
    #endif
    return p->pv;
}


void PID_Calc(PID *p)
{
    p->SEK+=p->EK;
    if(p->EK<7500||p->EK>-7500)
    {
        p->EK = p->sv - p->pv;
    }
    p->OUT=p->Kp*p->EK+p->Ki*p->SEK+p->Kd*(p->EK-p->Last_EK);

    p->Last_EK=p->EK;
}

