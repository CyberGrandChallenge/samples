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

#ifndef LIBC_H
#define LIBC_H
#include <libcgc.h>

void *memcpy(void *dst, void *src, size_t count);
int transmit_all(int fd, const char *buf, const size_t size);
void reverse_str(char *str, int len);
int strlen(char *buff);
void print_int(int num, int commas);
void print_byte(unsigned char byte);
void print_short(short int num);
void print_float(float num, int decimal_places);
void print_str(char *str);
int getnstr(char *buffer, size_t len);
void print_nl();
int strlcpy(char *s, char *d, int len);
int atoi(char *input);
float atof(char *input);
int isspace(char c);
int isdigit(char c);
int flush_input(int fd);

#endif
