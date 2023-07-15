#include "upper.h"
//#include <stdio.h>
//#include "stdlib.h"
//#include <stddef.h>
__interrupt void serialRxISR(void);
unsigned int recive_buffer[6];
Uint16 P_value,I_value,D_value;
float tempData_NM[]={};//Ҫ������λ��������

size_t dataSize = 3; // ����Ҫ���͵���������
/*------------------------------�ض���------------------------------------------*/
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

/*--------------------------------������λ��------------------------------------------*/

/*
    * @name  UART_Debug
    * @brief  ������Э��Ĵ������ݸ���λ���������ñ���ʱ��һ��Ҫ����int_16��ʽ��
    * @param  �����ֻ��float����д�ģ�Ҫ����3������
    * @retval None
*/
void NIMING_Debug(const float* data, size_t dataSize) {
    int sumcheck = 0;
    int addcheck = 0;
    int flen = dataSize * 2;
    tempData_NM[0] = 0xAB;
    tempData_NM[1] = 0xFE;
    tempData_NM[2] = 0x05;
    tempData_NM[3] = 0xF1;
    tempData_NM[4] = flen & 0xFF;
    tempData_NM[5] = (flen >> 8) & 0xFF;

    Uint16 i;
    for (i = 0; i < dataSize; i++) {
        tempData_NM[6 + i * 2] = (int)data[i] & 0xFF;
        tempData_NM[7 + i * 2] = ((int)data[i] >> 8) & 0xFF;
    }

    for (i = 0; i < (flen + 6); i++) {
        sumcheck += tempData_NM[i];
        addcheck += sumcheck;
    }
    tempData_NM[6 + flen] = (int)sumcheck;
    tempData_NM[7 + flen] = (int)addcheck;

    for (i = 0; i < 8 + flen; i++) {
        UARTa_SendByte(tempData_NM[i]);
    }
}

/*--------------------------------scia����-------------------------------------------*/
void UARTa_Init(Uint32 baud)
{
    unsigned char scihbaud=0;
    unsigned char scilbaud=0;
    Uint16 scibaud=0;

    //���ݺ�������baud���õĲ����ʼ���洢�������������ĸ�8λ�͵�8λ
    scibaud=37500000/(8*baud)-1;//ͨ��ʱ��37.5MHZ
    scihbaud=scibaud>>8;
    scilbaud=scibaud&0xff;


    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.SCIAENCLK = 1;   // SCI-A
    EDIS;

    InitSciaGpio();

    //Initalize the SCI FIFO
    SciaRegs.SCIFFRX.all=0x0028;//ʹ���жϣ����յ�8λ����(0~4λ)�����ж�
    SciaRegs.SCIFFTX.all=0xE040; //��ʹ���ж�(��5λ)
    SciaRegs.SCIFFCT.all=0x0;//��ʹ��FIFO������ʱ

    // Note: Clocks were turned on to the SCIA peripheral
    // in the InitSysCtrl() function
    SciaRegs.SCICCR.all =0x0007;   // 1 stop bit(��7λ),  No loopback(��4λ)�� No parity(����żУ��)(��5λ),
                                    //8 char bits(0~2λ),async mode, idle-line protocol(��3λ)(����ģʽѡ�񣺿�����ģʽ)
    SciaRegs.SCICTL1.all =0x0003;  // enable TX(��0λ), RX(��1λ), internal SCICLK,
                                   // Disable RX ERR, SLEEP(��2λ), TXWAKE(��3λ)(��������ѡ��)
    SciaRegs.SCICTL2.all =0x0003;//��Ч��������
    //SciaRegs.SCICTL2.bit.TXINTENA =1;//���ͻ�����(SCITFBUF)�ж�(TXRDY flag)
    //SciaRegs.SCICTL2.bit.RXBKINTENA =1;//���ý��ջ������ж�(RXRDY flag�� RKDT flag)
    SciaRegs.SCIHBAUD    =scihbaud;  // 115200 baud @LSPCLK = 37.5MHz.
    SciaRegs.SCILBAUD    =scilbaud;
//  SciaRegs.SCICCR.bit.LOOPBKENA =1; // Enable loop back,�ػ�����ģʽ
// ���ô��ڽ����ж�
    EALLOW;
    PieVectTable.SCIRXINTA = &serialRxISR;
//    PieVectTable.SCITXINTA = &sciaTxFifoIsr;
    EDIS;
    PieCtrlRegs.PIEIER9.bit.INTx1=1;
    IER |= M_INT9;
    SciaRegs.SCICTL1.all =0x0023;     // Relinquish SCI from Reset ��ʼͨ�ţ�������˯��ģʽ�ͽ��մ����жϣ���������SCIģ��ķ����жϺͽ����ж�
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

/*------------------------------��λ������PIDֵ-----------------------------*/
__interrupt void serialRxISR(void)
{
    static Uint16 Index = 0;

    // ��ȡ�������� ��λ������ 0x55 0xAB Pֵ Iֵ Dֵ 0x02(��0x01)
    Uint16 From_UPPer = SciaRegs.SCIRXBUF.all;
    if((Index==0&&From_UPPer==HEADER_1)||
            (Index==1&&From_UPPer==HEADER_2)||
                (Index>=2&&Index<=5))
    {
        recive_buffer[Index] = From_UPPer;
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
            pidParams2.P=1;
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

    SciaRegs.SCIFFRX.bit.RXFFINTCLR=1;//����жϱ�־
    // ����жϱ�־
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP9;
}
