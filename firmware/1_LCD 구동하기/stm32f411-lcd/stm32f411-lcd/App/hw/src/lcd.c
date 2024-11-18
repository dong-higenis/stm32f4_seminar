#include "lcd.h"



#ifdef _USE_HW_LCD
#include "gpio.h"
#include "cli.h"
#include "lcd/lcd_fonts.h"
#include "lcd/han.h"


#define _PIN_DEF_BL     0


#define _swap_int16_t(a, b)     { int16_t t = a; a = b; b = t; }
#define MAKECOL(r, g, b)        ( ((r)<<11) | ((g)<<5) | (b))


typedef struct
{
  int16_t x;
  int16_t y;
} lcd_pixel_t;


static bool is_init = false;
static uint8_t backlight_value = 100;
static bool lcd_request_draw = false;

static uint16_t draw_idx = 0;
static uint16_t *draw_buf = NULL;
static uint16_t frame_buffer[1][HW_LCD_WIDTH * HW_LCD_HEIGHT];
static uint16_t font_src_buffer[16 * 16];

static LcdFont lcd_font = LCD_FONT_HAN;
static lcd_font_t *font_tbl[LCD_FONT_MAX] = { &font_07x10, &font_11x18, &font_16x26, &font_hangul};

static void disHanFont(int x, int y, han_font_t *FontPtr, uint16_t textcolor);
static void disEngFont(int x, int y, char ch, lcd_font_t *font, uint16_t textcolor);
static void lcdDrawLineBuffer(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, lcd_pixel_t *line);


#ifdef _USE_HW_CLI
static void cliLcd(cli_args_t *args);
#endif





bool lcdInit(void)
{
  backlight_value = 100;


  is_init = st7789Init();


  memset(frame_buffer[0], 0x00, LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t));

  lcdSetBackLight(100);

  draw_buf = frame_buffer[draw_idx];

  if (is_init != true)
  {
    return false;
  }

#ifdef _USE_HW_CLI
  cliAdd("lcd", cliLcd);
#endif

  return true;
}

bool lcdIsInit(void)
{
  return is_init;
}

void lcdReset(void)
{
}

uint8_t lcdGetBackLight(void)
{
  return backlight_value;
}

void lcdSetBackLight(uint8_t value)
{
  value = constrain(value, 0, 100);

  if (value != backlight_value)
  {
    backlight_value = value;
  }


  if (backlight_value > 0)
  {
    gpioPinWrite(_PIN_DEF_BL, _DEF_HIGH);
  }
  else
  {
    gpioPinWrite(_PIN_DEF_BL, _DEF_LOW);
  }
}

uint32_t lcdReadPixel(uint16_t x_pos, uint16_t y_pos)
{
  return draw_buf[y_pos * LCD_WIDTH + x_pos];
}

void lcdDrawPixel(int16_t x_pos, int16_t y_pos, uint32_t rgb_code)
{
  if (x_pos < 0 || x_pos >= LCD_WIDTH) return;
  if (y_pos < 0 || y_pos >= LCD_HEIGHT) return;

  draw_buf[y_pos * LCD_WIDTH + x_pos] = rgb_code;
}

void lcdClear(uint32_t rgb_code)
{
  lcdClearBuffer(rgb_code);

  lcdUpdateDraw();
}

void lcdClearBuffer(uint32_t rgb_code)
{
  uint16_t *p_buf = draw_buf;

  for (int i=0; i<LCD_WIDTH * LCD_HEIGHT; i++)
  {
    p_buf[i] = rgb_code;
  }
}

uint32_t lcdGetDrawTime(void)
{
  st7789_info_t info;

  st7789GetInfo(&info);

  return info.req_time;
}

