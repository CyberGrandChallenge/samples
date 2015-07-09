#include <libcgc.h>
#include "stdarg.h"
#include "printf.h"
#include "inttypes.h"

#define PRINTF_BUF_SIZE 200

#define _PRINTF_BUF_OVERFLOW_SIZE 56
#define _PRINTF_BUF_SIZE (PRINTF_BUF_SIZE + _PRINTF_BUF_OVERFLOW_SIZE)

typedef struct _PrintfInfo
{
  int fd;
  char* buf;  
  size_t bufSize; 
  size_t end; 
  size_t count;
  struct _PrintfInfo*  next; 
} PrintfInfo;

#define INIT_PRINTF_INFO(_pInfo, _fd, _buf)	\
do						\
{						\
  (_pInfo)->fd = _fd;				\
  (_pInfo)->buf = _buf;				\
  (_pInfo)->bufSize = PRINTF_BUF_SIZE;		\
  (_pInfo)->end = 0;				\
  (_pInfo)->count = 0;				\
  (_pInfo)->next = NULL;			\
} while(0)					


PrintfInfo* gPrintfHead = NULL;
int gbFlushOnNewline = 0;

void printf_setFlushOnNewline(int bEnabled)
{
  gbFlushOnNewline = bEnabled;
}

/** Returns a pointer to the PrintfInfo struct that contains the buffer
 *    for the current fd. A new struct is created if it doesn't exist.
 *    Note that this function always creates a new one. A different
 *    version that only searches might be a good idea as well.
**/
static PrintfInfo* getPrintfInfo(int fd)
{
  PrintfInfo** pTemp = &gPrintfHead;

  if (*pTemp != NULL)
  {
    do
    {
      if ( (*pTemp)->fd == fd)
      {
        return (*pTemp);
      }
      pTemp = &((*pTemp)->next);
    } while (*pTemp != NULL);
  }
 
  if (allocate(sizeof(PrintfInfo) + _PRINTF_BUF_SIZE, 0, (void**)pTemp) != 0)
  {
    return (NULL);
  }

  INIT_PRINTF_INFO(*pTemp, fd, (char*)(*pTemp) + sizeof(PrintfInfo));

  return (*pTemp);
}

/** Flushes the corresponding buffer by calling transmit. 
 *    Note that this function does not handle transmit not
 *    transmitting the whole buffer. This can be done by
 *    adding another start field in the data structure and
 *    then moving the start out to the return value of rx_bytes
 *    but that seemed unnecessary right now.
**/

static size_t pflush(PrintfInfo* pInfo)
{
  size_t rx_bytes = 0;
  int ret = 0;

  if (pInfo == NULL)
  {
    return (-1);
  }

  //if empty
  if ( (pInfo->end == 0) || (pInfo->fd == -1) )
  {
    return (0);
  }

#ifdef PATCHED
  do {
    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(pInfo->fd, &writefds);
    ret = fdwait(pInfo->fd + 1,NULL,&writefds,NULL,NULL);
    if (ret != 0)
        _terminate(0);
#endif
    ret = transmit(pInfo->fd, pInfo->buf, pInfo->end, &rx_bytes);
    if ( (ret == 0) && (rx_bytes != 0) )
    {
      pInfo->count += pInfo->end;
      pInfo->end = 0;
    }
#ifdef PATCHED
    if (ret != 0) {
        _terminate(1);
    }
  } while (pInfo->end >= pInfo->bufSize);
#endif

  return (rx_bytes);
}

/** Exported function to flush a file descriptor's buffer
 *    uses pflush() internally
**/
size_t fflush(int fd)
{
  size_t rx_bytes = 0;
  int ret = 0;

  PrintfInfo* pInfo = getPrintfInfo(fd);
  if (pInfo == NULL)
  {
    return (-1);
  }

  return(pflush(pInfo));
}

#define _PUT_CHAR(_pInfo, _c) (_pInfo)->buf[(_pInfo)->end++] = (_c)

size_t printfProcessString(PrintfInfo* pInfo, const char* str)
{
  size_t ret = 0;
  size_t i = 0;

  if (pInfo == NULL)
  {
    return (-1);
  }
  if (str == NULL)
  {
    return (0);
  }

  for (i = 0; str[i] != '\0'; i++)
  {
    _PUT_CHAR(pInfo, str[i]);
    if (pInfo->end >= pInfo->bufSize)
    {
      if (pInfo->fd == -1) //i.e. its snprintf
      {
        return (pInfo->bufSize);
      }
      ret += pflush(pInfo);
    }
  }   
 
  if (pInfo->fd == -1)
  {
    return (i);
  }

  return (ret);
}

