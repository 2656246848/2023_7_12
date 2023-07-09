/*
 * speed.h
 *
 *  Created on: 2023年4月16日
 *      Author: gy
 */
#ifndef SPEED_H_
#define SPEED_H_

#include "IQmathLib.h"         // Include header for IQmath library

//电机物理信息:线数、减速比
#define lin_number      13
#define raduction_ratio     20 //减速比
#define pulse_count     1040    //一圈位置计数器QPOSCNT加1040
#define speed_method        2
#define speed_method_T      1
#define speed_method_M      2
#define sampling_T      5000      //采样周期1ms（1000us）

#if (CPU_FRQ_150MHZ)
  #define CPU_CLK   150e6
#endif
#if (CPU_FRQ_100MHZ)
  #define CPU_CLK   100e6
#endif

typedef struct {

                int DirectionQep;       // Output: Motor rotation direction (Q0)
                float Speed_pr;           // Output :  speed in per-unit
                float BaseRpm;         // BaseRpm=60/(1040*0.01)r/min
                _iq BaseRpm_Q;         //BaseRpm_Q=60*2^15=189046  (Q15)
                int32 SpeedRpm_pr;      // Output : speed in r.p.m. (Q0) - independently with global Q

                int Pos_increment;
                int  oldpos;            // Input: Electrical angle (pu)
                float Speed_fr;           // Output :  speed in per-unit
                _iq SpeedQ_fr;      // Output : Speed in rpm  (Q0) - independently with global Q
                 int temp1;
                //float average;
                void (*init)();         // Pointer to the init funcion
                void (*calc)();         // Pointer to the calc funtion
                }  POSSPEED;

/*-----------------------------------------------------------------------------
Define a POSSPEED_handle
-----------------------------------------------------------------------------*/
typedef POSSPEED *POSSPEED_handle;

/*-----------------------------------------------------------------------------
Default initializer for the POSSPEED Object.
-----------------------------------------------------------------------------*/
/*这里“\”用于连接多行代码*/
#if (CPU_FRQ_150MHZ)
  #define POSSPEED_DEFAULTS {0x0,0,57.6923076923/5,189046,0,\
        0,0,0,0,0,\
        (void (*)(long))POSSPEED_Init,\
        (void (*)(long))POSSPEED_Calc }
#endif

void POSSPEED_Init(void);
void POSSPEED_Calc(POSSPEED_handle, int eqepNum);
extern POSSPEED qep_posspeed;
extern POSSPEED qep_posspeed2;


#endif /* APP_EQEP_SPEED_H_ */

