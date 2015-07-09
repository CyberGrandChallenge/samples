#include <libcgc.h>

typedef unsigned long long uint64_t;
typedef unsigned long uint32_t;

#define PAGE_SIZE 4096
#define MAX_BUF_SIZE PAGE_SIZE
#define HEX_BUF_SIZE 11
#define MSG_BUF_SIZE 64
#define RET_MSG "Return Code: "
#define BYTES_MSG "Num Bytes: "
#define RAND_MSG "RAND BYTES: "
//Subtract 1 because sizeof contains the NULL character
#define RET_MSG_LEN (sizeof(RET_MSG) - 1)
#define BYTES_MSG_LEN (sizeof(BYTES_MSG) - 1)
#define RAND_MSG_LEN (sizeof(RAND_MSG) - 1)
#define NUM_TEST_FUNCS 8

void write(int fd, const void* buf, size_t len) {
   const void *end = buf + len;
   size_t n;
   while (buf < end) {
      int r = transmit(fd, buf, end - buf, &n);
      if (r != 0 || n == 0) {
         //error we can't continue
         _terminate(1);
      }
      buf += n;
   }
}

void testNotSupported(void)
{
  //not supported so do nothing
}

#ifdef PATCHED
#define NOT_SUPPORTED_FUNC (&testNotSupported)
#else
#define NOT_SUPPORTED_FUNC 0
#endif

