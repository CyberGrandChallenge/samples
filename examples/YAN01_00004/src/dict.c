#include <libcgc.h>
#include "printf.h"
#include "string.h"
#include "mylibc.h"

#include "definitions.h"

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

Defn* binary_search(const char* word, Defn* wordlist, size_t len)
{
  if ( (word == NULL) || (wordlist == NULL) )
  {
    return (NULL);
  }

  size_t start = 0;
  size_t end = len;
  size_t mid;

  int ret = 0;

  while (end > start)
  {
    mid = start + ((end - start) / 2);
    ret = strcmp(word, wordlist[mid].word);
    if (ret == 0)
    {
      return (&wordlist[mid]);
    }
    else if (ret < 0)
    {
      end = mid; 
    }
    else
    {
      start = mid + 1;
    }
  };
  return (NULL);
}


#define MAX_WORD_LEN (64)
#define MAX_USER_DICT_LEN (PAGE_SIZE / MAX_WORD_LEN)

Defn gUserDict[MAX_USER_DICT_LEN];
char gWordData[PAGE_SIZE];

#define BITMAP_LEN ((MAX_USER_DICT_LEN / 32) + 1)

uint32_t gWordDataBitmap[BITMAP_LEN];

char* getFreeWord()
{
  char* ret = NULL;

  for (size_t i = 0; i < BITMAP_LEN; i++)
  {
    if (gWordDataBitmap[i] != 0xFFFFFFFF) //not full
    {
      for (int j = 31; j >= 0; j--)
      {
        if (((0x1 << j) & gWordDataBitmap[i]) == 0) //if this position is free
        {
          ret = gWordData + ( ((i * 32) + (31 - j)) * MAX_WORD_LEN);
          gWordDataBitmap[i] |= (0x1 << j);
          break;
        }
      }
      break;
    }
  } 
  if ( ret > ((gWordData + PAGE_SIZE) - MAX_WORD_LEN) )
  {
    ret = NULL;
  }
  return (ret);
}

void releaseWord(const char* p)
{
  if ( ((p - gWordData) % MAX_WORD_LEN) != 0 )
  {
    return;
  }

  size_t offset = (p - gWordData) / MAX_WORD_LEN;
  size_t pos = offset / 32;
  size_t bit = offset % 32;
  gWordDataBitmap[pos] &= ~(0x1 << (31 - bit));
}


size_t gUserWordCount = 0;

char gBuf[PAGE_SIZE];

typedef enum { NO_ERROR, DICT_FULL, WORD_NOT_FOUND, WORD_EXISTS, MISC_ERROR } Errno;
char* errnoToMsg[] = { "SUCCESS", "DICTIONARY FULL", "WORD NOT FOUND", "WORD EXISTS", "MISC ERROR" };

Errno addWord(const char* newWord, const char* newDef)
{
  int ret = 0;
  
  if (gUserWordCount >= MAX_USER_DICT_LEN)
  {
    return (DICT_FULL);
  }

  size_t i = 0; 
  for (i = 0; i < MAX_USER_DICT_LEN; i++)
  {
    if (gUserDict[i].defn == NULL)
    {
      gUserDict[i].word = getFreeWord();
      if (gUserDict[i].word == NULL)
      {
        return (DICT_FULL);
      }
      strncpy(gUserDict[i].word, MAX_WORD_LEN, newWord);
      ret = allocate(PAGE_SIZE, 0, (void**)(&(gUserDict[i].defn)));
      if (ret != 0)
      {
        _terminate(-1);
      }
      strncpy(gUserDict[i].defn, PAGE_SIZE, newDef);
      gUserWordCount++;
      return (NO_ERROR);
    }

    ret = strcmp(newWord, gUserDict[i].word);
    if (ret == 0)
    {
      return (WORD_EXISTS);
    }
    else if (ret < 0) //if word is greater than the current word then move everything down
    {
      for (size_t j = gUserWordCount; j > i; j--)
      {
        gUserDict[j].word = gUserDict[j-1].word;
        gUserDict[j].defn = gUserDict[j-1].defn;
      }
      gUserDict[i].word = getFreeWord();
      if (gUserDict[i].word == NULL)
      {
        return (DICT_FULL);
      }
      strncpy(gUserDict[i].word, MAX_WORD_LEN, newWord);
      ret = allocate(PAGE_SIZE, 0, (void**)(&(gUserDict[i].defn)));
      if (ret != 0)
      {
        _terminate(-1);
      }
      strncpy(gUserDict[i].defn, PAGE_SIZE, newDef);
      gUserWordCount++;
      return (NO_ERROR);
    }
  }

  //SHOULD NOT BE HERE
  return (DICT_FULL);
}

