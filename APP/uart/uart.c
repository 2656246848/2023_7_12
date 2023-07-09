#include "uart.h"
#include "stdio.h"
#include "string.h"
#include "leds.h"
/*
    * @name   fputc,fputs
    * @brief  串口打印重定向
    * @param  None
    * @retval None
    * 实现了s%,d%的打印
*/

__interrupt void seriblRxISR(void);
// 定义接收缓冲区
volatile Uint16 rxBuffer[BUFFER_SIZE];
Uint16 rdataA[8];    // Received data for SCI-A
Uint16 rdataB[8];    // Received data for SCI-A
volatile Uint16 r_value,l_value,r_x,l_x;


int fputc(int ch,FILE *fp)
{
    while(SciaRegs.SCICTL2.bit.TXRDY == 0);
    SciaRegs.SCITXBUF= ch;
   return ch;
}

//重定向打印字符串
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
    SciaRegs.SCIFFTX.all=0xE040;//不使能中断(第5位)，清除中断标志位(第6位)，清空FIFO数据(第13位)，使用FIFO(第14位)，SCI FIFO可以恢复接受或发送(第15位);
                                //FIFO的数据个数为0(第8~12位);FIFO的数据个数<=0产生中断(0~4位)
    SciaRegs.SCIFFRX.all=0x204f; //不使能中断(第5位)，清除中断标志位(第6位)，清除RXFFOVF(接收溢出)标志位(第14位)，复位FIFO指针(第13位)
                                 //FIFO的数据个数为0(第8~12位);FIFO的数据个数>=15产生中断(0~4位)
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
    SciaRegs.SCICTL1.all =0x0023;     // Relinquish SCI from Reset 开始通信，禁用了睡眠模式和接收错误中断，但启用了SCI模块的发送中断和接收中断

}

void UARTb_Init(Uint32 baud)
{
    unsigned char scihbaud=0;
    unsigned char scilbaud=0;
    Uint16 scibaud=0;

    //根据函数参数baud设置的波特率计算存储波特率生成器的高8位和低8位
    scibaud=37500000/(8*baud)-1;//通信时钟37.5MHZ
    scihbaud=scibaud>>8;
    scilbaud=scibaud&0xff;


    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.SCIBENCLK = 1;   // SCI-B
    EDIS;

    InitScibGpio();

    //Initalize the SCI FIFO
//    ScibRegs.SCIFFTX.all=0xE040;//不使能中断(第5位)，清除中断标志位，清空FIFO数据，重置SCI的接收和发送通道
//    ScibRegs.SCIFFRX.all=0x204f; //
    ScibRegs.SCIFFTX.all=0xC028;//使能中断，接收到8位数据(0~4位)产生中断
    ScibRegs.SCIFFRX.all=0x0028;//使能中断，发送8位产生中断
    ScibRegs.SCIFFCT.all=0x0;

    // Note: Clocks were turned on to the SCIA peripheral
    // in the InitSysCtrl() function
    ScibRegs.SCICCR.all =0x0007;   // 1 stop bit,  No loopback
                                   // No parity,8 char bits,
                                   // async mode, idle-line protocol
    ScibRegs.SCICTL1.all =0x0003;  // enable TX, RX, internal SCICLK,
                                   // Disable RX ERR, SLEEP, TXWAKE
    ScibRegs.SCICTL2.all =0x0003;
    ScibRegs.SCICTL2.bit.TXINTENA =1;
    ScibRegs.SCICTL2.bit.RXBKINTENA =1;
    // 配置串口接收中断
    EALLOW;
    PieVectTable.SCIRXINTB = &seriblRxISR;
//    PieVectTable.SCITXINTB = &scibTxFifoIsr;
    EDIS;
    PieCtrlRegs.PIEIER9.bit.INTx3=1;
    IER |= M_INT9;
    ScibRegs.SCIHBAUD    =scihbaud;  // 9600 baud @LSPCLK = 37.5MHz
    ScibRegs.SCILBAUD    =scilbaud;
//  SciaRegs.SCICCR.bit.LOOPBKENA =1; // Enable loop back
    ScibRegs.SCICTL1.all =0x0023;     // Relinquish SCI from Reset 开始通信，禁用了睡眠模式和接收错误中断，但启用了SCI模块的发送中断和接收中断
    ScibRegs.SCIFFTX.bit.TXFIFOXRESET=1;
    ScibRegs.SCIFFRX.bit.RXFIFORESET=1;
}

void SCIa_SendByte(int dat)
{
    while (SciaRegs.SCIFFTX.bit.TXFFST != 0);
    SciaRegs.SCITXBUF = dat;
}

void SCIb_SendByte(int dat)
{
    while (ScibRegs.SCIFFTX.bit.TXFFST != 0);
    ScibRegs.SCITXBUF = dat;
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

void UARTb_SendByte(int a)
{
    while (ScibRegs.SCIFFTX.bit.TXFFST != 0);
    ScibRegs.SCITXBUF=a;
}

void UARTb_SendString(char * msg)
{
    int i=0;

    while(msg[i] != '\0')
    {
        UARTb_SendByte(msg[i]);
        i++;
    }
}

__interrupt void seriblRxISR(void)
{
    static Uint16 bufferIndex = 0;
        static Uint16 Index2,Index3,Index4,Index5;

        // 读取接收数据
        Uint16 receivedData = ScibRegs.SCIRXBUF.all;
        if((bufferIndex==0&&receivedData==FRAME_HEADER_1)||
                (bufferIndex==1&&receivedData==FRAME_HEADER_2)||
                    (bufferIndex>=2&&bufferIndex<=5)||
                        (bufferIndex==6 && receivedData==Index2*16+Index3+Index4-Index5)||
                             (bufferIndex==7&&receivedData==FRAME_TAIL))
        {
            bufferIndex++;
        }
        else{bufferIndex=0;r_value=0;l_value=0;}
        if(bufferIndex==3)
        {
            Index2=receivedData;
        }
        if(bufferIndex==4)
        {
            Index3=receivedData;
        }
        if(bufferIndex==5)
        {
            Index4=receivedData;
        }
        if(bufferIndex==6)
        {
            Index5=receivedData;
        }
        if(bufferIndex==8&&receivedData==FRAME_TAIL)
        {
            l_value=Index2;
            l_x=Index3;
            r_value=Index4;
            r_x=Index5;
            bufferIndex=0;
        }

        ScibRegs.SCIFFRX.bit.RXFFINTCLR=1;//清除中断标志
        // 清除中断标志
        PieCtrlRegs.PIEACK.all |= PIEACK_GROUP9;
    }
