#include <libcgc.h>
#include "libc.h"

void bzero(char *buf, int n);
void ready(int fd);

void bzero(char *buf, int n) {
    for (int i = 0; i < n; i++) {
        buf[i] = 0;
    }
}

void ready(int fd) {
    fd_set write_fds;
    int ret;
    int ready_fd;
    struct timeval tv;

    bzero((char *)&write_fds, sizeof(write_fds));
    FD_SET(fd, &write_fds);

    tv.tv_sec = 10;
    tv.tv_usec = 0;

    ret = fdwait(fd + 1, NULL, &write_fds, &tv, &ready_fd);

    if (ret != 0) {
        _terminate(1);
    }

    if (!FD_ISSET(ready_fd, &write_fds))
        _terminate(1);
}

int transmit_all(int fd, const char *buf, const size_t size) {
    size_t sent = 0;
    size_t sent_now = 0;
    int ret;

    if (!buf) 
        return 1;

    if (!size)
        return 2;

    while (sent < size) {
        ready(fd);

        ret = transmit(fd, buf + sent, size - sent, &sent_now);
        if (ret != 0) {
            return 4;
        }
        sent += sent_now;
    }

    return 0;
}
