#include <libpov.h>
int main(void) {
   negotiate_type2();
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
         "\x41\x42\x44\x0a";
      static unsigned int write_00000_00000_len = 4;
      unsigned char *write_00000 = NULL;
      unsigned int write_00000_len = 0;
      write_00000 = append_buf(write_00000, &write_00000_len, write_00000_00000, write_00000_00000_len);
      if (write_00000_len > 0) {
         transmit_all(1, write_00000, write_00000_len);
      }
      free(write_00000);
   } while (0);
   do {
      unsigned char *read_00002;
      unsigned int read_00002_len;
      unsigned int read_00002_ptr = 0;
      //**** length read
      size_t read_00002_len_len;
      char *read_00002_len_var = (char*)getenv("TYPE2_LENGTH", &read_00002_len_len);
      read_00002_len = *(unsigned int*)read_00002_len_var;
      free(read_00002_len_var);
      read_00002 = (unsigned char*)malloc(read_00002_len);
      int read_00002_res = length_read(0, read_00002, read_00002_len);
      //**** read assign to var "TYPE2_VALUE" from slice
      assign_from_slice("TYPE2_VALUE", read_00002, read_00002_len - read_00002_ptr, 0, 0, 1);
      free(read_00002);
   } while (0);
   //*** submitting type 2 POV results
   submit_type2("TYPE2_VALUE");
}