#define USEC_IN_SEC (1000000)
char hexChars[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

#define isHex(_c) ( ((_c >= 'a') && (_c <= 'f')) || ((_c >= 'A') && (_c <= 'F')) || ((_c >= '0') && (_c <= '9')) )

//BUF NEEDS TO BE >= 12 BYTES - we don't to add the '\0'
void hexToStr(char* buf, uint32_t num)
{
  int i = 0;
  int j = 0;
  int temp = 0;

  buf[0] = '0';
  buf[1] = 'x';
  
  for (i = 32 - 4, j = 2; i >= 0; i-=4, j++)
  {
    temp = (num >> i) & 0xF;
    buf[j] = hexChars[temp]; 
  }

  buf[j] = '\n';
}

void strToHex(char* buf, uint32_t* pNum)
{
  int i = 0;
  int num = 0; 
  int temp = 0;

  if ((buf == NULL) || (pNum == NULL))
  {
    return;
  }

  for ( i = 0; isHex(buf[i]); i++)
  {
    if ( (buf[i] >= 'A') && (buf[i] <= 'F') )
    {
      temp = 10 + buf[i] - 'A'; 
    }
    else if ( (buf[i] >= 'a') && (buf[i] <= 'f') )
    {
      temp = 10 + buf[i] - 'a';
    }
    else if ( (buf[i] >= '0') && (buf[i] <= '9') )
    {
      temp = buf[i] - '0';
    }
    num = (num << 4) | (temp & 0xF);
  }
  *pNum = num;
}

char gBuf[MAX_BUF_SIZE];
char gRetBuf[MSG_BUF_SIZE] = RET_MSG;
char gBytesBuf[MSG_BUF_SIZE] = BYTES_MSG;
char gRandBuf[MSG_BUF_SIZE] = RAND_MSG;

typedef void (*testFunc)(void);

struct 
{
  uint32_t count;
  uint32_t printGran;
  int bStopOnError;
  uint32_t option;
  uint32_t option2;
  testFunc func;
} gConfig;

void testTerminate(void)
{
  _terminate(gConfig.option);
}

typedef int (*txrxFunc)(int fd, void* buf, size_t count, size_t* txrx_bytes);
txrxFunc gTxRxTestFunc;
 
void testTxRx(int fd)
{
  uint32_t i = 0;
  int ret = 0; 
  size_t txrx_bytes;
  size_t totalBytes = 0;


  if (gConfig.option > MAX_BUF_SIZE)
  {
    return;
  }

  for (i = 0; i < gConfig.count; i++)
  {
    totalBytes = 0;
    do
    {
      ret = gTxRxTestFunc(fd, gBuf + totalBytes, gConfig.option - totalBytes, &txrx_bytes);
      if (gConfig.bStopOnError)
      {
        if (ret != 0)
        {
          hexToStr(gRetBuf + RET_MSG_LEN, ret);
          write(STDERR, gRetBuf, RET_MSG_LEN + HEX_BUF_SIZE);
          hexToStr(gBuf, i);
          write(STDERR, gBuf, HEX_BUF_SIZE);
          return;
        }
        if (txrx_bytes == 0)
        {
          hexToStr(gBytesBuf + BYTES_MSG_LEN, ret);
          write(STDERR, gBytesBuf, BYTES_MSG_LEN + HEX_BUF_SIZE);
          hexToStr(gBuf, i);
          write(STDERR, gBuf, HEX_BUF_SIZE);
          return;
        }
      }
      totalBytes += txrx_bytes;
    } while (totalBytes < gConfig.option);

    if ( (gConfig.printGran != 0) && (i % gConfig.printGran) == 0 )
    {
      hexToStr(gBuf, i);
      write(STDOUT, gBuf, HEX_BUF_SIZE);
    }
  }
}

void testTransmit(void)
{
  gTxRxTestFunc = (txrxFunc)&transmit;
  testTxRx(STDOUT);
}

void testReceive(void)
{
  gTxRxTestFunc = &receive;
  testTxRx(STDIN);
}

void testFDWait(void)
{
  int i = 0;
  int ret = 0;
  fd_set fdsToWait;
  struct timeval timeToWait;
  int readyfds = 0;

  for (i = 0; i < gConfig.count; i++)
  {
    timeToWait.tv_sec = gConfig.option / USEC_IN_SEC;
    timeToWait.tv_usec = gConfig.option % USEC_IN_SEC;
  
    FD_ZERO(&fdsToWait);
    FD_SET(gConfig.option2 & (FD_SETSIZE - 1), &fdsToWait);
    ret = fdwait(1, &fdsToWait, NULL, &timeToWait, &readyfds);
    if (gConfig.bStopOnError)
    {
      if (ret != 0)
      {
        hexToStr(gRetBuf + RET_MSG_LEN, ret);
        write(STDERR, gRetBuf, RET_MSG_LEN + HEX_BUF_SIZE);
        hexToStr(gBuf, i);
        write(STDERR, gBuf, HEX_BUF_SIZE);
        return;
      }
    }

    if ( (gConfig.printGran != 0) && (i % gConfig.printGran) == 0 )
    {
      hexToStr(gBuf, i);
      write(STDOUT, gBuf, HEX_BUF_SIZE);
    }
  }
}

void testAllocate(void)
{
  uint32_t i = 0; 
  uint32_t j = 0; 
  int ret = 0;
  void* buf;
  for (i = 0; i < gConfig.count; i++)
  {
    ret = allocate(gConfig.option, 0, &buf);
    if (ret == 0)
    {
      if (gConfig.option2)
      {
        for (j = 0; j < (gConfig.option / PAGE_SIZE); j++)
        {
          *((int*)(buf + (j*PAGE_SIZE))) = 1;
        }
      }
      ret = deallocate(buf, gConfig.option);
    }

    if ( (ret != 0) && (gConfig.bStopOnError) )
    { 
      hexToStr(gRetBuf + RET_MSG_LEN, ret); 
      write(STDERR, gRetBuf, RET_MSG_LEN + HEX_BUF_SIZE); 
      hexToStr(gBuf, i);
      write(STDERR, gBuf, HEX_BUF_SIZE);
      return;
    }
    
    if ( (gConfig.printGran != 0) && (i % gConfig.printGran) == 0 )
    {
      hexToStr(gBuf, i);
      write(STDOUT, gBuf, HEX_BUF_SIZE);
    }
  }
}

/** NOT NEEDED - paired with testAllocate
void testDeallocate(void)
{
}
**/

void testRandom(void)
{
  uint32_t i = 0;
  int ret = 0;
  size_t rnd_bytes;
  uint32_t randCount = (gConfig.option > MAX_BUF_SIZE) ? MAX_BUF_SIZE : gConfig.option;

  for (i = 0; i < gConfig.count; i++)
  {
    ret = random(gBuf, randCount, &rnd_bytes);

    if (gConfig.bStopOnError)
    {
      if (ret != 0)
      {
        hexToStr(gRetBuf + RET_MSG_LEN, ret);
        write(STDERR, gRetBuf, RET_MSG_LEN + HEX_BUF_SIZE);
        hexToStr(gBuf, i);
        write(STDERR, gBuf, HEX_BUF_SIZE);
        return;
      }
      if (rnd_bytes != randCount)
      {
        hexToStr(gBytesBuf + BYTES_MSG_LEN, ret);
        write(STDERR, gBytesBuf, BYTES_MSG_LEN + HEX_BUF_SIZE);
        hexToStr(gBuf, i);
        write(STDERR, gBuf, HEX_BUF_SIZE);
        return;
      }
    }
   
    if ( (gConfig.option2 != 0) && ((i % gConfig.option2) == 0))
    {
      hexToStr(gRandBuf + RAND_MSG_LEN, *((uint32_t*)gBuf));
      write(STDERR, gRandBuf, RAND_MSG_LEN + HEX_BUF_SIZE);
    }

    //now that we have the random values 
    if ( (gConfig.printGran != 0) && (i % gConfig.printGran) == 0 )
    {
      hexToStr(gBuf, i);
      write(STDOUT, gBuf, HEX_BUF_SIZE);
    }
  }
}

typedef int (*userOpFunc) (int i, int j);
typedef double (*userFPOpFunc) (double i, double j);

void testUserOp(userOpFunc fun)
{
  uint32_t i = 0;
  int i1, i2;
  for (i = 0; i < gConfig.option; i++)
  {
    fun(i1, i2);
  }
}

void testUserFPOp(userFPOpFunc fun)
{
  uint32_t i = 0;
  double d1, d2;
  for (i = 0; i < gConfig.option; i++)
  {
    fun(d1, d2); 
  }
}

static inline int testADD(int i, int j)
{
  return (i+j);
}

static inline int testMUL(int i, int j)
{
  return (i*j);
}

static inline double testFPADD(double i, double j)
{
  return (i+j);
}

static inline double testFPMUL(double i, double j)
{
  return (j*j);
}

void testUser(void)
{
  uint32_t i = 0;
  for (i = 0; i < gConfig.count; i++)
  {
    testUserOp(&testADD);
    testUserOp(&testMUL);
    testUserFPOp(&testFPADD);
    testUserFPOp(&testFPMUL);

    if ( (gConfig.printGran != 0) && (i % gConfig.printGran) == 0 )
    {
      hexToStr(gBuf, i);
      write(STDOUT, gBuf, HEX_BUF_SIZE);
    }
  }
}

testFunc gFuncs[NUM_TEST_FUNCS] = { &testUser, &testTerminate, &testTransmit, &testReceive, &testFDWait, &testAllocate, NOT_SUPPORTED_FUNC, &testRandom };

void parseConfig(void)
{
  int start = 0;
  int end = 0;
  uint32_t temp = 0;

  for (end = start ;isHex(gBuf[end]); end++);
  gBuf[end] = '\0';
  strToHex(&(gBuf[start]), &temp);
  temp = temp % NUM_TEST_FUNCS;
  gConfig.func = gFuncs[temp];

  start = end+1;
  temp = 0;
  for (end = start ;isHex(gBuf[end]); end++);
  gBuf[end] = '\0';
  strToHex(&(gBuf[start]), &temp);
  gConfig.count = temp;

  start = end+1;
  temp = 0;
  for (end = start ;isHex(gBuf[end]); end++);
  gBuf[end] = '\0';
  strToHex(&(gBuf[start]), &temp);
  gConfig.printGran = temp;

  start = end+1;
  temp = 0;
  for (end = start ;isHex(gBuf[end]); end++);
  gBuf[end] = '\0';
  strToHex(&(gBuf[start]), &temp);
  gConfig.option = temp;

  start = end+1;
  temp = 0;
  for (end = start ;isHex(gBuf[end]); end++);
  gBuf[end] = '\0';
  strToHex(&(gBuf[start]), &temp);
  gConfig.option2 = temp;

  start = end+1;
  temp = 0;
  for (end = start ;isHex(gBuf[end]); end++);
  gBuf[end] = '\0';
  strToHex(&(gBuf[start]), &temp);
  gConfig.bStopOnError = temp;

}

int main(void)
{
  size_t rx_bytes = 0;
  int ret = 0;
  size_t totalBytes = 0;

  for (int i = 0; i < MAX_BUF_SIZE; i++)
  {
    gBuf[i] = '0';
  }

  do
  {
    ret = receive(STDIN, gBuf + totalBytes, 128 - totalBytes, &rx_bytes);
    if ((ret != 0) || (rx_bytes == 0))
    {
      return (-1);
    }
    totalBytes += rx_bytes;
  } while (totalBytes < 128);

  gBuf[127] = '\0'; //force a c-string
  
  parseConfig();
 
  gConfig.func(); 

  write(STDOUT, "END\n", 4);
  return (0);
}

