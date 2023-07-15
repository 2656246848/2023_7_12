#include "time.h"
#include "leds.h"

//定时器1初始化函数
//Freq：CPU时钟频率（150MHz）
//Period：定时周期值，单位us
/*********定时器1任务变量************/
Uint32 Systime=0;   // 全局变量，用于存储毫秒级计数值
unsigned int Timer_5ms_Task;
unsigned int Timer_10ms_Task;
unsigned int Timer_50ms_Task;
unsigned int Timer_100ms_Task;
unsigned int Timer_1s_Task;

void TIM1_Init(float Freq, float Period)
{
    EALLOW;
    SysCtrlRegs.PCLKCR3.bit.CPUTIMER1ENCLK = 1; // CPU Timer 1
    EDIS;

    //设置定时器1的中断入口地址为中断向量表的INT13
    EALLOW;
    PieVectTable.XINT13 = &TIM1_IRQn;
    EDIS;

    //指向定时器1的寄存器地址
    CpuTimer1.RegsAddr = &CpuTimer1Regs;
    //设置定时器1的周期寄存器值
    CpuTimer1Regs.PRD.all  = 0xFFFFFFFF;
    //设置定时器预定标计数器值为0
    CpuTimer1Regs.TPR.all  = 0;
    CpuTimer1Regs.TPRH.all = 0;
    //确保定时器1为停止状态
    CpuTimer1Regs.TCR.bit.TSS = 1;
    //重载使能
    CpuTimer1Regs.TCR.bit.TRB = 1;
    // Reset interrupt counters:
    CpuTimer1.InterruptCount = 0;

    ConfigCpuTimer(&CpuTimer1, Freq, Period);

    //开始定时器功能
    CpuTimer1Regs.TCR.bit.TSS=0;
    //定时器1直接连到内核中断，无需配置PIE
    IER |= M_INT13;

    //使能总中断
    EINT;
    ERTM;

}

interrupt void TIM1_IRQn(void)
{
    EALLOW;
    Systime++;
    if(Systime%5==0)
        Timer_5ms_Task=1;
    if(Systime%10==0)
        Timer_10ms_Task=1;
    if(Systime%50==0)
        Timer_50ms_Task=1;
    if(Systime%100==0)
        Timer_100ms_Task=1;
    if(Systime%1000==0)
        Timer_1s_Task=1;
    if(Systime>=8000000) Systime=0;

    //LED2_TOGGLE;
    CpuTimer1Regs.TCR.bit.TSS=0;//启动定时器
    EDIS;
}


