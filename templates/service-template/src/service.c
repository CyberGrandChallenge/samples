#include <libcgc.h>
#include "libc.h"

#define STR1 "This implements a simple echo service\n"

int main(void) {
    char buf[1024];
    int ret;
    size_t size;

    ret = transmit_all(STDOUT, STR1, sizeof(STR1) - 1);
    if (ret != 0)
        _terminate(1);

    while (1) {
        ret = receive(STDIN, buf, sizeof(buf) - 1, &size);
        if (ret != 0)
            _terminate(2);
        if (size == 0)
            _terminate(0);

        buf[size] = 0;

#ifdef PATCHED
#else
        if (buf[0] == 0x41 && buf[1] == 0x42) {
            char *p = 0;
            p[0] = 10;
        }
#endif

        ret = transmit_all(STDOUT, buf, size);
        if (ret != 0)
            _terminate(3);
    }
}
