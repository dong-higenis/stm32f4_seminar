#include "button.h"
#include "cli.h"




typedef struct
{
  GPIO_TypeDef *port;
  uint32_t      pin;
  GPIO_PinState on_state;
} button_tbl_t;


const button_tbl_t button_tbl[BUTTON_MAX_CH] =
    {
        {GPIOB, GPIO_PIN_12, GPIO_PIN_RESET},
        {GPIOB, GPIO_PIN_13, GPIO_PIN_RESET},
        {GPIOB, GPIO_PIN_14, GPIO_PIN_RESET},
        {GPIOB, GPIO_PIN_15, GPIO_PIN_RESET},
        {GPIOA, GPIO_PIN_11, GPIO_PIN_RESET},
        {GPIOA, GPIO_PIN_12, GPIO_PIN_RESET},
        {GPIOA, GPIO_PIN_2,  GPIO_PIN_RESET},
    };


#ifdef _USE_HW_CLI
static void cliButton(cli_args_t *args);
#endif





bool buttonInit(void)
{
  bool ret = true;


#ifdef _USE_HW_CLI
  cliAdd("button", cliButton);
#endif

  return ret;
}

bool buttonGetPressed(uint8_t ch)
{
  bool ret = false;

  if (ch >= BUTTON_MAX_CH)
  {
    return false;
  }

  if (HAL_GPIO_ReadPin(button_tbl[ch].port, button_tbl[ch].pin) == button_tbl[ch].on_state)
  {
    ret = true;
  }

  return ret;
}

void cliButton(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "show"))
  {
    while(cliKeepLoop())
    {
      for (int i=0; i<BUTTON_MAX_CH; i++)
      {
        cliPrintf("%d", buttonGetPressed(i));
      }
      cliPrintf("\n");

      delay(100);
    }

    ret = true;
  }


  if (ret != true)
  {
    cliPrintf("button show\n");
  }
}
