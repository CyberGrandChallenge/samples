#include <libcgc.h>
#include "mylibc.h"

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
#ifdef PATCHED
    if (ret != 0)
        _terminate(0);
    if (numRead == 0)
        _terminate(0);
#endif
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
