#ifndef __POOL_HEADER__
#define __POOL_HEADER__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

/* A struct to keep the necessary information about each executed job */
typedef struct jobInfo {
	int jobid;				// job's serial number
	int pid;				// job's pid
	time_t start_time;
	int running;			// -1 if suspended, 0 if exited, 1 if active
} jobInfo;

#include "pool_functions.h"
#include "pool_commands.h"

#define MSGSIZE 512

extern char *pool_in, *pool_out, *path, *msgbuf;
extern int jobs_pool, num_of_total_jobs, num_of_local_jobs, num_of_running_jobs, exited_children;
extern jobInfo *job_arr;

#endif