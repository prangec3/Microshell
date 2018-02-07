#include "arg_parse.h"
int getSize(char *line);
void purgeQuotes(char* line);
//int expand(char* orig, char* newbuf, int newSize);
//int intlen(int x);

/*char newBuf[128];*/
/*Defined here because Atom gets confused by curly braces in quotes.*/
/*Apologies for any readability issues*/
/*static const char LCB ='{';
static const char RCB ='}';*/

/*
* char** arg_parse(char *line)
*Accepts a String of words separated by whitespace
*Returns an array of pointers to the first character in each word.
*/
char** arg_parse(char *line, int *argcp)
{
  /*if(!expand(line,newBuf,128))
    return NULL;*/
  //line = newBuf;
  //printf("%s\n",line);
  int size= getSize(line);
  *argcp = size;

  if(size<=0)
    return NULL;

  char** words = malloc((size+1) * sizeof(char*));
  if(words == NULL)
  {
    fprintf(stderr, "malloc");
    return NULL;
  }
  words[size]=NULL;
  /*End initialization*/
  char* clone = line;
  int state = 0;
  int pos = 0;
  int inquote = 0;
  while(*line != '\0')
  {
    if(*line == '\"')
    {
      inquote = !inquote;
    }
    if(state == 0 && !isspace(*line))
    {
      state = 1;
      words[pos] = line;
      pos++;
    }
    /*An argument is over if a whitespace is seen and
      an even number of quotes are present in said arg*/
    if(isspace(*line)&&!inquote)
    {
      state=0;
      *line = '\0';
    }
    line++;
    /*Terminate the line if we find a comment #*/
    if(*line == '#' && !inquote)
    {
      *line = '\0';
    }
  }
  /*Delete double quotes from all arguments*/
  for(int i = 0; i<size; i++)
  {
    purgeQuotes(words[i]);
  }
  return words;
}

/*
* void purgeQuotes
*removes all double quotation marks (") from
*a given string.
*/
void purgeQuotes(char* line)
{
  char *src = line;
  char *dst = line;
  while(*src!= '\0')
  {
    if(*src != '\"')
    {
      *dst = *src;
      dst++;
    }
    src++;
  }
  *dst = '\0';
}


/*
* int getSize(char *lineClone)
* Used by arg_parse. Returns the number of words in a string
* separated by whitespace.
* char *lineClone represents the start of the string.
*/
int getSize(char *line)
{
  int count = 0;
  /*State 0: Not in an argument*/
  /*State 1: In an argument*/
  int state = 0;
  /*inquote: whether the number of quotes in this argument
    is even or odd*/
  int inquote = 0;
  while(*line != '\0')
  {
    if(*line == '#' && !inquote)
    {
      return count;
    }
    if(*line == '\"')
      inquote = !inquote;
    if(state==0 && !isspace(*line))
    {
      state = 1;
      count++;
    }
    //Argument ends if we reach a whitespace with an even # of quotes.
    if(state==1 && isspace(*line) && !inquote)
    {
      state = 0;
    }
    line++;
  }
  if(inquote)
  {
    fprintf(stderr, "Odd # of quotes\n");
    return -1;
  }
  return count;
}
