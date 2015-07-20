#include <libcgc.h>
#include "libc.h"

int main(void) {
    char buf[1024];
    char *name;
    int ret;
    size_t size;

    while (1) {
        ret = receive(4, buf, sizeof(buf) - 1, &size);
        if (ret != 0)
            _terminate(4);

        if (size == 0)
            _terminate(5);

        buf[size] = 0;

        ret = transmit_all(5, buf, size);
        if (ret != 0)
            _terminate(6);
    }
}
