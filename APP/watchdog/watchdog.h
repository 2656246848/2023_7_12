#ifndef APP_WATCHDOG_WATCHDOG_H_
#define APP_WATCHDOG_WATCHDOG_H_

#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"   // DSP2833x Examples Include File
#include "eqep.h"
void WatchDog_init();
__interrupt void wakeint_isr(void);


#endif /* APP_WATCHDOG_WATCHDOG_H_ */
