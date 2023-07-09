#include "watchdog.h"
__interrupt void wakeint_isr(void);
extern volatile Uint16 va;
extern volatile Uint16 va2;

/*看门狗计数周期Timeout period = 1 / (OSCCLK / (512 * prescaler * 256) ).
 * OSCCLK ：晶振频率：30MHZ
 * prescaler：预分频系数：WDCR的0~2位决定
 * 时钟周期数:Timeout period/150M
 * 计数器计满后溢出可产生输出512个OSCCLK时钟的脉冲信号。该信号作为复位信号WDRST还是中断信号WDINT由SCSR的WDENNIT(SCSR的第1位)决定。
*/
void WatchDog_init()
{
       GpioDataRegs.GPASET.bit.GPIO0=1;
       ServiceDog();
       EALLOW;
       PieVectTable.WAKEINT = &wakeint_isr; //指定中断入口函数
       EDIS;

       PieCtrlRegs.PIECTRL.bit.ENPIE = 1;      // Enable the PIE block
       PieCtrlRegs.PIEIER1.bit.INTx8 = 1;      // Enable PIE Group 1 INT8
       IER |= M_INT1;                          // Enable CPU int1
       EINT;                                   // Enable Global Interrupts

       // Reset the watchdog counter
       ServiceDog();

       // Enable the watchdog
       EALLOW;
       //不分频时看门狗中断(或系统复位)响应时间最短，为1/(30M/512)*2^8S=1/(30/512)*0.256mS=4.37ms
       //当WDPS为111时，64分频。门狗中断响应时间最长，1/(30M/512/64)*2^8S=1/(30/512/64)*0.256mS=279.62ms
       SysCtrlRegs.WDCR = 0x002F;//看门狗控制寄存器（16位）：64分频(0-2位置111);WDCHK为101(3-5位：101);使能看门狗(第6位置0);看门狗复位标志位清零(第七位置0)
       //SysCtrlRegs.WDCR = 0x0028;//0分频
       SysCtrlRegs.SCSR = 0x0002;//使能看门狗中断(第2位为0);，第一位写1表示计数器计满后产生中断信号而非复位信号,第0位(WDOVERRIDE)写入1后将不能配置控制寄存器WDCR的WDDI(看门狗失能位)
       EDIS;
       GpioDataRegs.GPACLEAR.bit.GPIO0=1;
       //DisableDog();禁用看门狗
}

// wakeint_isr -
//
__interrupt void wakeint_isr(void)
{
    va = 0;
    va2 = 0;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;

}