static inline char _nibbleToHexChar(uint8_t nib, int bUpcase)
{
  nib &= 0xF;
  if ( nib <= 9)
  {
    return (nib + '0');
  }
  else
  {
    return ( (nib - 0xA) + (bUpcase ? 'A' : 'a'));
  }
  return (0); //should not be here
}

//Assumes that str is NOT NULL!! This is why its not exported
static size_t _byteToHexStr(char* str, uint8_t b, int bLeadingZeroes, int bUpcase)
{
  uint8_t upper = (b >> 4) & 0xF;    
  uint8_t lower = b & 0xF;

  str[0] = _nibbleToHexChar(upper, bUpcase); 
  if (!bLeadingZeroes)
  {
    //if we don't want leading zeroes and the upper nibble is zero then overwrite it
    if (str[0] == '0')
    {
      str[0] = _nibbleToHexChar(lower, bUpcase); 
      //if it is still zero then just return 0 as if nothing happened
      if (str[0] == '0')
      {
        return (0);
      }
      return (1);
    }
  }

  str[1] = _nibbleToHexChar(lower, bUpcase);
  return (2); 
}

static size_t uintToHexStr(char* str, void* opaque, int size, int bLeadingZeroes, int bUpcase)
{
  if ( (str == NULL) || (opaque == NULL) )
  {
    return (-1);
  }

  size_t curOffset = 0;
  size_t ret = 0;
  uint64_t temp = 0;

  switch(size)
  {
    case (sizeof(uint8_t)):
    {
      uint8_t* pTemp = (uint8_t*)opaque;
      temp = *pTemp;
      ret = _byteToHexStr(str + curOffset, *pTemp, bLeadingZeroes || ret, bUpcase);
      curOffset += ret;
      break;
    }  
    case (sizeof(uint16_t)):
    {
      uint16_t* pTemp = (uint16_t*)opaque;
      temp = *pTemp;
      ret = _byteToHexStr(str + curOffset, (*pTemp >> 8) & 0xFF, bLeadingZeroes || ret, bUpcase);
      curOffset += ret;
      //there are two cases when we need leading zeroes, 1 is if the user says so
      // and two, if the user doesn't say so, but we already printed stuff before
      ret = _byteToHexStr(str + curOffset, *pTemp & 0xFF, bLeadingZeroes || ret, bUpcase);
      curOffset += ret;
      break;
    }  
    case (sizeof(uint32_t)):
    {
      uint32_t* pTemp = (uint32_t*)opaque;
      temp = *pTemp;
      ret = _byteToHexStr(str + curOffset, (*pTemp >> 24) & 0xFF, bLeadingZeroes || ret, bUpcase);
      curOffset += ret;
      ret = _byteToHexStr(str + curOffset, (*pTemp >> 16) & 0xFF, bLeadingZeroes || ret, bUpcase);
      curOffset += ret;
      ret = _byteToHexStr(str + curOffset, (*pTemp >> 8) & 0xFF, bLeadingZeroes || ret, bUpcase);
      curOffset += ret;
      ret = _byteToHexStr(str + curOffset, *pTemp & 0xFF, bLeadingZeroes || ret, bUpcase);
      curOffset += ret;
      break;
    }  
    case (sizeof(uint64_t)):
    {
      uint64_t* pTemp = (uint64_t*)opaque;
      temp = *pTemp;
      ret = _byteToHexStr(str + curOffset, (*pTemp >> 56) & 0xFF, bLeadingZeroes || ret, bUpcase);
      curOffset += ret;
      ret = _byteToHexStr(str + curOffset, (*pTemp >> 48) & 0xFF, bLeadingZeroes || ret, bUpcase);
      curOffset += ret;
      ret = _byteToHexStr(str + curOffset, (*pTemp >> 40) & 0xFF, bLeadingZeroes || ret, bUpcase);
      curOffset += ret;
      ret = _byteToHexStr(str + curOffset, (*pTemp >> 32) & 0xFF, bLeadingZeroes || ret, bUpcase);
      curOffset += ret;
      ret = _byteToHexStr(str + curOffset, (*pTemp >> 24) & 0xFF, bLeadingZeroes || ret, bUpcase);
      curOffset += ret;
      ret = _byteToHexStr(str + curOffset, (*pTemp >> 16) & 0xFF, bLeadingZeroes || ret, bUpcase);
      curOffset += ret;
      ret = _byteToHexStr(str + curOffset, (*pTemp >> 8) & 0xFF, bLeadingZeroes || ret, bUpcase);
      curOffset += ret;
      ret = _byteToHexStr(str + curOffset, *pTemp & 0xFF, bLeadingZeroes || ret, bUpcase);
      curOffset += ret;
      break;
    }  
    default: //not supported
    {
      return (0);
    }
  }

  //at this point if the number is 0 and bLeadingZeroes is also 0 then that means we might
  // not have output any characters, need to fix that
  if ( (temp == 0) && (!bLeadingZeroes) )
  {
    str[0] =  '0';
    curOffset = 1;
  }
  return (curOffset);
}

