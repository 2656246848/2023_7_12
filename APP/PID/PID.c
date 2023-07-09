/*
 * PID.c
 *
 *  Created on: 2023Äê5ÔÂ29ÈÕ
 *      Author: gy
 */

#include "PID.h"

PID speed_pid;
PID speed_pid2;

void PID_Init(PID *p)
{
    p->sv=1500.0f;    //150r/min
    p->pv=0.0f;
    p->Kp=P;
    p->Ki=I;
    p->Kd=D;
    p->EK=0.0f;
    p->Last_EK=0.0f;
    p->SEK=0;
}

void PID2_Init(PID *p)
{
    p->sv=1500.0f;    //150r/min
    p->pv=0.0f;
    p->Kp=P2;
    p->Ki=I2;
    p->Kd=D2;
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

