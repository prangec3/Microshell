/* CS 347 -- Mini Shell!
 *
 *   Sept 21, 2000,  Phil Nelson
 *   Modified April 8, 2001
 *   Modified January 6, 2003
 *
 *   Cody Pranger CSCI 347 Fall 2017
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include "arg_parse.h"
#include "builtin.h"


/* Constants */

#define LINELEN 1024

/* Prototypes */

void processline (char *line,char* buf, int fd);
int expand(char* orig, char* newbuf, int newSize);
int intlen(int x);
char* findRight(char* orig);
static void handle_sigint(int signo);
/*Buffer used for storing expanded line*/
char newBuf[LINELEN];
char cmdBuf[LINELEN];
/*Defined here because Atom gets confused by curly braces in quotes.*/
static const char LCB ='{';
static const char RCB ='}';

/* Shell main */

int
main (void)
{
    char   buffer [LINELEN];
    int    len;
    signal(SIGINT,handle_sigint);
    while (1) {

        /* prompt and get line */
	fprintf (stderr, "%% ");
	if (fgets (buffer, LINELEN, stdin) != buffer)
	  break;

        /* Get rid of \n at end of buffer. */
	len = strlen(buffer);
	if (buffer[len-1] == '\n')
	    buffer[len-1] = 0;

	/* Run it ... */
	processline (buffer,newBuf, STDOUT_FILENO);

    }

    if (!feof(stdin))
        perror ("read");

    return 0;		/* Also known as exit (0); */
}

static void handle_sigint(int signo)
{
  return;
}

/*Line: The command. buf: Where the expanded line is stored.
 Fd: Where the output goes (default stdout)*/
void processline (char *line,char* buf, int fd)
{
    pid_t  cpid;
    int    status;
    int    count;
    /*Expand variables like $$ or ${VAR}*/
    if(!expand(line,buf,LINELEN-strlen(buf)))
      return;
    line = buf;
    /*Tokenize expanded line*/
    char** arguments = arg_parse(line, &count);

    /*Return if we have no arguments*/
    if(arguments==NULL)
      return;

    if(doBuiltIn(arguments, count, fd))
    {
      free(arguments);
      return;
    }
    /* Start a new process to do the job. */
    cpid = fork();
    if (cpid < 0) {
      perror ("fork");
      free(arguments);
      return;
    }

    /* Check for who we are! */
    if (cpid == 0) {
      /* We are the child! */
      dup2(fd,STDOUT_FILENO);
      execvp(arguments[0], arguments);
      perror ("exec");
      exit (127);
    }

    /* Have the parent wait for child to complete */
    /*The do-while was something the book said to do
    that likely isn't needed anymore that I added while confused.
    So that's why it's around pretty much every system call*/
    do {
      if (wait(&status) < 0)
        perror ("wait");
    } while(errno==EINTR);
    free(arguments);
}

int expand(char* orig, char* newbuf, int newsize)
{
  /*State 0: not encountering an arg
    State 1: Initial Dollar sign*/
  int state = 0;
  /*Count of characters in the new buffer*/
  /*Starts at 1 to ensure room for terminating \0*/
  int count = 1;
  pid_t pid = getpid();
  /*Ex: HOME*/
  char* varname;
  /*Ex: home/prangec3*/
  char* var;
  while(*orig!='\0' && count<newsize)
  {
    if(state == 0 && *orig == '$')
    {
      //Found a $, print but don't increment in case of overwrite
      state = 1;
      snprintf(newbuf,newsize-count,"%c",'$');
      orig++;
      continue;
    }
    else if(state==1)
    /*Previous thing was a $*/
    {
      if(*orig=='$')
      {
        /*Found $$, print the PID*/
        state = 0;
        snprintf(newbuf,newsize-count,"%d",pid);
        int length = intlen((int)pid);
        newbuf+=length;
        count += length;
        orig++;
      }
      else if(*orig==LCB)
      {
        /*LCB encountered, prepare for a variable name*/
        state = 0;
        orig++;
        /*Found pattern ${}*/
        /*If I don't stop here it segfaults*/
        if(*orig == RCB)
        {
          fprintf(stderr,"Invalid argument\n");
          return 0;
        }
        /*Checking if a right curly brace exists*/
        /*If not, throw an error and stop processing*/
        char* RCBFind = strstr(orig,"}");
        if(RCBFind == NULL)
        {
          fprintf(stderr,"Unmatched curly brace\n");
          return 0;
        }
        /*Collect everything between here and the right brace.*/
        varname = strtok(orig,&RCB);
        var = getenv(varname);
        if(var == NULL)
        {
          fprintf(stderr, "Invalid argument\n");
          return 0;
        }
        snprintf(newbuf,newsize-count,"%s",var);
        int l = strlen(var);
        count+=l;
        newbuf += l;
        /*Restore the original RCB*/
        orig+=strlen(varname);
        *orig=RCB;
        orig++;
      }
      else if(*orig=='(')
      {
        /*Command Expansion*/
        state = 0;
        orig++;
        /*Find the matching )*/
        char* rightparen = findRight(orig);
        if(rightparen==NULL)
        {
          fprintf(stderr,"Right parenthesis not found\n");
          return 0;
        }
        /*Isolate the command string*/
        *rightparen = '\0';
        /*Call a pipe*/
        int fd[2];
        do {
          if(pipe(fd)<0)
          {
            fprintf(stderr,"Pipe");
          }
        } while(errno==EINTR);
        /*Recur on the isolated command*/
        processline(orig,newbuf,fd[1]);
        close(fd[1]);
          /*Put the right parenthesis back*/
          *rightparen = ')';
          /*Read from the pipe*/
          do{
          read(fd[0],cmdBuf,LINELEN);
          }
          while(errno==EINTR);
          close(fd[0]);
          /*Print the output to the expand buffer*/
          int len = strlen(cmdBuf);
          snprintf(newbuf,newsize-count,"%s",cmdBuf);
          newbuf+=len-1;
          count+=len-1;
          orig=rightparen+1;
          /*Clear out the Buffer*/
          memset(cmdBuf,'\0',len);

      }
      else
      {
        /*The dollar sign didn't have anything meaningful after it*/
        /*Leave the $ and print another character*/
        newbuf++;
        count++;
        snprintf(newbuf,newsize-count,"%c",*orig);
        state=0;
        newbuf++;
        count++;
        orig++;
      }
    }
  else
  {
    /*Nothing interesting, copy a char and move on*/
    snprintf(newbuf,newsize-count,"%c",*orig);
    newbuf++;
    count++;
    orig++;
  }
}
newbuf++;
*newbuf = '\0';
return 1;
}

/*Finds the matching ) in the string
Assuming the character before orig is a ( */
char* findRight(char* orig)
{
  int leftcount = 1;
  int rightcount = 0;
  while(*orig != '\0')
  {
    if(*orig == '(')
    {
      leftcount++;
    }
    if(*orig == ')')
    {
      rightcount++;
      if(rightcount == leftcount)
      {
        return orig;
      }
    }
    orig++;
  }
  return NULL;
}

int intlen(int x)
{
  int count = 0;
  while(x>0)
  {
    x/=10;
    count++;
  }
  return count;
}
