#ifndef __POOL_FUNCTIONS__
#define __POOL_FUNCTIONS__

#include "pool_header.h"

extern char *pool_in, *pool_out, *path, *msgbuf;
extern int jobs_pool, num_of_total_jobs, num_of_local_jobs, num_of_running_jobs, exited_children;
extern jobInfo *job_arr;

void handshake_with_coord();
void listen_coord();
void send_coord();
void manage_operation(char*);
int check_children();
void terminate(int);

#endif