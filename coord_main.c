#include "coord_header.h"

/* Global Variables */
char *jms_in, *jms_out, *msgbuf, *pool_msgbuf, *path;
int jobs_pool, need_new_pool, num_of_pools, num_of_jobs, active_pools;
poolInfo *pool_arr;

int main(int argc, char* argv[]) {
	/* SET UP BASED ON COMMAND LINE'S ARGUMENTS */
    if (argc < 9) {
        fprintf(stderr, "ERROR! Insufficient number of arguments!\n");
        exit(EXIT_FAILURE);
    }

    /* INITIALIZE LOCAL AND THEN GLOBAL VARIABLES */
    int lflag = 0, nflag = 0, wflag = 0, rflag = 0, i, over = 0;
    path = NULL;
    jms_in = NULL;
    jms_out = NULL; 
    msgbuf = NULL;
    pool_msgbuf = NULL;
    need_new_pool = 1;  // yes, we need a new pool (we have none)
    num_of_pools = 0;
    num_of_jobs = 0;
    pool_arr = NULL;
    
    msgbuf = malloc(MSGSIZE);
    // memset to avoid memory-leaks because "Syscall param write(buf) points to uninitialised byte(s)"
    memset(msgbuf, '\0', MSGSIZE);
    pool_msgbuf = malloc(MSGSIZE);
    // memset to avoid memory-leaks because "Syscall param write(buf) points to uninitialised byte(s)"
    memset(pool_msgbuf, '\0', MSGSIZE);

    for(i = 1; i < argc-1; i += 2) {        // match variables with parameters' values
    	if(!lflag && !strcmp(argv[i], "-l")) {
    		lflag = 1;
    		path = malloc(strlen(argv[i+1])+1);
    		strcpy(path, argv[i+1]);
    	} else if(!nflag && !strcmp(argv[i], "-n")) {
    		nflag = 1;
    		jobs_pool = atoi(argv[i+1]);
    	} else if(!wflag && !strcmp(argv[i], "-w")) {
            wflag = 1;
            jms_out = malloc(strlen(argv[i+1])+1);
            strcpy(jms_out, argv[i+1]);
        } else if(!rflag && !strcmp(argv[i], "-r")) {
            rflag = 1;
            jms_in = malloc(strlen(argv[i+1])+1);
            strcpy(jms_in, argv[i+1]);
        } else {
            fprintf(stderr, "ERROR! Bad argument formation!\n");
            exit(EXIT_FAILURE);
        }
    }

    if (!lflag || !nflag || !wflag || !rflag) {   // test if all the neccessary flags have been included
        fprintf(stderr, "ERROR! Arguments missing!\n");
        exit(EXIT_FAILURE);
    }

    // printf("jobs_pool = %d, path = %s, jms_in = %s, jms_out = %s\n", jobs_pool, path, jms_in, jms_out);
    /* SET-UP OUR ENVIRONMENT (FOLDERS mainly) */
    setup_environment();

    /* INITIATE COMMUNICATION WITH JMS-CONSOLE */
    handshake_with_console();
    strcpy(msgbuf, "-");

    /* RECEIVING OPERATIONS */
    printf("standing-by for operations\n");
    while(!over) {      // while we haven't been instructed to terminate
        over = listen_console();
        if(!over) check_pools();    // check for exiting pools if we are over, pools can be expected to have exited
    }
    printf("Coordinator terminating...\nGoodbye\n");

    // unlink the pipes that were used for communication with the console end the pools
    unlink(jms_in);
    unlink(jms_out);
    for(i = 0; i < num_of_pools; i++) {
        unlink(pool_arr[i].pool_in);
        unlink(pool_arr[i].pool_out);
    }

    // free dynamically-allocated memory
    free(path);
    free(jms_in);
    free(jms_out);
    free(msgbuf);
    free(pool_msgbuf);
    for(i = 0; i < num_of_pools; i++) {
        free(pool_arr[i].pool_in);
        free(pool_arr[i].pool_out);
    }
    free(pool_arr);

	return 0;
}