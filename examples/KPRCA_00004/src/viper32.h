#include <stdint.h>

static uint32_t viper32(void *data, size_t length)
{
    uint16_t A = 1, B = 0;
    uint8_t *bytes = data;
    size_t i;
    for (i = 0; i < length; i++)
    {
        A = (A + bytes[i]) % 65497;
        B = (B + A) % 65129;
    }
    return (B << 16) | A;
}
