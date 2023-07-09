#ifndef TIME_H_
#define TIME_H_


#include "DSP2833x_Device.h"     // DSP2833x 头文件
#include "DSP2833x_Examples.h"   // DSP2833x 例子相关头文件

extern unsigned int Timer_10ms_Task;
extern unsigned int Timer_50ms_Task;
extern unsigned int Timer_100ms_Task;
extern unsigned int Timer_1s_Task;

//void TIM0_Init(float Freq, float Period);
//interrupt void TIM0_IRQn(void);

void TIM1_Init(float Freq, float Period);
interrupt void TIM1_IRQn(void);




#endif /* TIME_H_ */
