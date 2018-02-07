/*
*Cody Pranger
*CSCI 347 Fall 2017
*arg_parse.h;
*/
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
char** arg_parse(char *line, int *argcp);