void lcdDrawFillCircle(int32_t x0, int32_t y0, int32_t r, uint16_t color)
{
  int32_t  x  = 0;
  int32_t  dx = 1;
  int32_t  dy = r+r;
  int32_t  p  = -(r>>1);


  lcdDrawHLine(x0 - r, y0, dy+1, color);

  while(x<r)
  {

    if(p>=0) {
      dy-=2;
      p-=dy;
      r--;
    }

    dx+=2;
    p+=dx;

    x++;

    lcdDrawHLine(x0 - r, y0 + x, 2 * r+1, color);
    lcdDrawHLine(x0 - r, y0 - x, 2 * r+1, color);
    lcdDrawHLine(x0 - x, y0 + r, 2 * x+1, color);
    lcdDrawHLine(x0 - x, y0 - r, 2 * x+1, color);
  }
}

void lcdDrawCircleHelper( int32_t x0, int32_t y0, int32_t r, uint8_t cornername, uint32_t color)
{
  int32_t f     = 1 - r;
  int32_t ddF_x = 1;
  int32_t ddF_y = -2 * r;
  int32_t x     = 0;

  while (x < r)
  {
    if (f >= 0)
    {
      r--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;
    if (cornername & 0x4)
    {
      lcdDrawPixel(x0 + x, y0 + r, color);
      lcdDrawPixel(x0 + r, y0 + x, color);
    }
    if (cornername & 0x2)
    {
      lcdDrawPixel(x0 + x, y0 - r, color);
      lcdDrawPixel(x0 + r, y0 - x, color);
    }
    if (cornername & 0x8)
    {
      lcdDrawPixel(x0 - r, y0 + x, color);
      lcdDrawPixel(x0 - x, y0 + r, color);
    }
    if (cornername & 0x1)
    {
      lcdDrawPixel(x0 - r, y0 - x, color);
      lcdDrawPixel(x0 - x, y0 - r, color);
    }
  }
}

void lcdDrawRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint32_t color)
{
  // smarter version
  lcdDrawHLine(x + r    , y        , w - r - r, color); // Top
  lcdDrawHLine(x + r    , y + h - 1, w - r - r, color); // Bottom
  lcdDrawVLine(x        , y + r    , h - r - r, color); // Left
  lcdDrawVLine(x + w - 1, y + r    , h - r - r, color); // Right

  // draw four corners
  lcdDrawCircleHelper(x + r        , y + r        , r, 1, color);
  lcdDrawCircleHelper(x + w - r - 1, y + r        , r, 2, color);
  lcdDrawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
  lcdDrawCircleHelper(x + r        , y + h - r - 1, r, 8, color);
}

void lcdDrawFillCircleHelper(int32_t x0, int32_t y0, int32_t r, uint8_t cornername, int32_t delta, uint32_t color)
{
  int32_t f     = 1 - r;
  int32_t ddF_x = 1;
  int32_t ddF_y = -r - r;
  int32_t y     = 0;

  delta++;

  while (y < r)
  {
    if (f >= 0)
    {
      r--;
      ddF_y += 2;
      f     += ddF_y;
    }

    y++;
    ddF_x += 2;
    f     += ddF_x;

    if (cornername & 0x1)
    {
      lcdDrawHLine(x0 - r, y0 + y, r + r + delta, color);
      lcdDrawHLine(x0 - y, y0 + r, y + y + delta, color);
    }
    if (cornername & 0x2)
    {
      lcdDrawHLine(x0 - r, y0 - y, r + r + delta, color); // 11995, 1090
      lcdDrawHLine(x0 - y, y0 - r, y + y + delta, color);
    }
  }
}

void lcdDrawFillRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint32_t color)
{
  // smarter version
  lcdDrawFillRect(x, y + r, w, h - r - r, color);

  // draw four corners
  lcdDrawFillCircleHelper(x + r, y + h - r - 1, r, 1, w - r - r - 1, color);
  lcdDrawFillCircleHelper(x + r, y + r        , r, 2, w - r - r - 1, color);
}

void lcdDrawTriangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint32_t color)
{
  lcdDrawLine(x1, y1, x2, y2, color);
  lcdDrawLine(x1, y1, x3, y3, color);
  lcdDrawLine(x2, y2, x3, y3, color);
}

void lcdDrawFillTriangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint32_t color)
{
  uint16_t max_line_size_12 = cmax(abs(x1-x2), abs(y1-y2));
  uint16_t max_line_size_13 = cmax(abs(x1-x3), abs(y1-y3));
  uint16_t max_line_size_23 = cmax(abs(x2-x3), abs(y2-y3));
  uint16_t max_line_size = max_line_size_12;
  uint16_t i = 0;

  if (max_line_size_13 > max_line_size)
  {
    max_line_size = max_line_size_13;
  }
  if (max_line_size_23 > max_line_size)
  {
    max_line_size = max_line_size_23;
  }

  lcd_pixel_t line[max_line_size];

  lcdDrawLineBuffer(x1, y1, x2, y2, color, line);
  for (i = 0; i < max_line_size_12; i++)
  {
    lcdDrawLine(x3, y3, line[i].x, line[i].y, color);
  }
  lcdDrawLineBuffer(x1, y1, x3, y3, color, line);
  for (i = 0; i < max_line_size_13; i++)
  {
    lcdDrawLine(x2, y2, line[i].x, line[i].y, color);
  }
  lcdDrawLineBuffer(x2, y2, x3, y3, color, line);
  for (i = 0; i < max_line_size_23; i++)
  {
    lcdDrawLine(x1, y1, line[i].x, line[i].y, color);
  }
}

bool lcdDrawAvailable(void)
{
  return st7789IsReady();
}

bool lcdRequestDraw(void)
{
  if (is_init != true)
  {
    return false;
  }
  if (!lcdDrawAvailable())
  {
    return false;
  }

  lcdSetWindow(0, 0, LCD_WIDTH, LCD_HEIGHT);

  lcd_request_draw = true;
  st7789SendBuffer((uint8_t *)frame_buffer[draw_idx], LCD_WIDTH * LCD_HEIGHT, 0);

  return true;
}

void lcdUpdateDraw(void)
{
  uint32_t pre_time;

  if (is_init != true)
  {
    return;
  }

  lcdRequestDraw();

  pre_time = millis();
  while(lcdDrawAvailable() != true)
  {
    delay(1);
    if (millis()-pre_time >= 100)
    {
      break;
    }
  }
}

void lcdSetWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  if (is_init != true)
  {
    return;
  }

  st7789SetWindow(x+20, y, w-1+20, h-1);
}

int32_t lcdGetWidth(void)
{
  return LCD_WIDTH;
}

int32_t lcdGetHeight(void)
{
  return LCD_HEIGHT;
}


void lcdDrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);

  if (x0 < 0) x0 = 0;
  if (y0 < 0) y0 = 0;
  if (x1 < 0) x1 = 0;
  if (y1 < 0) y1 = 0;


  if (steep)
  {
    _swap_int16_t(x0, y0);
    _swap_int16_t(x1, y1);
  }

  if (x0 > x1)
  {
    _swap_int16_t(x0, x1);
    _swap_int16_t(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1)
  {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0<=x1; x0++)
  {
    if (steep)
    {
      lcdDrawPixel(y0, x0, color);
    } else
    {
      lcdDrawPixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0)
    {
      y0 += ystep;
      err += dx;
    }
  }
}

void lcdDrawLineBuffer(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, lcd_pixel_t *line)
{
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);

  if (x0 < 0) x0 = 0;
  if (y0 < 0) y0 = 0;
  if (x1 < 0) x1 = 0;
  if (y1 < 0) y1 = 0;


  if (steep)
  {
    _swap_int16_t(x0, y0);
    _swap_int16_t(x1, y1);
  }

  if (x0 > x1)
  {
    _swap_int16_t(x0, x1);
    _swap_int16_t(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1)
  {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0<=x1; x0++)
  {
    if (steep)
    {
      if (line != NULL)
      {
        line->x = y0;
        line->y = x0;
      }
      lcdDrawPixel(y0, x0, color);
    } else
    {
      if (line != NULL)
      {
        line->x = x0;
        line->y = y0;
      }
      lcdDrawPixel(x0, y0, color);
    }
    if (line != NULL)
    {
      line++;
    }
    err -= dy;
    if (err < 0)
    {
      y0 += ystep;
      err += dx;
    }
  }
}

void lcdDrawVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
  lcdDrawLine(x, y, x, y+h-1, color);
}

void lcdDrawHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
  lcdDrawLine(x, y, x+w-1, y, color);
}

void lcdDrawFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  for (int16_t i=x; i<x+w; i++)
  {
    lcdDrawVLine(i, y, h, color);
  }
}

void lcdDrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  lcdDrawHLine(x, y, w, color);
  lcdDrawHLine(x, y+h-1, w, color);
  lcdDrawVLine(x, y, h, color);
  lcdDrawVLine(x+w-1, y, h, color);
}

void lcdDrawFillScreen(uint16_t color)
{
  lcdDrawFillRect(0, 0, HW_LCD_WIDTH, HW_LCD_HEIGHT, color);
}

void lcdPrintf(int x, int y, uint16_t color,  const char *fmt, ...)
{
  va_list arg;
  va_start (arg, fmt);
  int32_t len;
  char print_buffer[256];
  int Size_Char;
  int i, x_Pre = x;
  han_font_t FontBuf;
  uint8_t font_width;
  uint8_t font_height;


  len = vsnprintf(print_buffer, 255, fmt, arg);
  va_end (arg);

  if (font_tbl[lcd_font]->data != NULL)
  {
    for( i=0; i<len; i+=Size_Char )
    {
      disEngFont(x, y, print_buffer[i], font_tbl[lcd_font], color);

      Size_Char = 1;
      font_width = font_tbl[lcd_font]->width;
      font_height = font_tbl[lcd_font]->height;
      x += font_width;

      if ((x+font_width) > HW_LCD_WIDTH)
      {
        x  = x_Pre;
        y += font_height;
      }
    }
  }
  else
  {
    for( i=0; i<len; i+=Size_Char )
    {
      hanFontLoad( &print_buffer[i], &FontBuf );

      disHanFont( x, y, &FontBuf, color);

      Size_Char = FontBuf.Size_Char;
      if (Size_Char >= 2)
      {
        font_width = 16;
        x += 2*8;
      }
      else
      {
        font_width = 8;
        x += 1*8;
      }

      if ((x+font_width) > HW_LCD_WIDTH)
      {
        x  = x_Pre;
        y += 16;
      }

      if( FontBuf.Code_Type == PHAN_END_CODE ) break;
    }
  }
}

uint32_t lcdGetStrWidth(const char *fmt, ...)
{
  va_list arg;
  va_start (arg, fmt);
  int32_t len;
  char print_buffer[256];
  int Size_Char;
  int i;
  han_font_t FontBuf;
  uint32_t str_len;


  len = vsnprintf(print_buffer, 255, fmt, arg);
  va_end (arg);


  str_len = 0;

  for( i=0; i<len; i+=Size_Char )
  {
    hanFontLoad( &print_buffer[i], &FontBuf );

    Size_Char = FontBuf.Size_Char;

    if (Size_Char >= 2)
    {
      str_len += (2 * 8);
    }
    else
    {
      str_len += (1 * 8);
    }
    if( FontBuf.Code_Type == PHAN_END_CODE ) break;
  }

  return str_len;
}

