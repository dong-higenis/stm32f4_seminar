#include "ap.h"




void apInit(void)
{
  cliOpen(_DEF_UART1, 115200);
  cliLogo();
}


void apMain(void)
{
  typedef struct
  {
    bool is_enable;
    uint16_t color;
    int16_t x;
    int16_t y;
    int16_t size;
    int16_t speed;
  } bomb_info_t;

  uint32_t pre_time = 0;
  int16_t block_x = 0;
  int16_t block_y = 0;
  int16_t block_size = 20;
  uint16_t block_speed = 4;
  uint32_t pre_time_buzzer;


  bomb_info_t bomb[10];


  for (int i=0; i<10; i++)
  {
    bomb[i].is_enable = false;
  }

  while(1)
  {
    if (millis() - pre_time >= 500)
    {
      pre_time = millis();
      ledToggle(_DEF_LED1);

    }
    cliMain();

    if (lcdDrawAvailable())
    {
      static uint32_t pre_time_draw;
      static uint32_t exe_time_draw;


      pre_time_draw = millis();

      lcdClearBuffer(black);
      exe_time_draw = millis() - pre_time_draw;

      lcdDrawFillRect(block_x, block_y, block_size, block_size, green);

      lcdPrintf(5, 5, white, "테스트");
      lcdPrintf(5, 25, white, "%d ms", exe_time_draw);
      lcdPrintf(5, 45, white, "%d ms", lcdGetDrawTime());
      lcdPrintf(150, 5, white, "X %d", adcRead(0));
      lcdPrintf(150, 25, white, "Y %d", adcRead(1));

      for (int i=0; i<BUTTON_MAX_CH; i++)
      {
        lcdPrintf(5+16*i, 65, white, "%d", buttonGetPressed(i));
      }

      lcdDrawRect(0, 0, LCD_WIDTH, LCD_HEIGHT, white);

      if (adcRead(0) < 1500)
      {
        block_x = constrain((block_x - block_speed), 0, LCD_WIDTH);
      }
      if (adcRead(0) > 2500)
      {
        block_x = constrain((block_x + block_speed), 0, LCD_WIDTH);
      }
      if (adcRead(1) > 2500)
      {
        block_y = constrain((block_y - block_speed), 0, LCD_HEIGHT);
      }
      if (adcRead(1) < 1500)
      {
        block_y = constrain((block_y + block_speed), 0, LCD_HEIGHT);
      }


      for (int i=0; i<10; i++)
      {
        if (bomb[i].is_enable)
        {
          bomb[i].x += bomb[i].speed;
          if (bomb[i].x > LCD_WIDTH)
          {
            bomb[i].is_enable = false;
          }
          lcdDrawFillRect(bomb[i].x, bomb[i].y, bomb[i].size, bomb[i].size, bomb[i].color);
          lcdPrintf(bomb[i].x+2, bomb[i].y+2, white, "%d", i);
        }
      }


      lcdRequestDraw();
    }

    if (buttonGetPressed(2) || buttonGetPressed(3))
    {
      if (millis()-pre_time_buzzer >= 200)
      {
        buzzerOn(1000, 100);
        pre_time_buzzer = millis();

        for (int i=0; i<10; i++)
        {
          if (!bomb[i].is_enable)
          {
            bomb[i].is_enable = true;
            bomb[i].y = block_y;
            bomb[i].x = block_x;
            bomb[i].size = block_size;

            if (buttonGetPressed(3))
            {
              bomb[i].color = red;
              bomb[i].speed = block_speed * 3;
            }
            else
            {
              bomb[i].color = blue;
              bomb[i].speed = block_speed;
            }
            break;
          }
        }
      }
    }
  }
}
