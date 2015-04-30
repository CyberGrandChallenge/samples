#include <libcgc.h>
#include "mylibc.h"


/***********************************************************/
/**               LIBC related functions                  **/
/***********************************************************/

//Continues to read into a buffer until '\n' is read in
// this is the problem
//inline should make this easier if needed since then we will remove
// a function boundary
//Question is whether the compiler will rearrange certain variables
// It looks okay when I inlined it in my test - but that does not
// necessarily mean that it is indeed good.
ssize_t readLine(int fd, char* buf, size_t len, int bTerm)
{
  char c = '\0';
  int ret = 0;
  size_t i = 0;
  size_t numRead;

  if (buf == NULL)
  {
    return (-EINVAL);
  }

  //loop until the character is read
  for (i = 0; i < len; i++)
  {
    ret = receive(fd, &c, sizeof(char), &numRead);
    if ( (ret != 0) )
    {
      if (bTerm)
      {
        _terminate(bTerm);
      }
      //since receive also returns errno, we can just pass it back
      return (-ret); 
    }
    if ( numRead == 0 ) //if EOF
    {
      if (bTerm)
      {
        _terminate(bTerm);
      }
      return (-EPIPE);
    }

    if (c == '\n')
    {
      buf[i] = '\0';
      break;
    }

    buf[i] = c; 
  }
 
  if (i == len)
  {
    buf[i - 1] = '\0';
  }

  return (i - 1);
}

void readAll(int fd, void* buf, size_t len) {
   size_t b = 0;
   size_t n;
   while (b < len) {
      if (receive(fd, buf + b, len - b, &n) != 0 || n == 0) {
         _terminate(1);
      }
      b += n;
   }
}

size_t myStrLen(const char* str)
{
  size_t i = 0;

  if (str != NULL)
  {
    while (str[i] != '\0')
    {
      i++;
    }
  }

  return (i);
}

size_t strncpy(char* dst, size_t len, const char* src)
{
  size_t i = 0;
  if ( (src == NULL) || (dst == NULL) || (len == 0) )
  {
    return (0);
  }

  for (i = 0; (i < len) && (src[i] != '\0'); i++)
  {
    dst[i] = src[i];
  }
  if (i == len)
  {
    dst[i - 1] = '\0';
  }
  else 
  {
    dst[i] = '\0';
  }
  return (i);
}

size_t my_printf(const char* str) {
  if (str == NULL)
  {
    return (-1);
  }
   const char *end = str + myStrLen(str);
   const char *b = str;
   size_t n;
   while (b < end) {
      int r = transmit(0, b, end - b, &n);
      if (r != 0 || n == 0) {
         //error we can't continue
         _terminate(1);
      }
      b += n;
   }
   return end - str;
}

//minlen is used for leading 0's in the fractional part
int snprintdecimal(char* str, size_t len, uint32_t num, int minlen)
{
  int i = 0;
  int j = 0;
  uint32_t rem = 0;
  char temp[24]; //the maximum uint64_t value is 2^64 - 1
                 // which is approx 10^20 so 24 characters should be good
  temp[0] = '0';
  temp[0] = '\0';

  if ( (str == NULL) || (len < 2) || (len <= minlen) )
  {
    return (-1);
  }

  if (num == 0)
  {
    str[0] = '0';
    str[1] = '\0';
    return (1);
  }

  while (num != 0)
  {
    rem = num % 10; //get the remainder
    temp[i] = '0' + rem;
    num = num / 10; //divide it by 10 
    i++;
  }

  //fill in the leading zeroes
  for (j = 0; j < (minlen - i); j++)
  {
    str[j] = '0';
  }
 
  //now see if everything fits
  if (i < len)
  {
    //remember this is NOT a null terminated string
    i--; // decrement i first to get to the last character written
    while (i >= 0)
    {
      str[j] = temp[i]; 
      j++; //move j up
      i--; //move i down
    }  
    str[j] = '\0';
    return (j);
  }

  //now lets end the string by setting the NULL character
  return (-1); 
}

