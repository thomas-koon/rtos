#include <stdint.h>
#include <stdio.h>

void format_hex(char* buffer, uint32_t val1)
{
    const char *hex_digits = "0123456789abcdef";
    int buf_idx = 0;
    int leading_zero = 1;  // To track if we're still dealing with leading zeros

    // Process val1
    for (int i = 0; i < 8; i++)
    {
        uint32_t shift = (7 - i) * 4;  // Shift by 28, 24, ..., 0
        uint8_t nibble = (val1 >> shift) & 0xF;  // Get 4 bits (nibble)

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

    if (buf_idx == 0)  // If the number is 0 (i.e., val1 == 0)
    {
        buffer[buf_idx++] = '0';
    }

    buffer[buf_idx] = '\0';
}

int main()
{
    char buffer[9];
    format_hex(buffer, 0x20001234);
    printf("%s", buffer);

    return 0;
}
