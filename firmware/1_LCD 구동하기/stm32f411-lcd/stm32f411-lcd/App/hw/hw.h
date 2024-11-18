#ifndef HW_H_
#define HW_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hw_def.h"

#include "led.h"
#include "uart.h"
#include "cli.h"
#include "buzzer.h"
#include "gpio.h"
#include "button.h"
#include "adc.h"
#include "st7789.h"
#include "lcd.h"


bool hwInit(void);


#ifdef __cplusplus
}
#endif

#endif
