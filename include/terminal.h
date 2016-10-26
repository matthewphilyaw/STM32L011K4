/**
 * @file terminal.h
 * @author Matthew Philyaw (matthew.philyaw@gmail.com)
 *
 * @brief Simply terminal program
 */

#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include "stm32l0xx.h"
#include "stm32l0xx_ll_gpio.h"
#include "stm32l0xx_ll_usart.h"
#include "ring_buffer.h"

#define CMD_LEN 20

namespace CentralCommand {
  class Terminal {
    private:
      GPIO_TypeDef* led_port;
      uint32_t led_pin;
      USART_TypeDef* usart_base;
      RingBuffer<uint8_t, 100> buffer;

      uint8_t cmd_index = 0;
      uint8_t cmd_buffer[CMD_LEN];

      void led_on();
      void led_off();
      void print_info();
      void print_help();
      void print_prompt();
      void clear_screen();
      void parse_command();
      void ok();
      void invalid_command();
      void reset_cmd_buff();

      void send(uint8_t data);

      enum TerminalState {
        LED_ON,
        LED_OFF,
        PRINT_INFO,
        PRINT_HELP,
        PRINT_PROMPT,
        CLEAR_SCREEN,
        PARSE_COMMAND,
        OK,
        INVALID_COMMAND
      };

      TerminalState current_state;

      typedef void (Terminal::*StateFunc)(void);
      static StateFunc state_map[];

    public:
      Terminal(GPIO_TypeDef* led_port, uint32_t led_pin, USART_TypeDef* usart_base);
      void initialise();
      void run();
      void on_interrupt(uint8_t byte);
  };
}

#endif /* ifndef TERMINAL_H */
