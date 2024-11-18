#ifndef HW_DEF_H_
#define HW_DEF_H_


#include "main.h"
#include "def.h"



// 하드웨어 정의 추가
//
#define _USE_HW_BUZZER


#define _USE_HW_LED
#define      HW_LED_MAX_CH          1

#define _USE_HW_UART
#define      HW_UART_MAX_CH         1

#define _USE_HW_CLI
#define      HW_CLI_CMD_LIST_MAX    32
#define      HW_CLI_CMD_NAME_MAX    16
#define      HW_CLI_LINE_HIS_MAX    8
#define      HW_CLI_LINE_BUF_MAX    64

#define _USE_HW_GPIO
#define      HW_GPIO_MAX_CH         4

#define _USE_HW_BUTTON
#define      HW_BUTTON_MAX_CH       7

#define _USE_HW_ADC
#define      HW_ADC_MAX_CH          2

#define _USE_HW_ST7789
#define      HW_ST7789_WIDTH        320
#define      HW_ST7789_HEIGHT       170

#define _USE_HW_LCD
#define      HW_LCD_WIDTH           300
#define      HW_LCD_HEIGHT          160



// 공통 사용 함수
//
#define logPrintf printf

void     delay(uint32_t ms);
uint32_t millis(void);


#endif
