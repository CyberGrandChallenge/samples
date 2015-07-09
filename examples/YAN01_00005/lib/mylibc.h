#ifndef MY_LIB_C_H
#define MY_LIB_C_H

#include "inttypes.h"
#include "string.h"
#include "printf.h"

//reads in a new line and returns the number of characters read
// Returns the number of characters read otherwise it will return 0
// or an errno on error
// errno can be
// The errno from fdwait or receive
// it can also be -EINVAL if buf is NULL.
// it can also be -EPIPE if EOF -- Notice that EPIPE is not used by either fdwait or receive
ssize_t readLine(int fd, char* buf, size_t len, int bTerm);

#endif//MY_LIB_C_H
