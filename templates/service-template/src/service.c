#include <libcgc.h>
#include "libc.h"

#define STR0 "Hello "
#define STR1 ".\nThis implements a simple echo service\n"

#define NAME_SIZE 4
#define BUF_SIZE 1024

char * get_name(void);

char * get_name(void) {
    char *buf;
    const char *names[] = {"Matt", "Mark", "Luke", "John"};
    unsigned int index;
    int ret;
    size_t bytes_written;
    int i;

    ret = random(&index, sizeof(index), &bytes_written);
    if (ret != 0)
        _terminate(1);

    if (bytes_written != sizeof(index))
        _terminate(2);

    ret = allocate(NAME_SIZE + 1, 0, (void **) &buf);
    if (ret != 0)
        _terminate(3);

    index = index % (sizeof(names) / sizeof(names[0]));

    for (i = 0; i < NAME_SIZE; i++) {
        buf[i] = names[index][i];
    }

    return buf;
}

int main(void) {
    char buf[1024];
    char *name;
    int ret;
    size_t size;
    
    name = get_name();

    ret = transmit_all(STDOUT, STR0, sizeof(STR0) - 1);
    if (ret != 0)
        _terminate(4);

    ret = transmit_all(STDOUT, name, NAME_SIZE);
    if (ret != 0)
        _terminate(5);

    ret = transmit_all(STDOUT, STR1, sizeof(STR1) - 1);
    if (ret != 0)
        _terminate(6);

    ret = deallocate(name, NAME_SIZE + 1);
    if (ret != 0)
        _terminate(7);

    while (1) {
        ret = receive(STDIN, buf, sizeof(buf) - 1, &size);
        if (ret != 0)
            _terminate(8);

        if (size == 0)
            _terminate(9);

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
            _terminate(10);
    }
}
