#include "time.h"
#include "leds.h"

//��ʱ��1��ʼ������
//Freq��CPUʱ��Ƶ�ʣ�150MHz��
//Period����ʱ����ֵ����λus
/*********��ʱ��1�������************/
Uint32 Systime=0;
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

    //���ö�ʱ��1���ж���ڵ�ַΪ�ж��������INT13
    EALLOW;
    PieVectTable.XINT13 = &TIM1_IRQn;
    EDIS;

    //ָ��ʱ��1�ļĴ�����ַ
    CpuTimer1.RegsAddr = &CpuTimer1Regs;
    //���ö�ʱ��1�����ڼĴ���ֵ
    CpuTimer1Regs.PRD.all  = 0xFFFFFFFF;
    //���ö�ʱ��Ԥ���������ֵΪ0
    CpuTimer1Regs.TPR.all  = 0;
    CpuTimer1Regs.TPRH.all = 0;
    //ȷ����ʱ��1Ϊֹͣ״̬
    CpuTimer1Regs.TCR.bit.TSS = 1;
    //����ʹ��
    CpuTimer1Regs.TCR.bit.TRB = 1;
    // Reset interrupt counters:
    CpuTimer1.InterruptCount = 0;

    ConfigCpuTimer(&CpuTimer1, Freq, Period);

    //��ʼ��ʱ������
    CpuTimer1Regs.TCR.bit.TSS=0;
    //��ʱ��1ֱ�������ں��жϣ���������PIE
    IER |= M_INT13;

    //ʹ�����ж�
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
    CpuTimer1Regs.TCR.bit.TSS=0;//������ʱ��
    EDIS;
}


