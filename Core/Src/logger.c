#include "logger.h"
#include "kernel.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "stm32f4xx_hal.h"

#define LOG_MESSAGE_SIZE 9
#define UART_BUFFER_SIZE 1
int buffer_index = 0;
char snprintf_buffer[LOG_MESSAGE_SIZE];
char uart_buffer[UART_BUFFER_SIZE];

static void uart_send_char(char c);
static void format_hex(char* buffer, uint32_t val);

void debug_log(uint32_t id, uint32_t param)
{
    enter_critical();
    //snprintf(snprintf_buffer, LOG_MESSAGE_SIZE, "%x %x\r\n", id, param);

    format_hex(snprintf_buffer, id);    
    uart_print(snprintf_buffer);

    uart_print(" ");

    format_hex(snprintf_buffer, param);    
    uart_print(snprintf_buffer);

    uart_print("\r\n");

    exit_critical();
}

void uart_print(const char *str)
{
  enter_critical();
  while (*str) 
  {
    // Add characters to the buffer
    uart_send_char(*str++);
  }
  exit_critical();
}

static void uart_send_char(char c)
{
  while( !( ((&huart2)->gState == HAL_UART_STATE_READY) && (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TXE)) ) );
  (&huart2)->Instance->DR = c;
}

static void format_hex(char* buffer, uint32_t val)
{
    const char *hex_digits = "0123456789abcdef";
    int buf_idx = 0;
    int leading_zero = 1;  // To track if we're still dealing with leading zeros

    for (int i = 0; i < 8; i++)
    {
        uint32_t shift = (7 - i) * 4;  // Shift by 28, 24, ..., 0
        uint8_t nibble = (val >> shift) & 0xF;  // Get 4 bits (nibble)

        // Skip leading zeros
        if (nibble == 0 && leading_zero && i != 7) 
        {
            continue;
        }
        else
        {
            leading_zero = 0;  // We've found a non-zero digit
            buffer[buf_idx++] = hex_digits[nibble];
        }
    }

    if (buf_idx == 0)  // If the number is 0 (i.e., val == 0)
    {
        buffer[buf_idx++] = '0';
    }

    buffer[buf_idx] = '\0';
}
