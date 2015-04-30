#ifndef PRINTF_H
#define PRINTF_H

#include <libcgc.h>

void printf_setFlushOnNewline(int bEnable);

/** Writes a string as formatted by the format string fstr to the file described by 
 *    file descriptor fd. The following format specifiers are supported
 *    %% : Prints out a % symbol - no args
 *    %x : 32bit hex output - uint32_t 
 *    %X : 32bit hex output but with upper case letters for A-F - uint32_t
 *    %d : 32bit decimal output - int32_t
 *    %D : 64bit decimal output - int64_t
 *    %u : 32bit unsigned decimal output (i.e. uint32_t) - uint32_t
 *    %U : 64 bit unsigned decimal output - uint64_t
 *    %s : string output - const char*
 *    %n : writes the number of bytes written thus far to a memory location - uint16_t*
 *    %N : same as %n, but the counter is 4 bytes long and not just 2 - uint32_t*
 *   The following escape sequences are also supported
 *    \\ : Backslask character
 *    \n : newline character
 *    \t : tab character
 *    \' : single quote
 *    \" : double quote
**/
size_t fprintf(int fd, const char* fstr, ...);
size_t printf(const char* fstr, ...);
size_t fflush(int fd);

size_t snprintf(char* str, size_t len, const char* fstr, ...);
 
#endif//PRINTF_H
