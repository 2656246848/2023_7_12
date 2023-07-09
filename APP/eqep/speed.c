/*
 * speed.h
 *
 *  Created on: 2023年4月16日
 *      Author: gy
 */

#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"     // Device Headerfile and Examples Include File
#include "speed.h"   // Example specific Include file

POSSPEED qep_posspeed=POSSPEED_DEFAULTS;
POSSPEED qep_posspeed2=POSSPEED_DEFAULTS;

void  POSSPEED_Init(void)
{

    EQep1Regs.QUPRD=CPU_CLK*sampling_T/1000000;
    EQep2Regs.QUPRD=CPU_CLK*sampling_T/1000000;

    EQep1Regs.QDECCTL.bit.QSRC=00;      // QEP quadrature count mode 正交计数模式 对AB进行4倍频
    EQep1Regs.QEPCTL.bit.FREE_SOFT=2;  //产生断点时位置计数器不受影响
    EQep1Regs.QEPCTL.bit.PCRM=10;       // PCRM=10 mode - QPOSCNT reset on first index event      QPOSCNT(位置计数器)的时钟来源于QCLK
    EQep1Regs.QEPCTL.bit.UTE=1;         // Unit Timeout Enable
    EQep1Regs.QEPCTL.bit.QCLM=1;        // Latch on unit time out   单位时间事件发生时锁存数据。位置计数器值、捕获计时器和捕获周期值保存在QPOSLAT、QCTMRLAT和QCPRDLAT寄存器中（保证捕获到的值是在同一个时间基准下）
    EQep1Regs.QPOSMAX=pulse_count*10;       //位置计数器的最大值。可以根据电机一圈的脉冲数确定.这里10圈清零一次
    EQep1Regs.QEPCTL.bit.QPEN=1;        // QEP 计数器enable

    EQep1Regs.QCAPCTL.bit.UPPS=1;       // 1/2 for unit position  2次QCLK产生一个UPEVENT脉冲
    EQep1Regs.QCAPCTL.bit.CCPS=7;       // 1/128 for CAP clock
    EQep1Regs.QCAPCTL.bit.CEN=1;        // QEP Capture Enable
    //EQep1Regs.QEPCTL.bit.IEI=2;         //索引信号上升沿时初始化锁存位置计数器
    //EQep1Regs.QEPCTL.bit.SWI=1;//软件复位 eQEP 子系统的命令,产生一次索引脉冲来标记旋转位置或进行校准

    EQep2Regs.QDECCTL.bit.QSRC=00;      // QEP quadrature count mode 正交计数模式

    EQep2Regs.QEPCTL.bit.FREE_SOFT=2;
    EQep2Regs.QEPCTL.bit.PCRM=10;       // PCRM=10 mode - QPOSCNT reset on first index event      QPOSCNT(位置计数器)的时钟来源于QCLK
    EQep2Regs.QEPCTL.bit.UTE=1;         // Unit Timeout Enable
    EQep2Regs.QEPCTL.bit.QCLM=1;        // Latch on unit time out   单位时间事件发生时锁存数据。位置计数器值、捕获计时器和捕获周期值保存在QPOSLAT、QCTMRLAT和QCPRDLAT寄存器中（保证捕获到的值是在同一个时间基准下）
    EQep2Regs.QPOSMAX=pulse_count*10;       //位置计数器的最大值。可以根据电机一圈的脉冲数确定.这里10圈清零一次
    EQep2Regs.QEPCTL.bit.QPEN=1;        // QEP 计数器enable

    EQep2Regs.QCAPCTL.bit.UPPS=1;       // 1/2 for unit position  2次QCLK产生一个UPEVENT脉冲
    EQep2Regs.QCAPCTL.bit.CCPS=7;       // 1/128 for CAP clock
    EQep2Regs.QCAPCTL.bit.CEN=1;        // QEP Capture Enable

}

void POSSPEED_Calc(POSSPEED *p, int eqepNum)
{

     unsigned int newp,oldp;//用于M法计算速度的中间量

     p->DirectionQep = EQep1Regs.QEPSTS.bit.QDF;    //方向

     //定时测角                                                   //位置计数器的值锁存到QPOSLAT
     if ((eqepNum == 1 && EQep1Regs.QFLG.bit.UTO == 1) ||
             (eqepNum == 2 && EQep2Regs.QFLG.bit.UTO == 1))    // If unit timeout (one 100Hz period) T已知（0.03s），▲X = QPOSLAT（k）-QPOSLAT（k-1）
    {

        if(eqepNum == 1)
        {
            newp=(unsigned int)EQep1Regs.QPOSLAT;
        }
        else if(eqepNum == 2)
        {
            newp=(unsigned int)EQep2Regs.QPOSLAT;
        }
        oldp=p->oldpos;

        if (p->DirectionQep==0)                     // POSCNT is counting down，Tmp为负数
        {
            if (newp>oldp)
                p->Pos_increment = -EQep1Regs.QPOSMAX + newp - oldp;
            else
                p->Pos_increment = newp -oldp;
        }
        else if (p->DirectionQep==1)                // POSCNT is counting up
        {
            if (newp>=oldp)
                p->Pos_increment = newp - oldp;
            else
                p->Pos_increment = EQep1Regs.QPOSMAX - oldp + newp;                     // x2-x1 should be positive
        }
        p->Speed_fr = p->Pos_increment*p->BaseRpm;              //      r/min
        //_IQ15mpy(10000,p->BaseRpm_Q)是先将10000和p->BaseRpm_Q转化为long型，然后令两者相乘再除以2^15 (long最大值 4294967295)
        p->SpeedQ_fr= _IQ15mpy(p->Pos_increment*100,p->BaseRpm_Q); // 定点运算,Q15*Q0。这里得到真实值的100倍，防止小数点被省略,经测试，将p->Pos_increment*100中100改为10时，p->SpeedQ_fr的值时常出现跳变，难以用于pid调节
        // Update the electrical angle
        p->oldpos = newp;
        if(eqepNum == 1)
        {
            EQep1Regs.QCLR.bit.UTO=1;                   // Clear interrupt flag
        }
        else if(eqepNum == 2)
        {
            EQep2Regs.QCLR.bit.UTO=1;                   // Clear interrupt flag
        }
    }
}
