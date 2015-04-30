#include "string.h"

size_t strcpy(char* dst, const char* src)
{
  return (strncpy(dst, SIZE_MAX, src));
}

size_t strncpy(char* dst, size_t len, const char* src)
{
  size_t i = 0; 
  if ( (dst == NULL) || (src == NULL) || (len == 0) )
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

int strcmp(const char* l, const char* r)
{
  size_t i = 0; 

  if (l == NULL) 
  {
    if (r == NULL) 
    {
      return (0);
    }
    else
    {
      return (-1);
    }
  }
  else if (r == NULL)
  {
    return (1);
  }

  for (i = 0; (l[i] != '\0') && (r[i] != '\0'); i++)
  {
    if (l[i] < r[i])
    {
      return (-1);
    }
    else if (l[i] > r[i])
    {
      return (1);
    }
  }

  if (l[i] == '\0')
  {
    if (r[i] == '\0')
    {
      return (0);
    }
    else
    {
      return (-1);
    }
  }
  else
  {
    return (1);
  }
}

char* strstr(char* haystack, const char* needle)
{
  size_t i = 0;
  size_t j = 0;

  if ( (needle == NULL) || (haystack == NULL) )
  {
    return (NULL);
  }

  for (i = 0; haystack[i] != '\0'; i++)
  {
    for (j = 0; (haystack[i + j] != '\0') && (needle[j] == haystack[i + j]); j++)
    {
      //empty
    }
    //if we are there then either haystack[i + j] is NULL
    if (haystack[i + j] == '\0')
    {
      if (needle[j] == '\0')
      {
        return (haystack + i);
      }
      else
      {
        return (NULL);
      }
    }
    // OR needle[j] != haystack[i + j] -- this could happen only if needle[j] is NULL
    // and haystack[i + j] is not - meaning we found it, or if they are different
    // but there is more to search
    else if (needle[j] == '\0')
    {
      return (haystack + i);
    }
  }
  
  return (NULL);
}

//lazy implementation
char* strrstr(char* haystack, const char* needle)
{
  char* ret = NULL;
  char* last = NULL;
  size_t i = 0;

  do
  {
    ret = last;
    last = strstr(haystack + i, needle);
    i++;
  } while (last != NULL);

  return (ret);
}

char* strchr(char* haystack, char needle)
{
  if (haystack != NULL)
  {
    for (size_t i = 0; haystack[i] != '\0'; i++)
    {
      if (haystack[i] == needle)
      {
        return (haystack + i);
      }
    }
  }
  
  return (NULL);
}

char* strrchr(char* haystack, char needle)
{
  char* ret = haystack;
  if (haystack != NULL)
  {
    for (size_t i = 0; haystack[i] != '\0'; i++)
    {
      if (haystack[i] == needle)
      {
        ret = haystack + i;
      }
    }
  }
  return (ret);
}

size_t strlen(const char* str)
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