void disHanFont(int x, int y, han_font_t *FontPtr, uint16_t textcolor)
{
  uint16_t    i, j, Loop;
  uint16_t  FontSize = FontPtr->Size_Char;
  uint16_t index_x;

  if (FontSize > 2)
  {
    FontSize = 2;
  }

  for ( i = 0 ; i < 16 ; i++ )        // 16 Lines per Font/Char
  {
    index_x = 0;
    for ( j = 0 ; j < FontSize ; j++ )      // 16 x 16 (2 Bytes)
    {
      uint8_t font_data;

      font_data = FontPtr->FontBuffer[i*FontSize +j];

      for( Loop=0; Loop<8; Loop++ )
      {
        if( (font_data<<Loop) & (0x80))
        {
          lcdDrawPixel(x + index_x, y + i, textcolor);
        }
        index_x++;
      }
    }
  }
}

void disEngFont(int x, int y, char ch, lcd_font_t *font, uint16_t textcolor)
{
  uint32_t i, b, j;


  // We gaan door het font
  for (i = 0; i < font->height; i++)
  {
    b = font->data[(ch - 32) * font->height + i];
    for (j = 0; j < font->width; j++)
    {
      if ((b << j) & 0x8000)
      {
        lcdDrawPixel(x + j, (y + i), textcolor);
      }
    }
  }
}

void lcdSetFont(LcdFont font)
{
  lcd_font = font;
}

LcdFont lcdGetFont(void)
{
  return lcd_font;
}

void lcdDrawPixelBuffer(int16_t x_pos, int16_t y_pos, uint32_t rgb_code)
{
  font_src_buffer[y_pos * 16 + x_pos] = rgb_code;
}

void disHanFontBuffer(int x, int y, han_font_t *FontPtr, uint16_t textcolor)
{
  uint16_t    i, j, Loop;
  uint16_t  FontSize = FontPtr->Size_Char;
  uint16_t index_x;

  if (FontSize > 2)
  {
    FontSize = 2;
  }

  if (textcolor == 0)
  {
    textcolor = 1;
  }
  for ( i = 0 ; i < 16 ; i++ )        // 16 Lines per Font/Char
  {
    index_x = 0;
    for ( j = 0 ; j < FontSize ; j++ )      // 16 x 16 (2 Bytes)
    {
      uint8_t font_data;

      font_data = FontPtr->FontBuffer[i*FontSize +j];

      for( Loop=0; Loop<8; Loop++ )
      {
        if(font_data & ((uint8_t)0x80>>Loop))
        {
          lcdDrawPixelBuffer(index_x, i, textcolor);
        }
        else
        {
          lcdDrawPixelBuffer(index_x, i, 0);
        }
        index_x++;
      }
    }
  }
}

uint16_t lcdGetColorMix(uint16_t c1, uint16_t c2, uint8_t mix)
{
  uint16_t r, g, b;
  uint16_t ret;


  r = ((uint16_t)((uint16_t) GETR(c1) * mix + GETR(c2) * (255 - mix)) >> 8);
  g = ((uint16_t)((uint16_t) GETG(c1) * mix + GETG(c2) * (255 - mix)) >> 8);
  b = ((uint16_t)((uint16_t) GETB(c1) * mix + GETB(c2) * (255 - mix)) >> 8);

  ret = MAKECOL(r, g, b);

  return ret;
}

void lcdDrawPixelMix(int16_t x_pos, int16_t y_pos, uint32_t rgb_code, uint8_t mix)
{
  uint16_t color1, color2;

  if (x_pos < 0 || x_pos >= LCD_WIDTH) return;
  if (y_pos < 0 || y_pos >= LCD_HEIGHT) return;


  color1 = draw_buf[y_pos * LCD_WIDTH + x_pos];
  color2 = rgb_code;

  draw_buf[y_pos * LCD_WIDTH + x_pos] = lcdGetColorMix(color1, color2, 255-mix);
}

