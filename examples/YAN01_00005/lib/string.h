#ifndef STRING_H
#define STRING_H

#include <libcgc.h>
#include "inttypes.h"

/** Functiont o make a copy of a string up to (len - 1) characters long. 
 *    '\0' is appended at the end!!
**/
size_t strncpy(char* dst, size_t len, const char* src);

/** A blind string copy - uses strncpy with SIZE_MAX **/
size_t strcpy(char* dst, const char* src);

/** Compares two strings using < , > and = for char. 
 *    0 : Equal
 *    -1 : l is less than r
 *    1 : 1 is > r
**/
int strcmp(const char* l, const char* r);

/** Finds the first needle substring in the haystack string
 *    Returns a pointer to the beginning of needle in haystack
**/
char* strstr(char* haystack, const char* needle);

/** Finds first occurrence of needle in haystack.
 *    Returns a pointer to the char in the haystack.
**/
char* strchr(char* haystack, char needle);

/** Same as strstr but searches for last needle. Uses strstr.
**/
char* strrstr(char* haystack, const char* needle);

/** Same as strchr but searches for the last occurrence.
**/
char* strrchr(char* haystack, char needle);

/** Returns the length of a cstring 
**/
size_t strlen(const char* str);

/** Converts a string number into an uint32_t 
 *  A return value of 0 could indicate an error as well as the integral value of 0
 *  The number can be terminated by '\0', '\n', '.' or ',' and must be 10 characters or less
**/
uint32_t strToUint32(const char* str);
#endif//STRING_H
