#include "buzzer.h"
#include "cli.h"



static bool is_init = false;
static bool is_on = false;
static uint16_t on_time_cnt = 0;

extern TIM_HandleTypeDef htim4;

static void cliBuzzer(cli_args_t *args);





bool buzzerInit(void)
{
  bool ret = true;


  cliAdd("buzzer", cliBuzzer);

  is_init = ret;
  return ret;
}

void buzzerISR(void *arg)
{
  if (is_on && on_time_cnt > 0)
  {
    on_time_cnt--;

    if (on_time_cnt == 0)
    {
      buzzerOff();
    }
  }
}

void buzzerOn(uint16_t freq_hz, uint16_t time_ms)
{
  uint32_t freq_cmp;

  freq_cmp = (1000000/freq_hz) - 1;

  if (freq_hz >= 20 && freq_hz <= 500000)
  {
    htim4.Instance->ARR = freq_cmp/2;

    HAL_TIM_OC_Start(&htim4, TIM_CHANNEL_1);
  }

  is_on = true;
  on_time_cnt = time_ms;
}

void buzzerOff(void)
{
  HAL_TIM_OC_Stop(&htim4, TIM_CHANNEL_1);
  is_on = false;
}

void cliBuzzer(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 3 && args->isStr(0, "on") == true)
  {
    uint32_t freq;
    uint32_t on_time;

    freq = args->getData(1);
    on_time = args->getData(2);

    buzzerOn(freq, on_time);

    ret = true;
  }


  if (ret != true)
  {
    cliPrintf("buzzer on [freq] [ms]\n");
  }
}