#ifdef HW_LCD_LVGL
image_t lcdCreateImage(lvgl_img_t *p_lvgl, int16_t x, int16_t y, int16_t w, int16_t h)
{
  image_t ret;

  ret.x = x;
  ret.y = y;

  if (w > 0) ret.w = w;
  else       ret.w = p_lvgl->header.w;

  if (h > 0) ret.h = h;
  else       ret.h = p_lvgl->header.h;

  ret.p_img = p_lvgl;

  if (p_lvgl->header.cf == LV_IMG_CF_INDEXED_8BIT)
  {
    const uint8_t *p_buf = p_lvgl->data;

    for (int i=0; i<256; i++)
    {
      uint8_t r, g, b;

      r = p_buf[2];
      g = p_buf[1];
      b = p_buf[0];

      ret.color_tbl[i] = MAKECOL(r, g, b);
      p_buf += 4;
    }
  }

  return ret;
}

bool lcdSpriteCreate(sprite_t *p_sprite)
{
  sprite_param_t *p_param = &p_sprite->param;

  p_sprite->image.x = 0;
  p_sprite->image.y = 0;

  p_sprite->image.w = p_param->w;
  p_sprite->image.h = p_param->h;

  p_sprite->image.p_img = p_param->p_img;
  p_sprite->pre_time = millis();
  p_sprite->cur_index = 0;

  if (p_param->p_img->header.cf == LV_IMG_CF_INDEXED_8BIT)
  {
    const uint8_t *p_buf = p_param->p_img->data;

    for (int i=0; i<256; i++)
    {
      uint8_t r, g, b;

      r = p_buf[2];
      g = p_buf[1];
      b = p_buf[0];

      p_sprite->image.color_tbl[i] = MAKECOL(r, g, b);
      p_buf += 4;
    }
  }

  return true;
}

void lcdSpriteDraw(sprite_t *p_sprite, int16_t x, int16_t y, uint16_t index)
{
  int16_t offset_x;
  int16_t offset_y;
  uint16_t draw_index;

  draw_index = index%p_sprite->param.cnt;

  offset_x = p_sprite->param.x + (p_sprite->param.stride_x * draw_index);
  offset_y = p_sprite->param.y + (p_sprite->param.stride_y * draw_index);

  lcdDrawImageOffset(&p_sprite->image, offset_x, offset_y, x, y);
}

void lcdSpriteDrawWrap(sprite_t *p_sprite, int16_t x, int16_t y, bool reset)
{
  if (reset == true)
  {
    p_sprite->cur_index = 0;
  }

  if (millis()-p_sprite->pre_time >= p_sprite->param.delay_ms)
  {
    p_sprite->pre_time = millis();
    p_sprite->cur_index = (p_sprite->cur_index + 1)%p_sprite->param.cnt;
  }
  lcdSpriteDraw(p_sprite, x, y, p_sprite->cur_index);
}

LCD_OPT_DEF void lcdDrawImageIndex8Bit(image_t *p_img, int16_t start_x, int16_t start_y, int16_t draw_x, int16_t draw_y)
{
  int32_t o_x;
  int32_t o_y;
  int16_t o_w;
  int16_t o_h;
  const uint8_t *p_data;
  uint16_t pixel;
  int16_t img_x = start_x;
  int16_t img_y = start_y;
  int16_t img_w = 0;
  int16_t img_h = 0;

  o_w = p_img->w;
  o_h = p_img->h;

  if (img_w > 0) o_w = img_w;
  if (img_h > 0) o_h = img_h;

  p_data = (uint8_t *)&p_img->p_img->data[4*256];

  for (int yi=0; yi<o_h; yi++)
  {
    o_y = p_img->y + yi + img_y;
    if (o_y >= p_img->p_img->header.h) break;

    o_y = o_y * p_img->p_img->header.w;
    for (int xi=0; xi<o_w; xi++)
    {
      o_x = p_img->x + xi + img_x;
      if (o_x >= p_img->p_img->header.w) break;

      pixel = p_img->color_tbl[p_data[o_y + o_x]];
      if (pixel != green)
      {
        lcdDrawPixel(draw_x+xi, draw_y+yi, pixel);
      }
    }
  }
}

