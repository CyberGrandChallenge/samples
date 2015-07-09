/*
 * Author: Sagar Momin <sagar.momin@kapricasecurity.com>
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
#include <ctype.h>
#include <string.h>

static char c64_decode_char(char c)
{
    if (isupper(c))
        return c - 'A';
    if (islower(c))
        return c - 'a' + 26;
    if (isalnum(c))
        return c - '0' + 52;
    if (c == '[')
        return 62;
    if (c == ']')
        return 63;
    if (c == '=')
        return -2;

    return -1;
}

int c64_decode(char *dec, char *enc)
{
    if (dec == NULL || enc == NULL)
        return -1;

    char decode[4] = {0, 0, 0, 0};

    size_t i, len = strlen(enc), di = 0;
    char dc;
    for (i = 0; i < len; i++)
    {
        if ((dc = c64_decode_char(enc[i])) == -1)
            return -1;
        else if (dc == -2)
            break;
        else
            decode[i % 4] = dc;

        if (i % 4 == 3)
        {
            dec[di++] = decode[0] << 2 | decode[1] >> 4;
            dec[di++] = decode[1] << 4 | decode[2] >> 2;
            dec[di++] = decode[2] << 6 | decode[3];

            if (!isprint(dec[di-3]) || !isprint(dec[di-2]) || !isprint(dec[di-1]))
                return -1;
            memset(decode, 0, 4);
        }
    }
    if (i == 0)
        return -1;
    if (--i % 4 == 0)
        return -1;
    if (i % 4 == 1)
    {
        dec[di++] = decode[0] << 2 | decode[1] >> 4;

        if (!isprint(dec[di-1]))
            return -1;
    }
    else if (i % 4 == 2)
    {
        dec[di++] = decode[0] << 2 | decode[1] >> 4;
        dec[di++] = decode[1] << 4 | decode[2] >> 2;

        if (!isprint(dec[di-2]) || !isprint(dec[di-1]))
            return -1;
    }

    dec[di] = '\0';
    return di;
}
