/*
 * Author: Brian Pak <brian.pak@kapricasecurity.com>
 * 
 * Copyright (c) 2014 Kaprica Security, Inc.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 */
#include <libcgc.h>
#include <ctype.h>
#include <string.h>

double strtod(const char *str, char **endptr)
{
    const char *orig = str;
    char c;
    int n, neg = 0, point = 0;
    double val = 0, under = 1;

    while (*str && isspace(*str))
        str++;

    if (*str == '-')
    {
        neg = 1;
        str++;
    }
    else if (*str == '+')
    {
        str++;
    }

    /* TODO: maybe handle hexdecimal format */

    const char *begin = str;

    while ((c = *str))
    {
        if (c == '.')
        {
            point = 1;
            str++;
        }
        else if (c >= '0' && c <= '9')
        {
            n = c - '0';
            if (point)
            {
                under /= 10;
                val = val + (under * (double) n);
            }
            else
            {
                val = val * 10.0 + (double) n;
            }
            str++;
        }
        else
            break;
    }

    if (begin == str)
    {
        str = orig;
        val = 0;
    }

    if (endptr)
        *endptr = (char *)str;
    if (neg)
        return -val;
    else
        return val;
}
