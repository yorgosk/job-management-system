#ifndef __CONSOLE_FUNCTIONS__
#define __CONSOLE_FUNCTIONS__

#include "console_header.h"

/* Global Variables */
extern char *jms_in, *jms_out, *msgbuf;

void handshake_with_coord();
int get_operations_from_file(char*);
void send_coord(char*);
int listen_coord();

#endif