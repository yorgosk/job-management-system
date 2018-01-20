#ifndef __CONSOLE_HEADER__
#define __CONSOLE_HEADER__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "console_functions.h"

#define MSGSIZE 512

/* Global Variables */
extern char *jms_in, *jms_out, *msgbuf;

#endif