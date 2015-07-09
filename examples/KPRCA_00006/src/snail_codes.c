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
#include <stddef.h>
#include <stdlib.h>
#include "snail_codes.h"

#define HASHSIZE 32
//Local global variable
static int initialized = 0;
static struct scode_list *scode_dict[HASHSIZE];

//List of all codes
static snail_code_t snail_codes[] = {
/* Hacked solution -- if code < 200, respond without the code */
/* Hacked solution 2 -- if code > 999, code=code/10, indicates alternate response */
    { EHLO_CODE, EHLO_RESP },
    { HELO_CODE, HELO_RESP },
    { 200, "mx.darpa.mil Mail service" },
    { 214, "I support: EHLO, HELO, AUTH, POST, SEND, DATA, RSET, NOOP, VRFY, HELP, QUIT" },
    { 220, "mx.darpa.mil Mail service ready" },
    { 221, "mx.darpa.mil Closing transmission channel" },
    { 235, "Accepted" },
    { 250, "Ok" },
    { 252, "Send some mail, I'll try my best" },
    { 334, "VXNlcm5hbWU6" },
    { 3342, "UGFzc3dvcmQ6" },
    { 354, "Start mail input; end with <LF>.<LF>" },
    { 421, "mx.darpa.mil Service not available, closing transmission channel" },
    { 450, "Requested mail action not taken: mailbox unavailable" },
    { 451, "Start a new session to send additional mail" },
    { 452, "Reached message data limit, sending..." },
    { 500, "Syntax error, command unrecognized" },
    { 501, "Cannot decode response" },
    { 5012, "Invalid address" },
    { 5013, "Too many recipients" },
    { 502, "Command not implemented" },
    { 503, "Bad sequence of commands" },
    { 5032, "No identity changes permitted" },
    { 504, "Command parameter not implemented" },
    { 5042, "Unrecognized authentication type" },
    { 521, "mx.darpa.mil does not accept mail (see rfc1846)" },
    { 530, "Authentication required" },
    { 535, "Username and Password not accepted" },
    { 550, "Requested action not taken: mailbox unavailable" },
    { 551, "User not local", },
    { 552, "Requested mail action aborted: exceeded storage allocation" },
    { 553, "Unable to find recipient domain" },
    { 554, "Transaction failed" },
    { 5542, "Line too long, try a shorter message" },
    { 5543, "Email body too long, try a shorter message" },
    { 555, "Syntax error"}
};

static unsigned int hash(int code)
{
    return (code * 31) % HASHSIZE;
}

static struct scode_list *get(int code)
{
    struct scode_list *sp;
    for (sp = scode_dict[hash(code)]; sp != NULL; sp = sp->next)
        if (code == sp->scode->code)
          return sp;
    return NULL;
}

static int update(snail_code_t *scode)
{
    struct scode_list *sp;

    if ((sp = get(scode->code)) == NULL)
    {
        int key;

        sp = (struct scode_list *) malloc(sizeof(*sp));
        if (sp == NULL)
            return 0;

        key = hash(scode->code);
        sp->next = scode_dict[key];
        scode_dict[key] = sp;
    }

    sp->scode = scode;
    return 1;
}

static int init()
{
    if (initialized == 1)
        return 1;

    size_t i, array_len = sizeof(snail_codes) / sizeof(snail_codes[0]);
    for (i = 0; i < array_len ; i++)
        if (!update(&snail_codes[i]))
            return 0;

    initialized = 1;
    return 1;
}

snail_code_t *get_snail_resp(int code)
{
    if(!initialized)
        if(!init())
            return NULL;

    struct scode_list *sp;
    if ((sp = get(code)) == NULL)
        return NULL;
    else
        return sp->scode;
}
