#include "upper.h"

__interrupt void serialRxISR(void);
unsigned char recive_buffer[13];
Uint16 P_value,I_value,D_value;
WAVEFORM waveform;
WAVEFORM* waveformPtr = &waveform;

/*------------------------------重定向------------------------------------------*/
int fputc(int ch,FILE *fp)
{
    while(SciaRegs.SCICTL2.bit.TXRDY == 0);
    SciaRegs.SCITXBUF= ch;
   return ch;
}

int fputs(const char *_ptr,FILE *_fp)
{
    unsigned int i,len;
    len = strlen(_ptr);
    for(i=0;i<len;i++)
    {
        while(SciaRegs.SCICTL2.bit.TXRDY == 0);
        SciaRegs.SCITXBUF= (unsigned char)(_ptr[i]);
    }
    return len;
}

/*--------------------------------匿名上位机------------------------------------------*/

/*
    * @name  UART_Debug
    * @brief  以匿名协议的传输数据给上位机，在设置变量时候一定要调成int_16形式。
    * @param  这个是只对float变量写的，要求有3个变量speed_pid.pv,speed_pid.SEK,speed_pid.sv。
    * @retval None
    * 实现了s%,d%的打印
*/
void NIMING_Debug(PID *pp)
 {
    static int tempData_NM[14] = {0xAB,0xFE,0x05,0xF1,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    waveformPtr->p = pp;
    int sumcheck=0;
    int addcheck=0;
    int flen = tempData_NM[4]+tempData_NM[5]*256;
    tempData_NM[6] =((short)((waveformPtr->p->pv))&0xFF);
    tempData_NM[7] =((short)((waveformPtr->p->pv))>>8);
    tempData_NM[8] =((short)((waveformPtr->p->sv))&0xFF);
    tempData_NM[9] = ((short)((waveformPtr->p->sv))>>8);
    tempData_NM[10] =((short)((waveformPtr->p->OUT))&0xFF);
    tempData_NM[11] =((short)((waveformPtr->p->OUT))>>8);

     unsigned int i;

      for(i=0;i<(flen+6);i++)
       {
         sumcheck+=tempData_NM[i];
         addcheck+=sumcheck;
       }
       tempData_NM[12]=(int)sumcheck;
       tempData_NM[13]=(int)addcheck;

       for(i=0;i<14;i++)
         {
           UARTa_SendByte(tempData_NM[i]);
         }


 }
/*--------------------------------scia函数-------------------------------------------*/
void UARTa_Init(Uint32 baud)
{
    unsigned char scihbaud=0;
    unsigned char scilbaud=0;
    Uint16 scibaud=0;

    //根据函数参数baud设置的波特率计算存储波特率生成器的高8位和低8位
    scibaud=37500000/(8*baud)-1;//通信时钟37.5MHZ
    scihbaud=scibaud>>8;
    scilbaud=scibaud&0xff;


    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.SCIAENCLK = 1;   // SCI-A
    EDIS;

    InitSciaGpio();

    //Initalize the SCI FIFO
    SciaRegs.SCIFFRX.all=0x0028;//使能中断，接收到8位数据(0~4位)产生中断
    SciaRegs.SCIFFTX.all=0xE040; //不使能中断(第5位)
    SciaRegs.SCIFFCT.all=0x0;//不使用FIFO传输延时

    // Note: Clocks were turned on to the SCIA peripheral
    // in the InitSysCtrl() function
    SciaRegs.SCICCR.all =0x0007;   // 1 stop bit(第7位),  No loopback(第4位)， No parity(无奇偶校验)(第5位),
                                    //8 char bits(0~2位),async mode, idle-line protocol(第3位)(传输模式选择：空闲线模式)
    SciaRegs.SCICTL1.all =0x0003;  // enable TX(第0位), RX(第1位), internal SCICLK,
                                   // Disable RX ERR, SLEEP(第2位), TXWAKE(第3位)(传输特性选择)
    SciaRegs.SCICTL2.all =0x0003;//等效下面两行
    //SciaRegs.SCICTL2.bit.TXINTENA =1;//发送缓冲区(SCITFBUF)中断(TXRDY flag)
    //SciaRegs.SCICTL2.bit.RXBKINTENA =1;//启用接收缓冲区中断(RXRDY flag和 RKDT flag)
    SciaRegs.SCIHBAUD    =scihbaud;  // 115200 baud @LSPCLK = 37.5MHz.
    SciaRegs.SCILBAUD    =scilbaud;
//  SciaRegs.SCICCR.bit.LOOPBKENA =1; // Enable loop back,回环测试模式
// 配置串口接收中断
    EALLOW;
    PieVectTable.SCIRXINTA = &serialRxISR;
//    PieVectTable.SCITXINTA = &sciaTxFifoIsr;
    EDIS;
    PieCtrlRegs.PIEIER9.bit.INTx1=1;
    IER |= M_INT9;
    SciaRegs.SCICTL1.all =0x0023;     // Relinquish SCI from Reset 开始通信，禁用了睡眠模式和接收错误中断，但启用了SCI模块的发送中断和接收中断
    SciaRegs.SCIFFTX.bit.TXFIFOXRESET=1;
    SciaRegs.SCIFFRX.bit.RXFIFORESET=1;
}


void SCIa_SendByte(int dat)
{
    while (SciaRegs.SCIFFTX.bit.TXFFST != 0);
    SciaRegs.SCITXBUF = dat;
}

// Transmit a character from the SCI'
void UARTa_SendByte(int a)
{
    while (SciaRegs.SCIFFTX.bit.TXFFST != 0);
    SciaRegs.SCITXBUF=a;
}

void UARTa_SendString(char * msg)
{
    int i=0;

    while(msg[i] != '\0')
    {
        UARTa_SendByte(msg[i]);
        i++;
    }
}

/*------------------------------上位机设置PID值-----------------------------*/
__interrupt void serialRxISR(void)
{
    static Uint16 Index = 0;

    // 读取接收数据
    Uint16 From_UPPer = SciaRegs.SCIRXBUF.all;
    if((Index==0&&From_UPPer==HEADER_1)||
            (Index==1&&From_UPPer==HEADER_2)||
                (Index>=2&&Index<=5))
    {
        Index++;
    }
    else{Index=0;P_value=0;I_value=0;D_value=0;}
    if(Index==3)
    {
        P_value=From_UPPer;
    }
    if(Index==4)
    {
        I_value=From_UPPer;
    }
    if(Index==5)
    {
        D_value=From_UPPer;
    }
    if(Index==6)
    {
        if(From_UPPer==0x01)
        {
            pidParams1.P= recive_buffer[2]/1000;
            pidParams1.I= recive_buffer[3]/1000;
            pidParams1.D= recive_buffer[4]/1000;
            PID_Init(&speed_pid1,&pidParams1,1500.0f);
            Index=0;
        }

        else if(From_UPPer==0x02)
        {
            pidParams2.P= (float)recive_buffer[2]/1000;
            pidParams2.I= (float)recive_buffer[3]/1000;
            pidParams2.D= (float)recive_buffer[4]/1000;
            PID_Init(&speed_pid2,&pidParams2,1500.0f);
            Index=0;
        }
        else
        {
            Index=0;
        }
    }

    SciaRegs.SCIFFRX.bit.RXFFINTCLR=1;//清除中断标志
    // 清除中断标志
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP9;
}
