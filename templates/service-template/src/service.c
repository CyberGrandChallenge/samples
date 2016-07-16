#include <libcgc.h>
#include "libc.h"

#define STR0 "Hello "
#define STR1 ".\nThis implements a simple echo service\n"

#define NAME_SIZE 4
#define BUF_SIZE 1024

size_t printf(const char *format, ...);

char * get_name(void *secret_page);
size_t receive_line(int fd, char *buf, size_t size);

char * get_name(void *secret_page) {
    char *buf;
    const char *names[] = {"Matt", "Mark", "Luke", "John"};
    unsigned int index;
    int ret;
    int i;
   
    index = ((int *)secret_page)[0] % (sizeof(names) / sizeof(names[0]));

    ret = allocate(NAME_SIZE + 1, 0, (void **) &buf);
    if (ret != 0)
        _terminate(1);

    for (i = 0; i < NAME_SIZE; i++) {
        buf[i] = names[index][i];
    }

    return buf;
}

size_t receive_line(int fd, char *buf, size_t size) {
    char c;
    size_t total = 0;
    size_t read = 0;

    while (1) {
        if (receive(fd, &c, 1, &read) != 0) {
            _terminate(1);
        }

        if (read == 0) {
            return 0;
        }

        if (total >= size) {
            return 0;
        }

        buf[total++] = c;

        if (c == '\n') {
            break;
        }
    }

    return total;
}

void *copy(void *dst, const void *src, size_t size) {
    char *dst_char = (char *)dst;
    char *src_char = (char *)src;

    while (size--)
        *dst_char++ = *src_char++;

    return dst;
}


int __attribute__((fastcall)) main(int secret_page_i, char *unused[]) {
    void *secret_page = (void *)secret_page_i;
    char buf[1024];
    char *name;
    int ret;
    size_t size;
    
    name = get_name(secret_page);

    ret = transmit_all(STDOUT, STR0, sizeof(STR0) - 1);
    if (ret != 0)
        _terminate(1);

    ret = transmit_all(STDOUT, name, NAME_SIZE);
    if (ret != 0)
        _terminate(1);

    ret = transmit_all(STDOUT, STR1, sizeof(STR1) - 1);
    if (ret != 0)
        _terminate(1);

    ret = deallocate(name, NAME_SIZE + 1);
    if (ret != 0)
        _terminate(1);

    while (1) {
        size = receive_line(STDIN, buf, sizeof(buf) - 1);
        if (size == 0)
            _terminate(1);

        buf[size] = 0;

#ifndef PATCHED_1
        if (buf[0] == 0x41 && buf[1] == 0x42 && buf[2] == 0x43) {
            int (*func)(int);
            copy(&func, buf+3, 4);
            copy(&size, buf+7, 4);
            transmit(STDERR, "type1\n", 6, NULL);
            ret = (*func)(size);
        }
#endif

#ifndef PATCHED_2
        if (buf[0] == 0x41 && buf[1] == 0x42 && buf[2] == 0x44) {
            transmit(STDERR, "type2\n", 6, NULL);
            transmit_all(STDOUT, secret_page, 0x1000);
        }
#endif

        ret = transmit_all(STDOUT, buf, size);
        if (ret != 0)
            _terminate(1);
    }
}
