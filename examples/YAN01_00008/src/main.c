#include <libcgc.h>
#include "mylibc.h"
#include "stdarg.h"

int check_diophantine(uint32_t power, uint32_t answer, uint32_t* terms, uint32_t numTerms)
{
  uint32_t sum = 0;
  uint32_t product = 0;
  
  if (terms == NULL)
  {
    return (-1);
  }

  for (int i = 0; i < numTerms; i++)
  {
    product = 1;
    for (int p = 0; p < power; p++)
    {
      product *= terms[i];
    }
    sum += product;
  }

  product = 1;
  for (int p = 0; p < power; p++)
  {
    product *= answer;
  }

  return (product == sum);
}

void tenthSecondSleep()
{
  struct timeval timeToWait;
  fd_set fdsToWait;

  FD_ZERO(&fdsToWait);
  FD_SET(3, &fdsToWait);
  
  timeToWait.tv_sec = 0;
  timeToWait.tv_usec = 100000; 

  fdwait(1, &fdsToWait, NULL, &timeToWait, NULL);
}

#define MAX_STR_LEN 64
#define MAX_PLAYERS 10

typedef struct _TopScoreEntry
{
  char name[MAX_STR_LEN];
  char notes[MAX_STR_LEN];
  uint32_t score;
} TopScoreEntry;

TopScoreEntry* gScoreboard = NULL;

void fermatEasterEgg()
{
  size_t rx_bytes;
  int ret = 0;
#ifdef PATCHED
  char buf[64];
#else
  char buf[32];
#endif
  my_printf("Fermat says you can't find integers a, b, c such that a^n + b^n = c^n for n > 2\n");
  my_printf("You were able to find three ints.\n");
  my_printf("What is your name?\n");
  rx_bytes = readLine(STDIN, buf, 32, 1);
  
  strncpy(gScoreboard[0].name, MAX_STR_LEN, buf);

  my_printf("Something we should know about you?\n");
  rx_bytes = readLine(STDIN, buf, MAX_STR_LEN, 1);

  strncpy(gScoreboard[0].notes, MAX_STR_LEN, buf);

  gScoreboard[0].score = 0xFFFFFFFF;
}

void initScoreboard()
{
  int ret = 0;
 
  ret = allocate(sizeof(TopScoreEntry) * MAX_PLAYERS, 0, (void**)(&gScoreboard));
  if (ret != 0)
  {
    _terminate(-1);
  }

  for (int i = 0; i < MAX_PLAYERS; i++)
  {
    strncpy(gScoreboard[i].name, 64, "Player ");
    snprintdecimal(gScoreboard[i].name + 7, MAX_STR_LEN - 7, i, 0);
    snprintdecimal(gScoreboard[i].notes, MAX_STR_LEN, MAX_PLAYERS - i, 0);
    gScoreboard[i].score = MAX_PLAYERS - i; 
  }
}

void printScoreboard()
{
  char buf[MAX_STR_LEN];
  size_t ret = 0;
  
  for (int i = 0; i < MAX_PLAYERS; i++)
  {
    my_printf(gScoreboard[i].name);
    buf[0] = '\t';
    ret = snprintdecimal(buf + 1, MAX_STR_LEN - 3, gScoreboard[i].score, 10);
    buf[ret + 1] = '\t';
    buf[ret + 2] = '\0';
    my_printf(buf);
    my_printf(gScoreboard[i].notes);
    my_printf("\n");
  }
}

void updateNotes(TopScoreEntry* player, uint32_t power, uint32_t numTerms)
{
  size_t ret = 0;
  size_t cur = 0;

  if (player == NULL)
  {
    return;
  }

  ret = snprintdecimal(player->notes, MAX_STR_LEN, power, 0);
  if (ret >= MAX_STR_LEN)
  {
    return;
  }
  player->notes[ret] = '.';
  cur = ret + 1;
  ret = snprintdecimal(player->notes + cur, MAX_STR_LEN - cur, 1, 0);
  if ( (cur + ret) >= MAX_STR_LEN)
  {
    return;
  }
  player->notes[cur + ret] = '.';
  cur += ret + 1;
  ret = snprintdecimal(player->notes + cur, MAX_STR_LEN - cur, numTerms, 0);
}

void updateScoreboard(TopScoreEntry* player)
{
  if (player == NULL)
  {
    return;
  }

  for (int i = 0; i < MAX_PLAYERS; i++)
  {
    if ( (player->score > gScoreboard[i].score) )
    {
      for (int j = (MAX_PLAYERS - 1); j > i; j--)
      {
        strncpy(gScoreboard[j].name, MAX_STR_LEN, gScoreboard[j-1].name);
        strncpy(gScoreboard[j].notes, MAX_STR_LEN, gScoreboard[j-1].notes);
        gScoreboard[j].score = gScoreboard[j-1].score;
      }
 
      strncpy(gScoreboard[i].name, MAX_STR_LEN, player->name);
      strncpy(gScoreboard[i].notes, MAX_STR_LEN, player->notes);
      gScoreboard[i].score = player->score;
      break;
    }
  }
}

#define MAX_TERMS 15

#define RECEIVE(_buf, _size, _rx_bytes)	readAll(STDIN, _buf, _size)

int main(void)
{
  uint32_t power = 0;
  uint32_t sol = 0;
  uint32_t numTerms = 0;
  uint32_t terms[MAX_TERMS];
  size_t rx_bytes = 0;

  char buf[MAX_STR_LEN];

  TopScoreEntry player;
  
  initScoreboard();

  my_printf("Welcome to the Diophantine Equation Finding Game\n");
  my_printf("Please enter your name\n");

  readLine(STDIN, player.name, MAX_STR_LEN, 1);

  do
  {
    my_printf("Please choose a difficulty level (Power) from 3-10\n");
    /*
    readLine(STDIN, buf, MAX_STR_LEN, 1);
    power = strToUint32(buf);
    */
    
    RECEIVE(&power, sizeof(power), &rx_bytes);

  } while ( (power > 10) || (power < 3) );

  do
  {
    my_printf("What is the solution?\n");
    RECEIVE(&sol, sizeof(power), &rx_bytes);
    /*
    readLine(STDIN, buf, MAX_STR_LEN, 1);
    sol = strToUint32(buf);
    */
    my_printf("How many terms (integers) are in your solution?\n");
    RECEIVE(&numTerms, sizeof(power), &rx_bytes);
    /*
    readLine(STDIN, buf, MAX_STR_LEN, 1);
    numTerms = strToUint32(buf);
    */ 
    if ( (numTerms < 2) || (numTerms > MAX_TERMS) )
    {
      break;
    }

    my_printf("Enter your numbers\n");
    for (int i = 0; i < numTerms; i++)
    {
      RECEIVE(&terms[i], sizeof(terms[i]), &rx_bytes);
      /*
      readLine(STDIN, buf, MAX_STR_LEN, 1);
      terms[i] = strToUint32(buf);
      */
    }

    for (int i = 0; i < numTerms - 1; i++)
    {
      if ( terms[i] >= terms[i+1] )
      {
        break;
      }
    }

    if ( terms[numTerms - 1] >= sol )
    {
      break;
    }

    if (!check_diophantine(power, sol, terms, numTerms))
    {
      break;
    }
  
    if (numTerms == 2)
    {
      fermatEasterEgg();
      my_printf("You win!\n");
      printScoreboard();
       _terminate(0);
    }
    
    player.score += power * numTerms;
    updateNotes(&player, power, numTerms);
  } while (1);
   
 
  my_printf("Thanks for playing. Here are the final scores\n");

  updateScoreboard(&player);

  printScoreboard();

  return (0);
}
