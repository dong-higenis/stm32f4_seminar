#ifndef ST7789_H_
#define ST7789_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hw_def.h"


#ifdef _USE_HW_ST7789


#include "st7789_regs.h"



enum class_color
{
 white     = 0xFFFF,
 gray      = 0x8410,
 darkgray  = 0xAD55,
 black     = 0x0000,
 purple    = 0x8010,
 pink      = 0xFE19,
 red       = 0xF800,
 orange    = 0xFD20,
 brown     = 0xA145,
 beige     = 0xF7BB,
 yellow    = 0xFFE0,
 lightgreen= 0x9772,
 green     = 0x07E0,
 darkblue  = 0x0011,
 blue      = 0x001F,
 lightblue = 0xAEDC,
};

typedef struct
{
  uint32_t req_time;
} st7789_info_t;


bool st7789Init(void);
bool st7789IsReady(void);
void st7789GetInfo(st7789_info_t *p_info);
void st7789SetWindow(int32_t x, int32_t y, int32_t w, int32_t h);
bool st7789SendBuffer(uint8_t *p_data, uint32_t length, uint32_t timeout_ms);

uint16_t st7789GetWidth(void);
uint16_t st7789GetHeight(void);

void st7789FillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color);

#endif

#ifdef __cplusplus
}
#endif

#endif