LCD_OPT_DEF void lcdDrawImageTrueColor(image_t *p_img, int16_t start_x, int16_t start_y, int16_t draw_x, int16_t draw_y)
{
  int32_t o_x;
  int32_t o_y;
  int16_t o_w;
  int16_t o_h;
  const uint16_t *p_data;
  uint16_t pixel;
  int16_t img_x = start_x;
  int16_t img_y = start_y;
  int16_t img_w = 0;
  int16_t img_h = 0;

  o_w = p_img->w;
  o_h = p_img->h;

  if (img_w > 0) o_w = img_w;
  if (img_h > 0) o_h = img_h;

  p_data = (uint16_t *)p_img->p_img->data;

  for (int yi=0; yi<o_h; yi++)
  {
    o_y = p_img->y + yi + img_y;
    if (o_y >= p_img->p_img->header.h) break;

    o_y = o_y * p_img->p_img->header.w;
    for (int xi=0; xi<o_w; xi++)
    {
      o_x = p_img->x + xi + img_x;
      if (o_x >= p_img->p_img->header.w) break;

      pixel = p_data[o_y + o_x];
      if (pixel != green)
      {
        lcdDrawPixel(draw_x+xi, draw_y+yi, pixel);
      }
    }
  }
}

LCD_OPT_DEF void lcdDrawImageOffset(image_t *p_img, int16_t offset_x, int16_t offset_y, int16_t draw_x, int16_t draw_y)
{
  switch(p_img->p_img->header.cf)
  {
    case LV_IMG_CF_TRUE_COLOR:
      lcdDrawImageTrueColor(p_img, offset_x, offset_y, draw_x, draw_y);
      break;

    case LV_IMG_CF_INDEXED_8BIT:
      lcdDrawImageIndex8Bit(p_img, offset_x, offset_y, draw_x, draw_y);
      break;
  }
}

LCD_OPT_DEF void lcdDrawImage(image_t *p_img, int16_t draw_x, int16_t draw_y)
{
  lcdDrawImageOffset(p_img, 0, 0, draw_x, draw_y);
}
#endif

#ifdef _USE_HW_CLI
void cliLcd(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "info") == true)
  {
    cliPrintf("Driver : ST7789\n");
    cliPrintf("Width  : %d\n", lcdGetWidth());
    cliPrintf("Height : %d\n", lcdGetHeight());
    cliPrintf("BKL    : %d%%\n", lcdGetBackLight());
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "test") == true)
  {
    uint8_t cnt = 0;

    lcdSetFont(LCD_FONT_HAN);

    while(cliKeepLoop())
    {
      uint32_t pre_time;
      uint32_t exe_time;

      if (lcdDrawAvailable() == true)
      {
        pre_time = millis();
        lcdClearBuffer(black);

        lcdPrintf(0, 0, green, "[LCD 테스트]");

        lcdPrintf(LCD_WIDTH-40, 50+40, white, "%02d", cnt%100);
        cnt++;

        lcdDrawFillRect( 0, 120, 10, 10, red);
        lcdDrawFillRect(10, 120, 10, 10, green);
        lcdDrawFillRect(20, 120, 10, 10, blue);

        lcdDrawFillRect(0, 170+16, 240, 60, green);
        exe_time = millis()-pre_time;
        lcdPrintf(0, 170, red, "draw %d ms", exe_time);

        lcdRequestDraw();
      }
      delay(1);
    }

    ret = true;
  }
  if (args->argc == 2 && args->isStr(0, "bl") == true)
  {
    uint8_t bl_value;

    bl_value = args->getData(1);

    lcdSetBackLight(bl_value);

    ret = true;
  }

  if (ret != true)
  {
    cliPrintf("lcd info\n");
    cliPrintf("lcd test\n");
    cliPrintf("lcd bl 0~100\n");
  }
}
#endif



#endif