size_t uint8ToHexStr(char* str, uint8_t ui, int bLeadingZeroes, int bUpcase)
{
  return (uintToHexStr(str, (void*)(&ui), sizeof(uint8_t), bLeadingZeroes, bUpcase));
}

size_t uint16ToHexStr(char* str, uint16_t ui, int bLeadingZeroes, int bUpcase)
{
  return (uintToHexStr(str, (void*)(&ui), sizeof(uint16_t), bLeadingZeroes, bUpcase));
}

size_t uint32ToHexStr(char* str, uint32_t ui, int bLeadingZeroes, int bUpcase)
{
  return (uintToHexStr(str, (void*)(&ui), sizeof(uint32_t), bLeadingZeroes, bUpcase));
}

size_t uint64ToHexStr(char* str, uint64_t ui, int bLeadingZeroes, int bUpcase)
{
  return (uintToHexStr(str, (void*)(&ui), sizeof(uint64_t), bLeadingZeroes, bUpcase));
}

#if 0
/** A function that uses double precision floating point math and and 64bit multiplication
 *    to perform 64bit division and mod. This is needed because the appropriate functions
 *    are missing in CGC
**/
uint64_t uint64Div(uint64_t dividend, uint64_t divisor, uint64_t* rem)
{
  double d1 = (double)dividend;
  double d2 = (double)divisor;

  uint64_t res = (uint64_t)(d1 / d2);

  if (rem != NULL)
  {
    *rem = d1 - (d2 * res);
  }

  return (res);
}
#endif

uint64_t my_div(uint64_t dend, uint32_t dsor, uint32_t* rem)
{
  uint32_t upper = (uint32_t)(dend >> 32);
  uint32_t lower = (uint32_t)(dend & 0xFFFFFFFF);

  uint64_t d0 = (uint64_t)(upper / dsor);
  uint64_t r0 = (uint64_t)(upper % dsor);

  d0 = d0 << 32;
  
  uint64_t d2 = (uint64_t)(lower / dsor);
  uint64_t r2 = (uint64_t)(lower % dsor);

  uint64_t d1 = (uint64_t)( (0x1 << 31) / dsor ) << 1;
  uint64_t r1 = (uint64_t)( (0x1 << 31) % dsor ) << 1; 

  //adjust the upper divisor to include remainder multiplied by the divisor
  // from 2^32 / dsor 
  d0 = d0 + r0 * d1;
  //add in the lower divisor
  d0 = d0 + d2;

  //now we add up all of the remainders and try again
  //adjust the upper remainder by multiplying it by the remainder from 2^32 / dsor
  r0 = r0 * r1;
  //add in the one from the lower remainder 
  r0 = r0 + r2;

  //at this point r0 should be at most dsor^2 + dsor so lets make the proper adjustments -- we can use a recursive call for this purpose
  if (r0 > dsor)
  {
    uint32_t temp = 0;
    d0 += my_div(r0, dsor, &temp);
    r0 = temp;
  }
  
  *rem = (uint32_t)r0;
  return (d0);
}