int snprintfloat(char* str, size_t len, float num)
{
  char tempStr[24];
  uint32_t iNum = 0;
  iNum = (uint32_t)num;
  int ret = 0;

  tempStr[0] = '0';
  tempStr[0] = '\0';
  //it seems that casting is the FLOOR function by default
  // do we need to enforce this?

  ret = snprintdecimal(str, len, iNum, 0);
  if (ret < 0)
  {
    return (ret);
  }

  if ( (ret + 1) < len)
  {
    str[ret] = '.';
    str[ret + 1] = '\0';

    iNum = (uint32_t)((num - (float)iNum) * (float)1000000.0);//get the next 6 numbers
    return (snprintdecimal(str+ret+1, len - ret - 1, iNum, 6)); 
  }
  return (-1);
}


/**
 * The maximum unsigned value is 4294967295 which is 0xFFFFFFFF
 * So the idea is to read in up to 9 digits and then convert that
 * into a uint64_t. We need a uint64_t because 9999999999 will overflow
 * uint32_t. 
 *
 * A 0 is returned - either because that is the actual number or because
 * the first 9 characters are not digits (NULL, '.' and ',' are all
 * terminal characters). Any subsequent characters are
 * simply ignored.
*/
uint32_t strToUint32(const char* str)
{
  int i = 0; 
  uint64_t val = 0; 
  uint32_t temp = 0;

  if (str == NULL)
  {
    return (0);
  }

  //the maximum unsigned value is 4294967295 which is 0xFFFFFFFF
  while ( (str[i] != '\0') && (str[i] != '\n') && (str[i] != '.') && (str[i] != ',') && (i < 10) )
  {
    if ( (str[i] >= '0') &&  (str[i] <= '9') )
    {
      temp = str[i] - '0';
      val *= 10;
      val += temp;
    }
    else //error - just return 0
    {
      return (0);
    }
    i++;
  }

  //should never be here
  if (val > 0xFFFFFFFFull)
  {
    return (0);
  }

  return ((uint32_t)val); //just cast it and return
}

/**
 * To convert a string into a double - we will do the following
 * go through the array to find the first '.' or ','
 * everything before the '.' will be converted into a uint32_t
 * and everything after the '.' will be converted into a uint32_t
 * We will then just calculate the double value by first converting
 * them into double and then using floating point
 * division and addition. Note that, there is indeed some funny
 * behavior since the number before the '.' or ',' can indeed
 * be over uint32_t. In that case, it is treated as 0xFFFFFFFF
 * same applies to the fractional part.
 * -0.0 is returned on error
**/
double strToDouble(const char* str)
{
  size_t i = 0;
  size_t fracStart = 0; 

  int bWhole = 1; //yes whole part

  uint32_t whole = 0;
  uint32_t frac = 0;
  uint32_t val = 0;
  uint32_t temp = 0;


  if (str == NULL)
  {
    return (NAN.d); 
  }

  //loop through the string and separate the number by either . or ,
  while ( (str[i] != '\0') && (str[i] != '\n') )
  {
    if ( (str[i] == '.') || (str[i] == ',') )
    {
      //if we were processing whole then switch to processing frac
      if (bWhole)
      {
        bWhole = 0;
        fracStart = i + 1;
        whole = val; 
        temp = 0;
        val = 0;
      }
      else //if we are already processing frac then error
      {
        return (NAN.d);
      }
    }
    else //see if there are already too many digits
    {
      if ( bWhole && (i >= 10) )
      {
        return (NAN.d);
      }
      else if ( !bWhole && ((i - fracStart) >= 9) ) //we can have an error here if we change 9 to 10 - in that case we get whatever is in memory at that location
      {
        return (NAN.d);
      }
 
      if ( (str[i] >= '0') &&  (str[i] <= '9') )
      {
        temp = str[i] - '0';
        val *= 10;
        val += temp;
      }
      else //error - just return 0
      {
        return (NAN.d);
      }
    }

    i++;
  }   

  //if we are here then we have reached the end of the string
  if (bWhole)
  {
    whole = val;
  }
  else
  {
    frac = val;
  }

  //now that we have both sections lets calculate and return the double
  return ( (double)whole + ( (double)frac / POWERS_OF_TEN[i - fracStart] ) );
  
  //something very curious - CLANG defaults to SSE instructions if available and
  // then uses x87 as a fallback. On the other hand, if we use long double instead
  // of double then CLANG will use the x87 instructions instead. 
  // To keep things consistent, we might want to just use long double 
}
