/*
 * PID.c
 *
 *  Created on: 2023年5月29日
 *      Author: gy
 */

#include "PID.h"

PID speed_pid;//结构体实例化要放到.c文件里

void PID_Init(PID *p)
{
    p->sv=1000.0f;    //100r/min
    p->pv=0.0f;
    p->system_scale = 9.60f;//速度0~960r/min
    p->Kp=P;
    p->Ki=I;
    p->Kd=D;
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

//float Get_Average_Speed(PID *p, POSSPEED *p2)
//{
//    static float speedArray[5]; // 存储每次调用 Get_speed 函数的返回值
//    static int index = 0; // 数组索引
//    int i;
//
//    speedArray[index] = Get_speed(p, p2); // 调用 Get_speed 函数并存储返回值
//
//    index++; // 增加索引
//
//    if (index >= 5) // 如果数组已经存满五个元素
//    {
//        // 舍弃最大和最小值
//        float minVal = speedArray[0];
//        float maxVal = speedArray[0];
//        float sum = speedArray[0];
//
//        // 找出最大和最小值，并计算总和
//        for (i = 1; i < 5; i++)
//        {
//            if (speedArray[i] < minVal)
//                minVal = speedArray[i];
//            if (speedArray[i] > maxVal)
//                maxVal = speedArray[i];
//            sum += speedArray[i];
//        }
//
//        // 求平均值
//        p2->average = (sum - minVal - maxVal) / 3.0;
//        p->pv = p2->average;
//        // 重置索引
//        index = 0;
//
//        return p2->average;
//    }
//
//    return 0.0; // 如果数组未满五个元素，返回0或其他合适的默认值
//}


void PID_Calc(PID *p)
{

    p->SEK+=p->EK;
    if(p->EK<7500||p->EK>-7500)
    {
        p->EK = p->sv - p->pv;
    }
    p->OUT=p->Kp*p->EK+p->Ki*p->SEK+p->Kd*(p->EK-p->Last_EK);
    p->OUT /= p->system_scale;

    p->Last_EK=p->EK;
}