static size_t _intToDecStr(char* str, void* opaque, int size, int isSigned)
{
  int i = 0;
  int j = 0;
  //uint64_t rem = 0;
  uint32_t rem = 0;
  char temp[24]; //the maximum uint64_t value is 2^64 - 1
                 // which is approx 10^20 so 24 characters should be good

  uint64_t num = 0;
  int bIsNeg = 0;

  if (isSigned)
  {
    switch(size)
    {
      case(sizeof(int8_t)):
      {
        if (*((int8_t*)opaque) < 0)
        {
          bIsNeg = 1;
          num = -(*((int8_t*)opaque));
        }
        else
        {
          num = *((int8_t*)opaque);
        }
        break;
      }
      case(sizeof(int16_t)):
      {
        if (*((int16_t*)opaque) < 0)
        {
          bIsNeg = 1;
          num = -(*((int16_t*)opaque));
        }
        else
        {
          num = *((int16_t*)opaque);
        }
        break;
      }
      case(sizeof(int32_t)):
      {
        if (*((int32_t*)opaque) < 0)
        {
          bIsNeg = 1;
          num = -(*((int32_t*)opaque));
        }
        else
        {
          num = *((int32_t*)opaque);
        }
        break;
      }
      case(sizeof(int64_t)):
      {
        if (*((int64_t*)opaque) < 0)
        {
          bIsNeg = 1;
          num = -(*((int64_t*)opaque));
        }
        else
        {
          num = *((int64_t*)opaque);
        }
        break;
      }
      default:
      {
        return (-1);
      }
    }
  }
  else
  {
    switch(size)
    {
      case(sizeof(uint8_t)):
      {
        num = *((uint8_t*)opaque);
        break;
      }
      case(sizeof(uint16_t)):
      {
        num = *((uint16_t*)opaque);
        break;
      }
      case(sizeof(uint32_t)):
      {
        num = *((uint32_t*)opaque);
        break;
      }
      case(sizeof(uint64_t)):
      {
        num = *((uint64_t*)opaque);
        break;
      }
      default:
      {
        return (-1);
      }
    }
  }

  if (num == 0)
  {
    str[0] = '0';
    return (1);
  }

  while (num != 0)
  {
    //rem = num % 10; //get the remainder
    //had to implement my own Div and % because __umoddi3 and __udivdi3 are not defined
    //num = uint64Div(num, 10, &rem);
    num = my_div(num, 10, &rem);
    temp[i] = '0' + rem;
    //num = num / 10; //divide it by 10 
    i++;
  }

  if (bIsNeg)
  {
    str[0] = '-';
  }

  for (j = 0; j < i; j++)
  {
    str[j + bIsNeg] = temp[i - j - 1]; 
  }  
  return (j + bIsNeg);
}

static inline size_t uintToDecStr(char* str, void* opaque, int size)
{
  return (_intToDecStr(str, opaque, size, 0));  
}

size_t uint8ToDecStr(char* str, uint8_t ui)
{
  return (uintToDecStr(str, (void*)(&ui), sizeof(uint8_t)));
}

size_t uint16ToDecStr(char* str, uint16_t ui)
{
  return (uintToDecStr(str, (void*)(&ui), sizeof(uint16_t)));
}

size_t uint32ToDecStr(char* str, uint32_t ui)
{
  return (uintToDecStr(str, (void*)(&ui), sizeof(uint32_t)));
}

size_t uint64ToDecStr(char* str, uint64_t ui)
{
  return (uintToDecStr(str, (void*)(&ui), sizeof(uint64_t)));
}

static inline size_t intToDecStr(char* str, void* opaque, int size)
{
  return (_intToDecStr(str, opaque, size, 1));
}

size_t int8ToDecStr(char* str, int8_t i)
{
  return (intToDecStr(str, (void*)(&i), sizeof(int8_t)));
}

size_t int16ToDecStr(char* str, int16_t i)
{
  return (intToDecStr(str, (void*)(&i), sizeof(int16_t)));
}

size_t int32ToDecStr(char* str, int32_t i)
{
  return (intToDecStr(str, (void*)(&i), sizeof(int32_t)));
}

size_t int64ToDecStr(char* str, int64_t i)
{
  return (intToDecStr(str, (void*)(&i), sizeof(int64_t)));
}

