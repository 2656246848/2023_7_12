#ifndef APP_UPPER_UPPER_H_
#define APP_UPPER_UPPER_H_

#include "PID.h"
#include "stdio.h"
#include "string.h"

#define HEADER_1    0x55
#define HEADER_2    0xab

void PID_Change(unsigned char ucData,PIDConstants* p);
void SCIa_SendByte(int dat);
void UARTa_Init(Uint32 baud);
void UARTa_SendByte(int a);
void UARTa_SendString(char * msg);
int fputs(const char *_ptr,FILE *_fp);
int fputc(int ch,FILE *fp);
__interrupt void serialRxISR(void);

#endif /* APP_UPPER_UPPER_H_ */
