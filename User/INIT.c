/*
 * INIT.c
 *
 *  Created on: 2023年5月29日
 *      Author: gy
 */

#include "main.h"

void SYSTERM_INIT()
{
    InitSysCtrl();
    //初始化PIE控制寄存器和PIE中断向量表
    InitPieCtrl();
    IER = 0x0000;
    IFR = 0x0000;
    InitPieVectTable();
    LED_Init();
    //EXTI_Init();
    OLED_Init();
    OLED_Clear();
    EQEP1_Init();
    UARTa_Init(115200);
    UARTb_Init(115200);
    TIM1_Init(150, 999);//1ms进一次中断
    TIM2_Init(150,sampling_T*1.2-1);
    PID_Init(&speed_pid1,&pidParams1,1500.0f);
    PID_Init(&speed_pid2,&pidParams2,1500.0f);
    MPU6050_Init();//这个放在MPU_Initial();前面
    //MPU_Initial();
    WatchDog_init();
}
