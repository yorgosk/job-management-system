#ifndef __COORD_HEADER__
#define __COORD_HEADER__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <dirent.h>
#include <math.h>	// for ceil()
#include <signal.h>

/* A struct to keep the necessary information to communicate with a pool-process */
typedef struct poolInfo {
	int id;				// pool serial number
	int pid;
	int active;			// 1 when the pool is "running", 0 when the pool has "finished"
	char* pool_in;
	char* pool_out;
	int num_of_jobs;	// how many jobs have been assigned to a pool
	int exit_status;	// pool's exit-status
} poolInfo;

#include "coord_functions.h"
#include "coord_commands.h"

#define MSGSIZE 512

/* Global Variables */
extern char *jms_in, *jms_out, *msgbuf, *pool_msgbuf, *path;
extern int jobs_pool, need_new_pool, num_of_pools, num_of_jobs, active_pools;
extern poolInfo *pool_arr;

#endif