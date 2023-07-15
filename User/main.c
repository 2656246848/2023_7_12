#include "main.h"
#include "INIT.h"
#include "stdlib.h"

void main()
 {
    #if RUN_TYPE==FLASH_RUN
        MemCopy(&RamfuncsLoadStart, &RamfuncsLoadEnd, &RamfuncsRunStart);
        InitFlash();
    #endif
    SYSTERM_INIT();
    if(0)
    {
        OLED_DisShowCHinese(56,5,2);
        OLED_DisShowCHinese(74,5,3);
        OLED_DisShowCHinese(92,5,4);
        OLED_ShowString(0,0,"freq:150kHz",16);
        OLED_ShowString(90,0,"IO0",16);
        OLED_ShowString(0,2,"duty:",16);
        OLED_ShowNum(70,2,va/150,2,2,16);
        OLED_ShowString(0,4,"K1/K2",16);
        OLED_ShowString(0,6,"10/1",16);
    }

    #if RUN_TYPE==SRAM_RUN
        // 正常
        printf("Hello World\n");
        //PID_Change(ReceivedChar);
    #endif

    while (1)
    {
        if(Timer_5ms_Task==1){
            Timer_5ms_Task=0;
            //MPU_RUN_CIRCULATION();
            MPU6050_Read_All(&MPU6050);
        }

        if(Timer_10ms_Task==1) //10ms扫描一次
        {
            Timer_10ms_Task=0;
            #if PC_Communication_Mode == VOFA
                //printf("%d,%d,%d,%d\n",(int)(speed_pid.pv),(int)speed_pid.sv,(int)speed_pid.OUT,va);
                printf("%d,%d,%d,%d\n",(int)(speed_pid2.pv),(int)speed_pid2.sv,(int)speed_pid2.OUT,va2);
            #endif
            #if PC_Communication_Mode==NIMING
                float* sentdata = NULL;
                size_t dataSize = 3;//修改发送数量
                sentdata = (float*)malloc(dataSize * sizeof(float));
                if (sentdata == NULL) {
                        printf("error\n");
                    }
                // 修改发送内容
                sentdata[0] = speed_pid2.pv;
                sentdata[1] = speed_pid2.sv;
                sentdata[2] = speed_pid2.OUT;
                //sentdata[3] = 2.0;
                // 调用发送数据函数
                NIMING_Debug(sentdata,dataSize);
                free(sentdata);// 释放动态分配的内存
            #endif
        }
        if(Timer_50ms_Task==1) //50ms扫描一次
        {
            Timer_50ms_Task=0;
            ServiceDog();//喂狗，每279.62ms/4.37ms内要喂一次，否则进看门狗中断
        }
        if(Timer_1s_Task==1)   //1s扫描一次
        {
            Timer_1s_Task=0;
            LED2_TOGGLE;
        }
    }

}
