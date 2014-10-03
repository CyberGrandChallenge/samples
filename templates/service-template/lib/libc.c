#include <libcgc.h>
#include "libc.h"

void bzero(char *buf, int n);
int ready(int fd);

void bzero(char *buf, int n) {
    for (int i = 0; i < n; i++) {
        buf[i] = 0;
    }
}

int ready(int fd) {
    fd_set write_fds;
    int err;
    int ready_fd;
    struct timeval tv;

    bzero((char *)&write_fds, sizeof(write_fds));
    FD_SET(fd, &write_fds);

    tv.tv_sec = 10;
    tv.tv_usec = 0;

    err = fdwait(fd + 1, NULL, &write_fds, &tv, &ready_fd);
    if (err != 0) {
        return err;
    }
    
    if (!FD_ISSET(ready_fd, &write_fds))
        _terminate(11);

    return 0;
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
        ret = ready(fd);
        if (ret != 0) {
            return 3;
        }

        ret = transmit(fd, buf + sent, size - sent, &sent_now);
        if (ret != 0) {
            return 4;
        }
        sent += sent_now;
    }

    return 0;
}
