#include <libcgc.h>
#include "libc.h"

int transmit_all(int fd, const char *buf, const size_t size) {
    size_t sent = 0;
    size_t sent_now = 0;
    int ret;

    if (!buf) 
        return 1;

    if (!size)
        return 2;

    while (sent < size) {
        ret = transmit(fd, buf + sent, size - sent, &sent_now);
        if (sent_now == 0) {
            //should never return until at least something was transmitted
            //so consider this an error too
            return 3;
        }
        if (ret != 0) {
            return 3;
        }
        sent += sent_now;
    }

    return 0;
}

int receive_delim(int fd, char *buf, const size_t size, char delim, size_t *rxlen) {
    size_t rx = 0;
    size_t rx_now = 0;
    int ret;

    if (!buf) 
        return 1;

    if (!size)
        return 2;

    while (rx < size) {
        ret = receive(fd, buf + rx, 1, &rx_now);
        if (rx_now == 0) {
            //should never return until at least something was received
            //so consider this an error too
            return 3;
        }
        if (ret != 0) {
            return 3;
        }
        rx += rx_now;
        if (buf[rx - 1] == delim) {
           break;
        }
    }
    *rxlen = rx;
    return 0;
}

void *
memcpy(void *dst, const void *src, size_t n)
{
    char *d = dst;
    const char *s = src;

    while (n-- > 0)
        *d++ = *s++;
    return dst;
}
