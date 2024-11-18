#include "hw.h"




bool hwInit(void)
{
  // 하드웨어 초기화 함수
  //
  cliInit();
  ledInit();
  uartInit();
  buzzerInit();
  gpioInit();
  buttonInit();
  adcInit();
  st7789Init();
  lcdInit();

  return true;
}

void delay(uint32_t ms)
{
  HAL_Delay(ms);
}

uint32_t millis(void)
{
  return HAL_GetTick();
}
