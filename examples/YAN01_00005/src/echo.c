#include <libcgc.h>
#include "printf.h"

int main(void)
{
  char buf[128];
  size_t rx_bytes;
  int ret = 0;
  
  do
  { 
#ifdef PATCHED
    ret = receive(STDIN, buf, 127, &rx_bytes);
#else
    ret = receive(STDIN, buf, 256, &rx_bytes);
#endif
    
    if ( (ret != 0) || (rx_bytes == 0) )
    {
      return (-1);
    }

    buf[rx_bytes] = '\0';

#ifdef PATCHED
    printf("%s", buf);
#else
    printf(buf);
#endif
    fflush(STDOUT);

  } while (1);

  return (0);
}
