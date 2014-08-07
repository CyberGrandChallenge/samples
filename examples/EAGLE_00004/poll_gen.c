#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char **argv) {
   char *readbuf;
   int k;
   int numPollers = 1;
   FILE *outfile;
   if (argc == 2) {
      int np = atoi(argv[1]);
      if (np > 0 && np <= 1000) {
         numPollers = np;
      }
   }
   srand(time(NULL));
   readbuf = (char*)malloc(0x10000);
   for (k = 0; k < numPollers; k++) {
      char pname[256];
      int ridx = 0;
      int vals[128];
      int mem[128];
      int i;
      int npush = 0;
      int addr = 0;
      int vidx = 0;
      int stack[128];
      int sp = -1;
      int nvals = (rand() % 40) + 10;
      for (i = 0; i < nvals; i++) {
         vals[i] = rand() + rand();
      }
      if (numPollers == 1) {
         outfile = stdout;
      }
      else {
         struct stat sb;
         snprintf(pname, sizeof(pname), "poller-%d.xml", k + 1);
         if (stat(pname, &sb) == 0) {
            continue;  //don't overwrite existing files
         }
         outfile = fopen(pname, "w");
         if (outfile == NULL) {
            fprintf(stderr, "Unable to open %s for writing\n", pname);
            continue;  //don't overwrite existing files
         }
      }
      fprintf(outfile, "<?xml version=\"1.0\" standalone=\"no\" ?>\n");
      fprintf(outfile, "<!DOCTYPE pov SYSTEM \"/usr/share/cgc-replay/replay.dtd\">\n");
      fprintf(outfile, "<pov>\n<cbid>EAGLE_00004</cbid>\n<replay>\n");
      fprintf(outfile, "   <write>\n      <data>aPenFullOfSheep:)\\n</data>\n");
      for (i = 0; i < (nvals * 2); i++) {
         int op;
         if (npush == nvals) {   //must pop
            int reg = rand() & 3;
            mem[addr] = stack[sp];
            sp--;
            if (rand() & 1) {
               fprintf(outfile, "      <data>pop r%d\\nout r%d\\nmov m%d,r%d\\n</data>\n", reg, reg, addr, reg);
            }
            else {
               fprintf(outfile, "      <data>pop m%d\\nout m%d\\n</data>\n", addr, addr);
            }
            ridx += sprintf(readbuf + ridx, "   <read echo=\"ascii\"><delim>\\n</delim><match><data>OUT: 0x%.8x\\n</data></match></read>\n", mem[addr]);
            addr++;
         }
         else if (sp == -1) {  //all we can do is push
            if (rand() & 1) {
               int reg = rand() & 3;
               sp++;
               stack[sp] = vals[vidx++];
               fprintf(outfile, "      <data>mov r%d,%d\\npush r%d\\n</data>\n", reg, stack[sp], reg);
            }
            else {
               sp++;
               stack[sp] = vals[vidx++];
               fprintf(outfile, "      <data>push %d\\n</data>\n", stack[sp]);
            }
            npush++;
         }
         else {
            if (rand() & 1) { //push
               if (rand() & 1) {
                  int reg = rand() & 3;
                  sp++;
                  stack[sp] = vals[vidx++];
                  fprintf(outfile, "      <data>mov r%d,%d\\npush r%d\\n</data>\n", reg, stack[sp], reg);
               }
               else {
                  sp++;
                  stack[sp] = vals[vidx++];
                  fprintf(outfile, "      <data>push %d\\n</data>\n", stack[sp]);
               }
               npush++;
            }
            else {  //pop
               int reg = rand() & 3;
               mem[addr] = stack[sp];
               sp--;
               if (rand() & 1) {
                  fprintf(outfile, "      <data>pop r%d\\nout r%d\\nmov m%d,r%d\\n</data>\n", reg, reg, addr, reg);
               }
               else {
                  fprintf(outfile, "      <data>pop m%d\\nout m%d\\n</data>\n", addr, addr);
               }
               ridx += sprintf(readbuf + ridx, "   <read echo=\"ascii\"><delim>\\n</delim><match><data>OUT: 0x%.8x\\n</data></match></read>\n", mem[addr]);
               addr++;
            }
         }
      }
      
      if (addr != nvals) {
         fprintf(stderr, "something went wrong\n");
         fclose(outfile);
         if (numPollers == 1) {
            break;
         }
         unlink(pname);
         k--;
      }
      else {
         int acc = rand() & 3;
         int res = rand() + rand();
         fprintf(outfile, "      <data>mov r%d,%d\\n</data>\n", acc, res);
         for (addr = 0; addr < nvals; addr++) {
            int reg;
            do {
               reg = rand() & 3;
            } while (reg == acc);
            switch (rand() & 7) {
               case 0:
                  res += mem[addr];
                  if (rand() & 1) {
                     fprintf(outfile, "      <data>mov r%d,m%d\\nadd r%d,r%d\\n</data>\n", reg, addr, acc, reg);
                  }
                  else {
                     fprintf(outfile, "      <data>add r%d,m%d\\n</data>\n", acc, addr);
                  }               
                  break;
               case 1:
                  res -= mem[addr];
                  if (rand() & 1) {
                     fprintf(outfile, "      <data>mov r%d,m%d\\nsub r%d,r%d\\n</data>\n", reg, addr, acc, reg);
                  }
                  else {
                     fprintf(outfile, "      <data>sub r%d,m%d\\n</data>\n", acc, addr);
                  }               
                  break;
               case 2:
                  res *= mem[addr];
                  if (rand() & 1) {
                     fprintf(outfile, "      <data>mov r%d,m%d\\nmul r%d,r%d\\n</data>\n", reg, addr, acc, reg);
                  }
                  else {
                     fprintf(outfile, "      <data>mul r%d,m%d\\n</data>\n", acc, addr);
                  }               
                  break;
               case 3:
                  res /= mem[addr];
                  if (rand() & 1) {
                     fprintf(outfile, "      <data>mov r%d,m%d\\ndiv r%d,r%d\\n</data>\n", reg, addr, acc, reg);
                  }
                  else {
                     fprintf(outfile, "      <data>div r%d,m%d\\n</data>\n", acc, addr);
                  }               
                  break;
               case 4:
                  res %= mem[addr];
                  if (rand() & 1) {
                     fprintf(outfile, "      <data>mov r%d,m%d\\nmod r%d,r%d\\n</data>\n", reg, addr, acc, reg);
                  }
                  else {
                     fprintf(outfile, "      <data>mod r%d,m%d\\n</data>\n", acc, addr);
                  }               
                  break;
               case 5: {
                  int shift = rand() & 7;
                  res <<= shift;
                  fprintf(outfile, "      <data>shl r%d,%d\\n</data>\n", acc, shift);
                  addr--;
                  break;
               }
               case 6: {
                  int shift = rand() & 7;
                  res >>= shift;
                  fprintf(outfile, "      <data>shr r%d,%d\\n</data>\n", acc, shift);
                  addr--;
                  break;
               }
               case 7:
                  res = htonl(res);
                  fprintf(outfile, "      <data>swp r%d\\n</data>\n", acc);
                  addr--;
                  break;
            }
            fprintf(outfile, "      <data>out r%d\\n</data>\n", acc);   
            ridx += sprintf(readbuf + ridx, "   <read echo=\"ascii\"><delim>\\n</delim><match><data>OUT: 0x%.8x\\n</data></match></read>\n", res);
            if (res != 0x80000000) {
               fprintf(outfile, "      <data>out %d\\n</data>\n", res);   
               ridx += sprintf(readbuf + ridx, "   <read echo=\"ascii\"><delim>\\n</delim><match><data>OUT: 0x%.8x\\n</data></match></read>\n", res);
            }
         }
   //      fprintf(outfile, "      <data>out r%d\\nout %d\\n</data>\n", acc, res);   
   //      ridx += sprintf(readbuf + ridx, "   <read echo=\"ascii\"><delim>\\n</delim><match><data>OUT: 0x%.8x\\n</data></match></read>\n", res);
   //      ridx += sprintf(readbuf + ridx, "   <read echo=\"ascii\"><delim>\\n</delim><match><data>OUT: 0x%.8x\\n</data></match></read>\n", res);
      }
      fprintf(outfile, "      <data>END\\n</data>\n");
      fprintf(outfile, "   </write>\n");
      fprintf(outfile, "%s", readbuf);
      fprintf(outfile, "   <read echo=\"ascii\"><delim>\\n</delim><match><data>COMPLETE\\n</data></match></read>\n");
      fprintf(outfile, "</replay>\n</pov>\n");
      fclose(outfile);
   }
}
