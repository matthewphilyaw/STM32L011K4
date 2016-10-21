#include "main.h"

void ll_init(void) {
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  /* System interrupt init*/
  NVIC_SetPriority(SysTick_IRQn, 0);
}

void system_clock_config(void) {
  LL_RCC_PLL_Disable();
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);
  LL_RCC_MSI_Enable();

  while(LL_RCC_MSI_IsReady() != 1) {
    //empty
  }

  LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_5);  
  LL_RCC_MSI_SetCalibTrimming(0x0);

  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_MSI);

  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_MSI) {
    //empty
  }

  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

  LL_Init1msTick(2097000);
  LL_SetSystemCoreClock(2097000);

  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE3);
  LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_PWR);
}

int main(void) {
  //ll_init();
  //system_clock_config();

  /*LL_GPIO_InitTypeDef led;
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);

  led.Pin = LL_GPIO_PIN_3;
  led.Mode = LL_GPIO_MODE_OUTPUT;
  led.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  led.Pull = LL_GPIO_PULL_NO;

  LL_GPIO_Init(GPIOB, &led);
  */
  for (;;) {
    //LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_3);
    //LL_mDelay(150);
  }
}
