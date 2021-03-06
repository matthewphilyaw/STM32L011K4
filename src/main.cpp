#include "main.h"

extern "C" void USARTx_IRQHandler(void);

using namespace CentralCommand;

Terminal term(GPIOB, LL_GPIO_PIN_3, USARTx_INSTANCE);

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

  while(LL_RCC_MSI_IsReady() != 1);
  LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_5);
  LL_RCC_MSI_SetCalibTrimming(0x0);

  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_MSI);

  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_MSI);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

  LL_Init1msTick(2097000);
  LL_SetSystemCoreClock(2097000);

  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE3);
  LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_PWR);
}

void Configure_USART(void) {

  USARTx_GPIO_CLK_ENABLE();

  LL_GPIO_SetPinMode(USARTx_TX_GPIO_PORT, USARTx_TX_PIN, LL_GPIO_MODE_ALTERNATE);
  USARTx_SET_TX_GPIO_AF();
  LL_GPIO_SetPinSpeed(USARTx_TX_GPIO_PORT, USARTx_TX_PIN, LL_GPIO_SPEED_FREQ_VERY_HIGH);
  LL_GPIO_SetPinOutputType(USARTx_TX_GPIO_PORT, USARTx_TX_PIN, LL_GPIO_OUTPUT_PUSHPULL);
  LL_GPIO_SetPinPull(USARTx_TX_GPIO_PORT, USARTx_TX_PIN, LL_GPIO_PULL_UP);

  LL_GPIO_SetPinMode(USARTx_RX_GPIO_PORT, USARTx_RX_PIN, LL_GPIO_MODE_ALTERNATE);
  USARTx_SET_RX_GPIO_AF();
  LL_GPIO_SetPinSpeed(USARTx_RX_GPIO_PORT, USARTx_RX_PIN, LL_GPIO_SPEED_FREQ_VERY_HIGH);
  LL_GPIO_SetPinOutputType(USARTx_RX_GPIO_PORT, USARTx_RX_PIN, LL_GPIO_OUTPUT_PUSHPULL);
  LL_GPIO_SetPinPull(USARTx_RX_GPIO_PORT, USARTx_RX_PIN, LL_GPIO_PULL_UP);

  NVIC_SetPriority(USARTx_IRQn, 0);
  NVIC_EnableIRQ(USARTx_IRQn);

  USARTx_CLK_ENABLE();
  USARTx_CLK_SOURCE();

  LL_USART_SetTransferDirection(USARTx_INSTANCE, LL_USART_DIRECTION_TX_RX);
  LL_USART_ConfigCharacter(USARTx_INSTANCE, LL_USART_DATAWIDTH_8B, LL_USART_PARITY_NONE, LL_USART_STOPBITS_1);
  LL_USART_SetBaudRate(USARTx_INSTANCE, SystemCoreClock, LL_USART_OVERSAMPLING_16, 115200); 
  LL_USART_Enable(USARTx_INSTANCE);

  while((!(LL_USART_IsActiveFlag_TEACK(USARTx_INSTANCE))) || (!(LL_USART_IsActiveFlag_REACK(USARTx_INSTANCE))));

  LL_USART_EnableIT_RXNE(USARTx_INSTANCE);
}

void USART2_IRQHandler(void) {
  if (LL_USART_IsActiveFlag_RXNE(USARTx_INSTANCE) ||
      LL_USART_IsEnabledIT_RXNE(USARTx_INSTANCE)) {
    volatile uint8_t received_char;

    received_char = LL_USART_ReceiveData8(USARTx_INSTANCE);

    term.on_interrupt(received_char);
  }
}

int main(void) {
  system_clock_config();

  LL_GPIO_InitTypeDef led;
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);

  led.Pin = LL_GPIO_PIN_3;
  led.Mode = LL_GPIO_MODE_OUTPUT;
  led.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  led.Pull = LL_GPIO_PULL_NO;

  LL_GPIO_Init(GPIOB, &led);

  Configure_USART();

  term.initialise();

  for (int i = 0; i < 10; i++) {
    LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_3);
    LL_mDelay(100);
  }

  for (;;) {
    term.run();
  }
}
