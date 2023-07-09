/*
 * eqep.c
 *
 *  Created on: 2023年4月16日
 *      Author: gy
 */
#include "eqep.h"

interrupt void prdTick(void);

void EPwm1Setup(void)
{
    InitEPwm1Gpio();

    EALLOW;
    GpioCtrlRegs.GPADIR.bit.GPIO4 = 1;    // GPIO4 as output simulates Index signal
    GpioDataRegs.GPACLEAR.bit.GPIO4 = 1;  // Normally low
    EDIS;

    EPwm1Regs.TBSTS.all=0;
    EPwm1Regs.TBPHS.half.TBPHS =0;
    EPwm1Regs.TBCTR=0;

    EPwm1Regs.CMPCTL.all=0x50;     // immediate mode for CMPA and CMPB
    EPwm1Regs.CMPA.half.CMPA=0;
    EPwm1Regs.CMPB=0;

    EPwm1Regs.AQCTLA.all=0x90;     // CTR=CMPA when inc->EPWM1A=0, when dec->EPWM1A=1
    EPwm1Regs.AQCTLB.bit.CBD=2;
    EPwm1Regs.AQCTLB.bit.CBU=1;
    EPwm1Regs.AQSFRC.all=0;
    EPwm1Regs.AQCSFRC.all=0;

    EPwm1Regs.TZSEL.all=0;
    EPwm1Regs.TZCTL.all=0;
    EPwm1Regs.TZEINT.all=0;
    EPwm1Regs.TZFLG.all=0;
    EPwm1Regs.TZCLR.all=0;
    EPwm1Regs.TZFRC.all=0;

    EPwm1Regs.ETSEL.all=0x0A;      // Interrupt on PRD
    EPwm1Regs.ETPS.all=1;
    EPwm1Regs.ETFLG.all=0;
    EPwm1Regs.ETCLR.all=0;
    EPwm1Regs.ETFRC.all=0;

    EPwm1Regs.PCCTL.all=0;

    EPwm1Regs.TBCTL.all=0x0010+TBCTLVAL; // Enable Timer
    EPwm1Regs.TBPRD=SP;//SP=15000

    EALLOW;  // This is needed to write to EALLOW protected registers
    PieVectTable.EPWM1_INT= &prdTick;
    EDIS;

    IER |= M_INT3;
    PieCtrlRegs.PIEIER3.bit.INTx1 = 1;
    EINT;   // Enable Global interrupt INTM
    ERTM;   // Enable Global realtime interrupt DBGM

}



void EQEP1_Init(void)
{
    EALLOW;  // This is needed to write to EALLOW protected registers
    SysCtrlRegs.PCLKCR1.bit.EQEP1ENCLK = 1;  // eQEP1
    EDIS;

    InitEQep1Gpio();
    InitEQep2Gpio();
    EPwm1Setup();

    qep_posspeed.init(&qep_posspeed);         //init是结构体类型POSSPEED里的一个函数指针，qep_posspeed是一个POSSPEED类型的结构体，qep_posspeed的内容被初始化为POSSPEED_DEFAULTS，包括将void (*init)()初始化为(void (*)(long))POSSPEED_Init,所以这里是执行了POSSPEED_Init(&qep_posspeed)
}


void EPwm1A_SetCompare(Uint16 val)
{
    EPwm1Regs.CMPA.half.CMPA = val;  //设置占空比
}
void EPwm1B_SetCompare(Uint16 val)
{
    EPwm1Regs.CMPB = val;  //设置占空比
}

interrupt void prdTick(void)                  // EPWM1 Interrupts once every 4 QCLK counts (one period)
{
    // Position and Speed measurement
    qep_posspeed.calc(&qep_posspeed,1);
    qep_posspeed2.calc(&qep_posspeed2,2);

    // Acknowledge this interrupt to receive more interrupts from group 1
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
    EPwm1Regs.ETCLR.bit.INT=1;
}


