#include "pool_header.h"

char *pool_in, *pool_out, *path, *msgbuf;
int jobs_pool, num_of_total_jobs, num_of_local_jobs, num_of_running_jobs, exited_children;
jobInfo *job_arr;

int main(int argc, char* argv[]) {
	/* SET HANDLER FOR TERMINATION SIGNAL */
	signal(SIGTERM, terminate);

	/* SET UP BASED ON COMMAND LINE'S ARGUMENTS */
    pool_in = NULL;
    pool_out = NULL;
    path = NULL;
    msgbuf = NULL;
    num_of_local_jobs = 0;
    num_of_running_jobs = 0;
    job_arr = NULL;
    exited_children = 0;	// how many children-jobs have exited

    msgbuf = malloc(MSGSIZE);
    // memset to avoid memory-leaks because "Syscall param write(buf) points to uninitialised byte(s)"
    memset(msgbuf, '\0', MSGSIZE);
    
    // match variables with parameters' values
    pool_in = malloc(strlen(argv[0])+1);
    strcpy(pool_in, argv[0]);
    pool_out = malloc(strlen(argv[1])+1);
    strcpy(pool_out, argv[1]);
    path = malloc(strlen(argv[2])+1);
    strcpy(path, argv[2]);
    jobs_pool = atoi(argv[3]);
    num_of_total_jobs = atoi(argv[4]);
    // printf("I am a pool with pid = %d, pool_in = %s, pool_out = %s, path = %s, jobs_pool = %d, num_of_total_jobs = %d\n", getpid(), pool_in, pool_out, path, jobs_pool, num_of_total_jobs);

	/* INITIATE COMMUNICATION WITH JMS-COORDINATOR */
    handshake_with_coord();

    /* RECEIVING OPERATIONS FROM JMS-COORDINATOR */
    printf("pool: stand-by for operations\n");
    while(exited_children < jobs_pool) {
    	listen_coord();
    	exited_children += check_children();
    }
    
    printf("pool: maximum number of children have finished... Terminating... Goodbye\n");

    // free dynamically-allocated memory
    free(msgbuf);
    free(pool_in);
    free(pool_out);
    free(path);
	free(job_arr);

	// the pool returns as an exit status the number of it's children-jobs that are still in progress
	int still_in_progress = jobs_pool - exited_children;
	// printf("STILL IN PROGRESS =========================== %d\n", still_in_progress);
	exit(still_in_progress);
}