#ifndef __COORD_COMMANDS__
#define __COORD_COMMANDS__

#include "coord_header.h"

/* Global Variables */
extern char *jms_in, *jms_out, *msgbuf, *pool_msgbuf, *path;
extern int jobs_pool, need_new_pool, num_of_pools, num_of_jobs, active_pools;
extern poolInfo *pool_arr;

int submit(char*);
int status(char*);
int status_all(char*);
int show_active(char*);
int show_pools(char*);
int show_finished(char*);
int suspend(char*);
int resume(char*);
int shutdown(char*);

#endif