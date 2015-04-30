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
#include <libcgc.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "snail_codes.h"

//Cat 64 decoder from c64.c
int c64_decode(char *dec, char *enc);

// SNAIL Server Commands
#define EHOL "EHOL"
#define HEOL "HEOL"
#define AUTH "AUTH"
#define MAIL "POST FROM"
#define RCPT "SEND TO"
#define DATA "DATA"
#define RSET "RSET"
#define NOOP "NOOP"
#define VRFY "VRFY"
#define HELP "HELP"
#define QUIT "QUIT"

// Support authentication types
#define LOGIN "LOGIN"
#define USER "cgc@darpa.mil"
#define PASS "cgc!darpa08041997"

//SNAIL Mail option buffer sizes
#define MAX_MSGS 10
#define MAX_RCPT 16
#define MAX_ADDR_LEN 32
#define MAX_DATA 512
#define MAX_USER_LEN 64
#define MAX_PASS_LEN 64
#define MAX_LINE 64

// SNAIL Email Data (Subset of DATA)
#define FINISHED ".\n"


//Struct allocates all data on the stack - *from gets set in init()
typedef struct {
    char *from;
    char rcpts[MAX_RCPT][MAX_ADDR_LEN];
    char body[MAX_DATA];
    int num_rcpts;
} email_t;

typedef int (*handler_t) (char *param, snail_state *state);

typedef struct {
    const char *name;
    handler_t handler;
} command_t;

static int handle_ehlo(char *param, snail_state *state);
static int handle_helo(char *param, snail_state *state);
static int handle_auth(char *param, snail_state *state);
static int handle_mail(char *param, snail_state *state);
static int handle_rcpt(char *param, snail_state *state);
static int handle_data(char *param, snail_state *state);
static int handle_rset(char *param, snail_state *state);
static int handle_noop(char *param, snail_state *state);
static int handle_vrfy(char *param, snail_state *state);
static int handle_help(char *param, snail_state *state);
static int handle_quit(char *param, snail_state *state);

static command_t commands[] = {
    { EHOL, handle_ehlo },
    { HEOL, handle_helo },
    { AUTH, handle_auth },
    { MAIL, handle_mail },
    { RCPT, handle_rcpt },
    { DATA, handle_data },
    { RSET, handle_rset },
    { NOOP, handle_noop },
    { VRFY, handle_vrfy },
    { HELP, handle_help },
    { QUIT, handle_quit },
    /* End */
    { 0, NULL}
};

// Global variables to check state -- all globals allocated on stack
static int g_authenticated = 0;

//Max of 1 message can be sent, for now
static email_t msgs[MAX_MSGS];
static size_t sent_msgs = 0;

/* Start Functions */
/* Takes an char* and removes all trailing nulls, \t, \r, \n, and ' '
    returns a pointer to the beginning of the first non whitespace */
static char *sanitize_input(char *input)
{
    if (input == NULL)
        return NULL;

    while (*input == ' ')
        input++;

    int i, len = strlen(input);
    for(i = len; i >= 0; i--)
        if(input[i] == '\0' || input[i] == '\t' || input[i] == '\r'
            || input[i] == '\n' || input[i] == ' ')
            input[i] = '\0';
        else
            break;

    if (input[0] == '\0')
        return NULL;
    else
        return input;
}


/* Doesn't actually check to make sure it's valid. Just checks to see if it has x@y.something */
static int valid_email(char *input)
{
    if (input == NULL)
        return 0;

    while (*input != '\0')
        if(*input++ == '@')
            break;

    if (*input == '\0' || *input++ == '.')
        return 0;

    while (*input != '\0')
    {
        if(*input++ == '.')
        {
            if(*input != '\0')
                return 1;
            else
                return 0;
        }
    }

    return 0;
}

static command_t *get_command(char *name)
{
    if ((name = sanitize_input(name)) == NULL)
       return NULL;

    char *upper_name = (char *) malloc(strlen(name) + 1);
    strcpy(upper_name, name);
    size_t i, len = strlen(upper_name);
    for (i = 0; i < len; i++)
        upper_name[i] = toupper(upper_name[i]);

    command_t *cmd = NULL;
    for (cmd = &commands[0]; cmd->name != NULL; cmd++)
        if (strcmp(cmd->name, upper_name) == 0)
            break;

    free(upper_name);
    if (cmd->name == NULL)
        return NULL;

    return cmd;
}

