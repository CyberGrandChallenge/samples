#include <libcgc.h>
#include "libc.h"

#define BUF_SIZE 1024
#define CB_FD 3
#define CL_FD STDIN

char cb_buff[BUF_SIZE];
char cl_buff[BUF_SIZE];
int cb_len = 0;
int cl_len = 0;

void write(void) {
    struct timeval tv;
    fd_set ready_set;
    int ret;
    int fd;
    int err;

    FD_ZERO(&ready_set);

    if (cb_len == 0 && cl_len == 0) 
        return;

    if (cb_len)
        FD_SET(CL_FD, &ready_set);

    if (cl_len)
        FD_SET(CB_FD, &ready_set);

    tv.tv_sec = 0;
    tv.tv_usec = 1;

    err = fdwait(3 + 1, NULL, &ready_set, &tv, &fd);


    if (err)
        _terminate(0);

    if (fd > 0) {
        if (FD_ISSET(CB_FD, &ready_set)) {
            transmit_all(CB_FD, cl_buff, cl_len);
            cl_len = 0;
        }

        if (FD_ISSET(CL_FD, &ready_set)) {
            transmit_all(CL_FD, cb_buff, cb_len);
            cb_len = 0;
        }
    }
}

void read(void) {
    struct timeval tv;
    fd_set ready_set;
    int fd;
    int err;
    int ret;
    size_t size;
    
    FD_ZERO(&ready_set);

    if (cb_len < BUF_SIZE)
        FD_SET(CB_FD, &ready_set);
   
   if (cl_len < BUF_SIZE)
        FD_SET(CL_FD, &ready_set);

    tv.tv_sec = 0;
    tv.tv_usec = 1;

    err = fdwait(3 + 1, &ready_set, NULL, &tv, &fd);

    if (err)
        _terminate(1);

    if (fd > 0) {
        if (FD_ISSET(CB_FD, &ready_set)) {
            ret = receive(CB_FD, cb_buff + cb_len, BUF_SIZE-cb_len, &size);
            if (ret != 0) _terminate(2); 
            if (size == 0) _terminate(3);
            cb_len += size; 
        }
        
        if (FD_ISSET(CL_FD, &ready_set)) {
            ret = receive(CL_FD, cl_buff + cl_len, BUF_SIZE-cl_len, &size);
            if (ret != 0) _terminate(2); 
            if (size == 0) _terminate(3);
            cl_len += size; 
        }
    }
}

int main(void) {
    while (1) {
        write();
        read();
    }
}
