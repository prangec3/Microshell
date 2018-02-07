#include "builtin.h"
#include <unistd.h>

/*Begin Definitions*/
typedef int (*cmd) (char** command, int argc, int fd);

typedef struct
{
  char *name;
  cmd func;
} comm ;

#define functioncount 5
/*End definitions*/

/*Begin prototypes*/
int BI_exit(char** arguments, int argc, int fd);
int BI_aecho(char** arguments, int argc, int fd);
int BI_CD(char** arguments, int argc, int fd);
int BI_setenv(char** arguments, int argc, int fd);
int BI_unsetenv(char** arguments, int argc, int fd);
/*End prototypes*/

/*array of built in commands*/
static const comm commands[] = {
  /*{name, function pointer}*/
  {"exit",&BI_exit},
  {"aecho",&BI_aecho},
  {"cd",&BI_CD},
  {"envset",&BI_setenv},
  {"evunset",&BI_unsetenv},
};

/*
* int BI_exit
* Calls exit using an error code supplied
* through the second given argument, or 0
* if none is given
* Probably shouldn't return.
*/
int BI_exit(char** arguments, int argc, int fd)
{
  int excode = 0;
  if(argc>=2)
  {
    excode = atoi(arguments[1]);
  }
  exit(excode);
  return 1;
}

/*
* int BI_aecho
* Prints all arguments besides the command name and -n
* if present. If -n is present as the second argument
* the string will not be terminated by a newline
*
*/
int BI_aecho(char** arguments, int argc, int fd)
{
  int nflag = 0;
  /*Sets nflag if n is present*/
  if(argc>=2)
  {
    if(strcmp(arguments[1],"-n")==0)
    {
      nflag = 1;
    }
  }
  /*Prints args after nflag if present*/
  if(argc>=2+nflag)
  {
    int i = 0;
    for(i = 1+nflag; i<argc-1; i++)
    {
      //fprintf(stdout, "%s ", arguments[i]);
      dprintf(fd, "%s ", arguments[i]);
    }
    /*Print final arg without a space after it*/
    dprintf(fd, "%s",arguments[i]);
  }
  /*Print a newline if nflag isn't indicated.*/
  if(nflag==0)
  {
    dprintf(fd,"%s","\n");
  }

  return 1;
}

int BI_CD(char** arguments, int argc, int fd)
{
  if(argc!=2)
  {
    fprintf(stderr,"Invalid number of arguments\n");
    return 0;
  }
  else
  {
    chdir(arguments[1]);
    return 1;
  }
}

int BI_setenv(char** arguments, int argc, int fd)
{
  if(argc!=3)
  {
    fprintf(stderr,"%d",argc);
    fprintf(stderr, "Invalid number of arguments\n");
    return 0;
  }
  else
  {
    return setenv(arguments[1], arguments[2], 1);
  }
}

int BI_unsetenv(char** arguments, int argc, int fd)
{
  if(argc!=2)
  {
    fprintf(stderr, "Invalid number of arguments");
    return 0;
  }
  else
  {
    return unsetenv(arguments[1]);
  }
}
/*
*int doBuiltIn
*Checks if the first argument given is a built-in
*command implemented in this file and runs it using
*the provided arguments.
*/
int doBuiltIn(char** arguments, int argc, int fd)
{
  /*printf("CALL\n");
  for(int i = 0; i<argc; i++)
    printf("%s\n",arguments[i]);*/
  if(argc>0)
  {
    for(int i = 0; i<functioncount; i++)
    {
      if(strcmp(arguments[0], commands[i].name)==0)
      {
        commands[i].func(arguments, argc, fd);
        return 1;
      }
    }
  }
  return 0;
}