//return 0 for fail, 1 for success, -1 for failed print to STDOUT
static int send_response(int code)
{
    snail_code_t *snail_resp = get_snail_resp(code);
    if (snail_resp == NULL)
    {
        if (fdprintf(STDOUT, "INVALID CODE ~d\n", code) != -1)
            return 0;
        else
            return -1;
    }

    if (code < 200)
    {
        if (fdprintf(STDOUT, "~s\n", snail_resp->desc) != -1)
            return 1;
        else
            return -1;
    }

    code = code > 999 ? code/10 : code; //hacked to support multiple responses w/single code
    if (fdprintf(STDOUT, "~d ~s\n", code, snail_resp->desc) != -1)
        return 1;
    else
        return -1;
}

static void init()
{
    size_t i;
    for (i = 0; i < MAX_MSGS; i++)
    {
        msgs[i].from = USER;
        msgs[i].num_rcpts = 0;
    }
}

static void output_sent_mail() {
    size_t i;
    for (i = 0; i < sent_msgs; i++)
    {
        fdprintf(STDOUT, "-----BEGIN MESSAGE-----\n");
        size_t j;
        for (j = 0; j < msgs[i].num_rcpts; j++)
            fdprintf(STDOUT, "Delivered-To: ~s\n", msgs[i].rcpts[j]);
        fdprintf(STDOUT, "Message-ID: <000000~d@mx.darpa.mil>\n", i+1);
        fdprintf(STDOUT, "From: <~s>\n", msgs[i].from);
#if PATCHED
        fdprintf(STDOUT, "~s", msgs[i].body);
#else
        fdprintf(STDOUT, msgs[i].body);
#endif
        fdprintf(STDOUT, "-----END MESSAGE-----\n");
    }
}

static int handle_ehlo(char *param, snail_state *state)
{
    if (*state == naught || *state == helo)
        *state = ehlo;
    return EHLO_CODE;
}

static int handle_helo(char *param, snail_state *state)
{
    if (*state == naught || *state == ehlo)
        *state = helo;
    return HELO_CODE;
}

static int handle_auth(char *param, snail_state *state)
{
    char *auth_type;
    char c64_user[MAX_USER_LEN * 2];
    char c64_pass[MAX_PASS_LEN * 2];
    char username[MAX_USER_LEN];
    char password[MAX_PASS_LEN];
    char *pc64_user, *pc64_pass;

    if ((param = sanitize_input(param)) == NULL)
        return 555;

    if (g_authenticated)
        return 5032;

    if (*state != helo && *state != ehlo)
        return 503;

    auth_type = strsep(&param, " ");
    char *upper_name = (char *) malloc(strlen(auth_type) + 1);
    strcpy(upper_name, auth_type);
    size_t i, len = strlen(upper_name);
    for (i = 0; i < len; i++)
        upper_name[i] = toupper(upper_name[i]);

    if (strcmp(upper_name, LOGIN) != 0) {
        free(upper_name);
        return 5042;
    }
    else
        free(upper_name);

    send_response(334);
    readuntil(STDIN, c64_user, sizeof(c64_user), '\n');
    pc64_user = sanitize_input(c64_user);
    if(c64_decode(username, pc64_user) == -1)
        return 501;

    send_response(3342);
    readuntil(STDIN, c64_pass, sizeof(c64_pass), '\n');
    pc64_pass = sanitize_input(c64_pass);
    if(c64_decode(password, pc64_pass) == -1)
        return 501;

    if (strcmp(username, USER) != 0 || strcmp(password, PASS) != 0)
        return 535;

    /* Successfully authenticated */
    g_authenticated = 1;
    *state = mail;
    return 235;
}

static int handle_mail(char *param, snail_state *state)
{
    /*  This doesn't actually do anything with the supplied param, except check its format
        Since it's authenticated snail, we always send from the auth'd user */
    if (!g_authenticated)
        return 530;

    if (*state != mail)
        return 503;

    if ((param = sanitize_input(param)) == NULL)
        return 555;

    strsep(&param, "<");
    if (param == NULL)
        return 555;

    strsep(&param, ">");
    if (param == NULL)
        return 555;

    *state = rcpt;
    return 250;
}

