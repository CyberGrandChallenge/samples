#include <libcgc.h>
#include "libc.h"

int transmit_all(int fd, const char *s, size_t size) {
    int ret;
    size_t total_sent = 0;
    size_t sent = 0;

    while (total_sent < size) {
        ret = transmit(fd, s + total_sent, size - total_sent, &sent);
        if (ret > 0) {
            _terminate(2);
        }
        total_sent += sent;
    }

    return 1;
}
