#include <libpov.h>
int main(void) {
   negotiate_type1(0xfefefefe, 0xfefefefe, 2);
   do {
      unsigned char *read_00000;
      unsigned int read_00000_len;
      unsigned int read_00000_ptr = 0;
      //**** delimited read
      static unsigned char read_00000_delim[] = 
         "\x0a";
      read_00000 = NULL;
      read_00000_len = 0;
      int read_00000_res = delimited_read(0, &read_00000, &read_00000_len, read_00000_delim, 1);
      free(read_00000);
   } while (0);
   do {
      unsigned char *read_00001;
      unsigned int read_00001_len;
      unsigned int read_00001_ptr = 0;
      //**** delimited read
      static unsigned char read_00001_delim[] = 
         "\x0a";
      read_00001 = NULL;
      read_00001_len = 0;
      int read_00001_res = delimited_read(0, &read_00001, &read_00001_len, read_00001_delim, 1);
      free(read_00001);
   } while (0);
   do {
      //*** writing data
      static unsigned char write_00000_00000[] = 
         "\x41\x42\x43";
      static unsigned int write_00000_00000_len = 3;
      static unsigned char write_00000_00003[] = 
         "\x0a";
      static unsigned int write_00000_00003_len = 1;
      unsigned char *write_00000 = NULL;
      unsigned int write_00000_len = 0;
      write_00000 = append_buf(write_00000, &write_00000_len, write_00000_00000, write_00000_00000_len);
      write_00000 = append_var("TYPE1_IP", write_00000, &write_00000_len);
      write_00000 = append_var("TYPE1_REG", write_00000, &write_00000_len);
      write_00000 = append_buf(write_00000, &write_00000_len, write_00000_00003, write_00000_00003_len);
      if (write_00000_len > 0) {
         transmit_all(1, write_00000, write_00000_len);
      }
      free(write_00000);
   } while (0);
}
