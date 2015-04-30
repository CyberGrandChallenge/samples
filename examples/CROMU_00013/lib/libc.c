/*

Author: Steve Wood <swood@cromulence.co>

Copyright (c) 2014 Cromulence LLC

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#include <libcgc.h>
#include "libc.h"

void reverse_str(char *str, int len)
{
    char temp;
    int i;

    for (i=0; i<len/2; ++i)
    {
        temp=str[i];
        str[i]=str[len-i-1];
        str[len-i-1]=temp;
    }

}

int isspace(char c){

    if ((c > 0x08 && c < 0x0e) || c == 0x20)
        return 1;
    else
        return 0;

}

int isdigit(char c){

    if (c >= '0' && c <= '9')
        return 1;
    else
        return 0;

}


int atoi(char *input) {

    int num=0;
    int negative=0;

    while (isspace(*input))
        ++input;

    if (*input=='-'){
        negative=1;
        ++input;
    }
    else if (*input=='+')
        ++input;

    while (*input){

        if (isdigit(*input))
            num=num*10+(*input - '0');
        else
            break;

        ++input;
    }

    if (negative)
        return -num;
    else
        return num;

} // atoi()



int strlen(char *buff) {
    int i=0;

    while (*buff++)
        ++i;

    return (i);
}


void print_nl() {

    char newline=0x0a;
    transmit_all(STDOUT, &newline, 1);
}

// if commas==1 then number separating commas will be inserted into the output
void print_int(int num, int commas) {
    int tmpnum;
    int digit;
    int i=0;

#ifdef PATCHED
    char buff[14]; // maximum length of int string = 10 digits, 3 commas, negative sign
#else
    char buff[13];  // in this one, the extra byte for the sign is missing
#endif

    if (num < 0)
        tmpnum=-num;
    else
        tmpnum=num;


    do {
        digit=tmpnum%10;

        // insert commas after every third digit
        if (commas && (i+1)%4==0){
            buff[i]=',';
            ++i;
        }

        buff[i]=digit+'0';
        ++i;
        tmpnum=tmpnum/10;

    } while (tmpnum);

    if (num < 0) {
        buff[i++]='-';
    }

    //now correct the order of the digits in the string
    reverse_str(buff, i);

    transmit_all(STDOUT, buff, i);

} // print_int()

int strlcpy(char *s, char *d, int len)
{
    int i;

    if (s==0 || d==0)
        return 0;

    for (i=0; i< len; ++i)
    {
        *d++=*s++;
    }

    *d=0;

    return len;
}

void bzero(char *buf, int n) {
    for (int i = 0; i < n; i++) {
        buf[i] = 0;
    }
}

void print_str(char *str){

    transmit_all(STDOUT, str, strlen(str));

} //print_str()

int getnstr(char *buffer, size_t len) {

    size_t rcv_cnt;

    bzero(buffer, len);

    if (receive(STDIN, buffer, len, &rcv_cnt) != 0)
        return (-1);

    buffer[rcv_cnt-1]=0;

    if (rcv_cnt == len)
        flush_input(STDIN);

    return (rcv_cnt-1);

}



// use this if there might be extra characters after a readline
int flush_input(int fd) {
    fd_set read_fds;
    int err;
    int ready_fd;
    struct timeval tv;
    char buffer[1024];
    size_t rcv_cnt;

    while (1)  {

        bzero((char *)&read_fds, sizeof(read_fds));
        FD_SET(fd, &read_fds);

        tv.tv_sec = 0;
        tv.tv_usec = 10;

        err = fdwait(fd + 1, &read_fds, NULL, &tv, &ready_fd);
        if (err != 0) {
            return err;
        }

        if (!FD_ISSET(fd, &read_fds))
            break;
        else
            receive(fd, buffer, 1024, &rcv_cnt);
     
    }  

    return 0;
} // ready()

int transmit_all(int fd, const char *buf, const size_t size) {

    size_t sent = 0;
    int ret;

        ret = transmit(fd, buf, size, &sent);

    return 0;
}


void *memcpy(void *dst, void *src, size_t count)  {
    int i;

    for (i=0;i< count;++i)
        *((char *)dst+i)=*((char *)src+i);

    return (dst);

}