static int handle_rcpt(char *param, snail_state *state)
{
    char *rcpt_to;
    if (!g_authenticated)
        return 530;

    if (*state != rcpt && *state != data)
        return 503;

    if ((param = sanitize_input(param)) == NULL)
        return 555;

    strsep(&param, "<");
    if (param == NULL)
        return 555;

    rcpt_to = strsep(&param, ">");
    if (param == NULL)
        return 555;

    if (!valid_email(rcpt_to))
        return 553;

    if ((strlen(rcpt_to) + 1) > MAX_ADDR_LEN)
        return 5012;

    /* valid recpient */
    email_t *msg = &msgs[sent_msgs];
    if (msg->num_rcpts < MAX_RCPT)
        strcpy(msg->rcpts[msg->num_rcpts++], rcpt_to);
    else
        return 5013;

    *state = data;
    return 250;

}

static int handle_data(char *param, snail_state *state)
{
    char email_data[MAX_DATA];
    char *email_buf;
    char line[MAX_LINE];
    size_t bytes_written = 0;
    if (!g_authenticated)
        return 530;

    if (*state != data)
        return 503;

    memset(email_data, 0, MAX_DATA);
    send_response(354);
    int ret_code = 250;
    while (bytes_written < MAX_DATA - 1)    /* Account for null terminator, which isn't written */
    {
        if (readuntil(STDIN, line, sizeof(line), '\n') != 0)
        {
            send_response(5542);
            continue;
        }

        if (strcmp(line, FINISHED) == 0)
            break;

        email_buf = &email_data[bytes_written];
        if (bytes_written + strlen(line) + 1 <= MAX_DATA)
        {
            bytes_written += strlen(line);
            memcpy(email_buf, line, strlen(line));

            if (bytes_written + 1 == MAX_DATA)
                ret_code = 452;
        }
        else
        {
            send_response(5543);
        }

    }

    strcpy(msgs[sent_msgs++].body, email_data);
    *state = mail;
    return ret_code;

}

static int handle_rset(char *param, snail_state *state)
{
    if (sent_msgs == MAX_MSGS)
        return 250;

    if (*state > mail)
        *state = mail;
    else
        return 250;

    size_t i;
    email_t *msg = &msgs[sent_msgs];
    for (i = 0; i < msg->num_rcpts; i++)
        memset(msg->rcpts[i], 0, MAX_ADDR_LEN);
    msg->num_rcpts = 0;

    return 250;
}

static int handle_noop(char *param, snail_state *state)
{
    return 250;
}

static int handle_vrfy(char *param, snail_state *state)
{
    if (!g_authenticated)
        return 252;

    if (sent_msgs > 0)
    {
        output_sent_mail();
        return 250;
    }
    else
        return 252;
}

static int handle_help(char *param, snail_state *state)
{
    return 214;
}

static int handle_quit(char *param, snail_state *state)
{
    *state = quit;
    return 221;
}

int main()
{
    init();
    snail_state state = naught;
    char line[MAX_LINE];

    send_response(220);
    while (state != quit)
    {
        if (readuntil(STDIN, line, sizeof(line), '\n') != 0)
        {
            send_response(5542);
            continue;
        }

        command_t *cmd;
        char *cmd_name, *param = line;

        cmd_name = strsep(&param, ":");
        if (param == NULL || ((cmd = get_command(cmd_name)) == NULL))
        {
            param = line;

            cmd_name = strsep(&param, " ");
            if ((cmd = get_command(cmd_name)) == NULL)
            {
                send_response(502);
                continue;
            }
        }


        if (sent_msgs == MAX_MSGS)
        {
            if (strcmp(cmd->name, NOOP) != 0 && strcmp(cmd->name, QUIT) != 0 &&
                strcmp(cmd->name, RSET) != 0 && strcmp(cmd->name, VRFY) != 0)
            {
                send_response(451);
                continue;
            }
            else if (send_response(cmd->handler(param, &state)) == -1)
                break;
        }
        else if (send_response(cmd->handler(param, &state)) == -1)
            break;

    }

    return 0;
}
