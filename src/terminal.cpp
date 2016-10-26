#include <cstring>
#include "common.h"
#include "terminal.h"

#define CLR "\e[2J"
#define PROMPT "> "
#define INVALID "Invalid Command\r"
#define OK_STRING "OK\r"

static const char *info_text[] {
  CLR,
  "********************************************************************************\r",
  "    Firmware Version: ",
  FIRMWARE_VERSION,
  "\r    Hardware Version: ",
  HARDWARE_VERSION,
  "\r    Build Date:       ",
  COMPILED_DATE_TIME
  "\r********************************************************************************\r",
};

static const char *help_text[] {
  "h|?    - prints this text\r",
  "p      - prints header and clears screen\r",
  "c      - clears screen\r",
  "l1 1|0 - turns l1 on (1) or off(0)\r",
};

#define NUM_CMDS 5
#define CMD_PRINT     0
#define CMD_HELP_Q    1
#define CMD_HELP      2
#define CMD_CLEAR     3
#define CMD_LED_ON    4
#define CMD_LED_OFF   5

static const uint8_t cmds[][20] {
  {'p'},
  {'?'},
  {'h'},
  {'c'},
  {'l', '1', ' ', '1'},
  {'l', '1', ' ', '0'},
};

namespace CentralCommand {
  Terminal::Terminal(GPIO_TypeDef* led_port, uint32_t led_pin, USART_TypeDef* usart_base):
    led_port(led_port),
    led_pin(led_pin),
    usart_base(usart_base),
    current_state(TerminalState::PRINT_INFO)
  {

  }

  void Terminal::reset_cmd_buff() {
    this->cmd_index = 0;
    memset(this->cmd_buffer, 0, CMD_LEN);
  }

  void Terminal::initialise() {
    reset_cmd_buff();
  }

  void Terminal::run() {
    (this->*(state_map[current_state]))();
  }

  void Terminal::led_on() {
    LL_GPIO_SetOutputPin(this->led_port, this->led_pin);
    current_state = Terminal::OK;
  }

  void Terminal::led_off() {
    LL_GPIO_ResetOutputPin(this->led_port, this->led_pin);
    current_state = Terminal::OK;
  }

  void Terminal::print_info() {
    for (uint32_t i = 0; i < (sizeof(info_text) / sizeof(info_text[0])); i++) {
      const char *l = info_text[i];

      while (*l != '\0') {
        this->send(*l++);
      }
    }
    current_state = Terminal::PRINT_PROMPT;
  }

  void Terminal::print_help() {
    for (uint32_t i = 0; i < (sizeof(help_text) / sizeof(help_text[0])); i++) {
      const char *l = help_text[i];

      while (*l != '\0') {
        this->send(*l++);
      }
    }
    current_state = Terminal::PRINT_PROMPT;
  }

  void Terminal::print_prompt() {
    const char *prompt = PROMPT;

    while(*prompt != '\0') {
      this->send(*prompt++);
    }

    current_state = Terminal::PARSE_COMMAND;
  }

  void Terminal::clear_screen() {
      const char *clr = CLR;

      while(*clr != '\0') {
        this->send(*clr++);
      }

      current_state = Terminal::PRINT_PROMPT;
  }

  void Terminal::ok() {
    const char *ok = OK_STRING;

    while(*ok != '\0') {
      this->send(*ok++);
    }

    current_state = Terminal::PRINT_PROMPT;
  }

  void Terminal::invalid_command() {
    const char *invalid_command = INVALID;

    while(*invalid_command != '\0') {
      this->send(*invalid_command++);
    }

    current_state = Terminal::PRINT_PROMPT;
  }

  void Terminal::parse_command() {
    uint8_t data = 0;

    if (!buffer.read(&data)) {
      return;
    }

    this->send(data);

    // Can't exceed command buffer length
    if (data != '\r' && this->cmd_index >= CMD_LEN) {
      reset_cmd_buff();
      current_state = Terminal::INVALID_COMMAND;
      return;
    }

    // Read till new line (or max length above)
    // State continues as is
    if (data != '\r') {
      this->cmd_buffer[this->cmd_index] = data;
      this->cmd_index++;
      return;
    }

    // If we are here then we have a new line a and potential command
    // cmd_index will be plus one the actual index
    if (this->cmd_index == 1) {
      if (memcmp(this->cmd_buffer, cmds[CMD_PRINT], 1) == 0) {
        reset_cmd_buff();
        current_state = Terminal::PRINT_INFO;
        return;
      }

      if (memcmp(this->cmd_buffer, cmds[CMD_CLEAR], 1) == 0) {
        reset_cmd_buff();
        current_state = Terminal::CLEAR_SCREEN;
        return;
      }

      if ((memcmp(this->cmd_buffer, cmds[CMD_HELP], 1) == 0) ||
          (memcmp(this->cmd_buffer, cmds[CMD_HELP_Q], 1) == 0)) {
        reset_cmd_buff();
        current_state = Terminal::PRINT_HELP;
        return;
      }
    }

    if (this->cmd_index == 4) {
      if (memcmp(this->cmd_buffer, cmds[CMD_LED_ON], 4) == 0) {
        reset_cmd_buff();
        current_state = Terminal::LED_ON;
        return;
      }

      if (memcmp(this->cmd_buffer, cmds[CMD_LED_OFF], 4) == 0) {
        reset_cmd_buff();
        current_state = Terminal::LED_OFF;
        return;
      }
    }

    reset_cmd_buff();
    current_state = Terminal::INVALID_COMMAND;
    return;
  }

  void Terminal::send(uint8_t data) {
    while(!LL_USART_IsActiveFlag_TXE(this->usart_base));
    LL_USART_TransmitData8(this->usart_base, data);
  }

  void Terminal::on_interrupt(uint8_t data) {
    buffer.write(data);
  }

  Terminal::StateFunc Terminal::state_map[] {
    &Terminal::led_on,
    &Terminal::led_off,
    &Terminal::print_info,
    &Terminal::print_help,
    &Terminal::print_prompt,
    &Terminal::clear_screen,
    &Terminal::parse_command,
    &Terminal::ok,
    &Terminal::invalid_command
  };
}
