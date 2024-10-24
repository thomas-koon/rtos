#include "kernel.h"
#include "logger.h"
#include "main.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_uart.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define LOG_MESSAGE_SIZE 9
char hex_format_buffer[LOG_MESSAGE_SIZE];

static void write_to_log_buffer(const char *msg);
static void format_hex(char* buffer, uint32_t val);

log_buffer_t log_buffer = { .head = 0, .tail = 0 };

void debug_log(uint32_t id, uint32_t param)
{
    enter_critical();
    //snprintf(hex_format_buffer, LOG_MESSAGE_SIZE, "%x %x\r\n", id, param);

    format_hex(hex_format_buffer, id);    
    write_to_log_buffer(hex_format_buffer);

    write_to_log_buffer(" ");

    format_hex(hex_format_buffer, param);    
    write_to_log_buffer(hex_format_buffer);

    write_to_log_buffer("\r\n");

    exit_critical();
}

static void write_to_log_buffer(const char *msg)
{
  while (*msg) 
  {
    log_buffer.buffer[log_buffer.head] = *msg++;
    log_buffer.head = (log_buffer.head + 1) % LOG_BUFFER_SIZE;
  }
}

void uart_dma_send()
{
  // if data is available
  if(log_buffer.head != log_buffer.tail)
  {
    uint16_t data_size = (log_buffer.head >= log_buffer.tail) ? 
                        (log_buffer.head - log_buffer.tail) : 
                        (LOG_BUFFER_SIZE - log_buffer.tail);

    HAL_UART_Transmit_DMA(&huart2, (uint8_t*)&log_buffer.buffer[log_buffer.tail], data_size);

    log_buffer.tail = (log_buffer.tail + data_size) % LOG_BUFFER_SIZE;
  }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    // called when DMA transfer is complete
    // check if more data needs to be sent
    if (log_buffer.head != log_buffer.tail) {
      uart_dma_send();
    }
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
