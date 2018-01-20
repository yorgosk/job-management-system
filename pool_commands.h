#ifndef __POOL_COMMANDS__
#define __POOL_COMMANDS__

#include "pool_header.h"

extern char *pool_in, *pool_out, *path, *msgbuf;
extern int jobs_pool, num_of_total_jobs, num_of_local_jobs, num_of_running_jobs, exited_children;
extern jobInfo *job_arr;

void submit(char*);
void status(int);
void status_all(int);
void show_active();
void show_pools();
void show_finished();
void suspend(int);
void resume(int);

#endif