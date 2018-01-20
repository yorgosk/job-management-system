#ifndef __COORD_FUNCTIONS__
#define __COORD_FUNCTIONS__

#include "coord_header.h"

/* Global Variables */
extern char *jms_in, *jms_out, *msgbuf, *pool_msgbuf, *path;
extern int jobs_pool, need_new_pool, num_of_pools, num_of_jobs, active_pools;
extern poolInfo *pool_arr;

void setup_environment();
void make_coord_console_pipes();
void handshake_with_console();
int listen_console();
void check_pools();
void send_console();
int manage_operation(char*);
void create_pool();
void make_pool_coord_pipes();
void handshake_with_pool();
void send_pool(char*);

#endif