static size_t _pprintf(PrintfInfo* pInfo, const char* fstr, va_list args)
{
  size_t ret = 0;
  size_t tempRet = 0;
  
  if (fstr == NULL)
  {
    return (0);
  }

  if (pInfo == NULL)
  {
    return (0);
  }

  size_t i = 0;
  size_t nonFormatCount = 0;
  int bLeadingZeroes = 0;
  while (fstr[i] != '\0')
  {
    switch(fstr[i])
    {
      case('$'):
      {
        i++;
        if (fstr[i] == '0')
        {
          bLeadingZeroes = 1;
          i++;
        }

        switch(fstr[i])
        {
          case('$'):
          {
            _PUT_CHAR(pInfo, '$');
            nonFormatCount++;
            i++;
            break;
          }
          case('x'):
          {
            tempRet = uint32ToHexStr(pInfo->buf + pInfo->end, va_arg(args, uint32_t), bLeadingZeroes, 0); 
            pInfo->end += tempRet;
            ret += tempRet;
            i++;
            break;  
          }
          case('X'):
          {
            tempRet = uint64ToHexStr(pInfo->buf + pInfo->end, va_arg(args, uint64_t), bLeadingZeroes, 1);
            pInfo->end += tempRet;
            ret += tempRet; 
            i++;
            break;  
          }
          case('u'):
          {
            tempRet = uint32ToDecStr(pInfo->buf + pInfo->end, va_arg(args, uint32_t));
            pInfo->end += tempRet;
            ret += tempRet; 
            i++;
            break;
          }
          case('U'):
          {
            tempRet = uint64ToDecStr(pInfo->buf + pInfo->end, va_arg(args, uint64_t));
            pInfo->end += tempRet;
            ret += tempRet; 
            i++;
            break;
          }
          case('d'):
          {
            tempRet = int32ToDecStr(pInfo->buf + pInfo->end, va_arg(args, uint32_t));
            pInfo->end += tempRet;
            ret += tempRet; 
            i++;
            break;
          }
          case('D'):
          {
            tempRet = int64ToDecStr(pInfo->buf + pInfo->end, va_arg(args, uint64_t));
            pInfo->end += tempRet;
            ret += tempRet; 
            i++;
            break;
          }
          case('s'):
          {
            //pInfo->end is already updated by printfProcessString
            tempRet = printfProcessString(pInfo, va_arg(args, const char*));
            ret += tempRet; 
            i++;
            break;
          }
          case('n'):
          {
            *(va_arg(args, uint16_t*)) = (uint16_t)(pInfo->count + pInfo->end);
            break;
          }
          case('N'):
          {
            *(va_arg(args, uint32_t*)) = (uint32_t)(pInfo->count + pInfo->end);
            break;
          }
          default:
          {
            _PUT_CHAR(pInfo, '$');
            nonFormatCount++;
            //we don't increment i since we didn't consume the char after %
          }
        }
        break;
      }
      case('\\'):
      {
        i++;
        switch(fstr[i])
        {
          case ('n'):
          {
            _PUT_CHAR(pInfo, '\n');
            nonFormatCount++;
            i++;
            break;
          }
          case ('\\'):
          {
            _PUT_CHAR(pInfo, '\\');
            nonFormatCount++;
            i++;
            break;
          }
          case ('t'):
          {
            _PUT_CHAR(pInfo, '\t');
            nonFormatCount++;
            i++;
            break;
          }
          case ('\"'):
          {
            _PUT_CHAR(pInfo, '\"');
            nonFormatCount++;
            i++;
            break;
          }
          case ('\''):
          {
            _PUT_CHAR(pInfo, '\'');
            nonFormatCount++;
            i++;
            break;
          }
          default:
          {
            _PUT_CHAR(pInfo, '\\');
            nonFormatCount++;
            break;
          }
        }
        break;
      }
      default:
      {
        _PUT_CHAR(pInfo, fstr[i]);
        nonFormatCount++;
        i++;
        if (gbFlushOnNewline && (fstr[i-1] == '\n'))
        {
          ret += pflush(pInfo);
        }
        break;
      }
    }
    if (pInfo->end >= pInfo->bufSize)
    {
      ret += pflush(pInfo);
    }
  }
  
  if (pInfo->fd == -1) //if snprintf
  {
    return (ret + nonFormatCount);
  }

  return (ret);
}

static size_t _fprintf(int fd, const char* fstr, va_list args)
{
  PrintfInfo* pInfo = NULL;
  pInfo = getPrintfInfo(fd);
  return (_pprintf(pInfo, fstr, args));
}

size_t snprintf(char* str, size_t len, const char* fstr, ...)
{
  size_t ret = 0;
  PrintfInfo printfInfo;

  va_list args;
  va_start(args, fstr);

  INIT_PRINTF_INFO(&printfInfo, -1, str);
  printfInfo.bufSize = len;

  ret = _pprintf(&printfInfo, fstr, args);

  va_end(args);
 
  return (ret);
}

size_t fprintf(int fd, const char* fstr, ...)
{
  size_t ret = 0;
  va_list args;
  va_start(args, fstr);

  ret = _fprintf(fd, fstr, args);

  va_end(args);

  return (ret);
}

size_t printf(const char* fstr, ...)
{
  size_t ret = 0;
  va_list args;
  va_start(args, fstr);

  ret = _fprintf(STDOUT, fstr, args);

  va_end(args);

  return (ret);
}

#undef _PRINTF_BUF_OVERFLOW_SIZE
#undef _PRINTF_BUF_SIZE
