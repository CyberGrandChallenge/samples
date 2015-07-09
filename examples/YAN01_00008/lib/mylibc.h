#ifndef MY_LIB_C_H
#define MY_LIB_C_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

static double POWERS_OF_TEN[10] = {
  1e0,
  1e1,
  1e2,
  1e3,
  1e4,
  1e5,
  1e6,
  1e7,
  1e8,
  1e9,
};

//I have to define my own NAN constant in this case
// this is the error that is returned if conversion doesn't work
typedef union _DoubleInt
{
  double d;
  uint64_t u;
} DoubleInt;

static DoubleInt NAN = {.u = 0xFFFFFFFFFFFFFFFFull};

//reads in a new line and returns the number of characters read
// Returns the number of characters read otherwise it will return 0
// or an errno on error
// errno can be
// The errno from fdwait or receive
// it can also be -EINVAL if buf is NULL.
// it can also be -EPIPE if EOF -- Notice that EPIPE is not used by either fdwait or receive
ssize_t readLine(int fd, char* buf, size_t len, int bTerm);
size_t myStrLen(const char* str);
size_t strncpy(char* dst, size_t len, const char* src);
size_t my_printf(const char* str);
int snprintdecimal(char* str, size_t len, uint32_t num, int minlen);
int snprintfloat(char* str, size_t len, float num);
uint32_t strToUint32(const char* str);
double strToDouble(const char* str);
void readAll(int fd, void* buf, size_t len);

#endif//MY_LIB_C_H
