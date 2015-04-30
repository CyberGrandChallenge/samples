#ifndef SNAIL_CODES_H_
#define SNAIL_CODES_H_

typedef struct {
    int code;
    char *desc;
} snail_code_t;

struct scode_list {
    struct scode_list *next;
    snail_code_t *scode;
};

typedef enum { naught, ehlo, helo, mail, rcpt, data, quit } snail_state;

#define EHLO_CODE ehlo
#define EHLO_RESP "250-mx.darpa.mil at your service\n"\
                  "250-8BITMIME\n"\
                  "250 AUTH LOGIN"

#define HELO_CODE helo
#define HELO_RESP "250 mx.darpa.mil at your service"

snail_code_t *get_snail_resp(int code);

#endif