#define USER_DICT_ADDR_TO_INDEX(_addr) ( (_addr - gUserDict) )

Errno removeWord(const char* word)
{
  Defn* pDef = binary_search(word, gUserDict, gUserWordCount);
  if (pDef == NULL)
  {
    return (WORD_NOT_FOUND);
  }

  //just free the definition memory
  deallocate(pDef->defn, PAGE_SIZE);
 
#ifdef PATCHED
  //free up the word buffer
  releaseWord(pDef->word);
  for (size_t i = USER_DICT_ADDR_TO_INDEX(pDef); i < (gUserWordCount - 1); i++)
  {
    gUserDict[i].word = gUserDict[i+1].word;
    gUserDict[i].defn = gUserDict[i+1].defn;
  }
  gUserDict[gUserWordCount - 1].word = NULL;
  gUserDict[gUserWordCount - 1].defn = NULL;
  gUserWordCount--; 
#endif 
 
  return (NO_ERROR);
}

void init()
{
  for (size_t i = 0; i < MAX_USER_DICT_LEN; i++)
  {
    gUserDict[i].word = gWordData + (i * MAX_WORD_LEN);
    gUserDict[i].defn = NULL;
  }

  for (size_t i = 0; i < PAGE_SIZE; i++)
  {
    gWordData[i] = 0;
  }
  
  for (size_t i = 0; i < BITMAP_LEN; i++)
  {
    gWordDataBitmap[i] = 0;
  }
}


char gBuf[PAGE_SIZE];

int main(void)
{
  char wordBuf[MAX_WORD_LEN];
  Errno ret = NO_ERROR;
  Defn* def = NULL;
  char* temp = NULL;
  size_t i = 0;
  size_t j = 0;

  printf_setFlushOnNewline(1);
 
  while (1)
  {
    gBuf[0] = '\0';
#ifdef PATCHED
    readLine(STDIN, gBuf, PAGE_SIZE, 0);
#else
    readLine(STDIN, gBuf, PAGE_SIZE, 1);
#endif
    if (gBuf[0] == 'B')
    {
      printf("BYE\n");
      _terminate(0);
    }

    //the other commands all need a word to lets look for it
    if (gBuf[1] != ' ')
    {
      printf("BAD CMD\n");
      continue;
    }
 
    wordBuf[0] = '\0';

    for (i = 0; i < MAX_WORD_LEN; i++)
    {
      wordBuf[i] = gBuf[i + 2]; 
      if ( wordBuf[i] == ' ' )
      {
        wordBuf[i] = '\0';
        break; //reached the end of the word
      }
      else if (wordBuf[i] == '\0')
      {
        break; //end of line
      }
    }
   
    if (i == MAX_WORD_LEN)
    {
      wordBuf[i-1] = '\0';
    }

    if (gBuf[0] == 'L')
    {
      def = binary_search(wordBuf, gUserDict, gUserWordCount);
      if (def == NULL)
      {
        def = binary_search(wordBuf, dictionary, dictionary_len);
      }

      if (def != NULL)
      {
        printf("$s : $s\n", def->word, def->defn);
      }
      else
      {
        printf("$s\n", errnoToMsg[WORD_NOT_FOUND]);
      }
    }
    else if (gBuf[0] == 'R')
    {
      ret = removeWord(wordBuf);
      printf("$s\n", errnoToMsg[ret]);
    }
    else if (gBuf[0] == 'A')
    {
      for ( ; (gBuf[i+2] != ' ') && (gBuf[i+2] != '\0') && ((i+2) < PAGE_SIZE); i++)
      {
        //empty
      }
          
      if (gBuf[i+2] != ' ')
      {
        printf("BAD CMD\n");
      }
      else
      {
        ret = addWord(wordBuf, gBuf + i + 2 + 1); //i+2 to get there, 1 for the space
        printf("$s\n", errnoToMsg[ret]);
      }
    } 
  }
}
