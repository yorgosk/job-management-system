#include "pool_functions.h"

void handshake_with_coord() {
	int pool_in_fd, pool_out_fd, hand_shake = 0;
    /* open pool_out fifo */
    while((pool_out_fd = open(pool_out, O_WRONLY | O_NONBLOCK)) < 0);
    /* respond to hand-shake request by coordinator */
    strcpy(msgbuf, "hey_coord");
    while(write(pool_out_fd, msgbuf, MSGSIZE) < 0);
	/* open pool_in fifo */
    while((pool_in_fd = open(pool_in, O_RDONLY | O_NONBLOCK)) < 0);
    /* stand-by for hand-shake request from coordinator */
    while(!hand_shake) {
        while(read(pool_in_fd, msgbuf, MSGSIZE) < 0);
        if(!strcmp(msgbuf, "hey_pool")) {
            printf("hand_shake acceptance : \"%s\" received\n", msgbuf);
            fflush(stdout);
            hand_shake = 1;
        }
    }
}

void listen_coord() {
	int pool_in_fd;
	/* open pool_in fifo */
    while((pool_in_fd = open(pool_in, O_RDONLY | O_NONBLOCK)) < 0);
	/* stand-by for job assignment from coordinator */
    while(read(pool_in_fd, msgbuf, MSGSIZE) < 0);
    if(strcmp(msgbuf, "-") != 0) {
    	char *operation = NULL;
    	operation = malloc(strlen(msgbuf)+1);
    	strcpy(operation, msgbuf);
    	manage_operation(operation);
    	strcpy(msgbuf, "-");
    	free(operation);
    }
}

void send_coord() {
	int pool_out_fd;
	/* open pool_out fifo */
    while((pool_out_fd = open(pool_out, O_WRONLY)) < 0);
    /* respond to the coordinator */
    while(write(pool_out_fd, msgbuf, MSGSIZE) < 0);
}

void manage_operation(char* operation) {
	exited_children += check_children();	// before doing anything, check the children one more time

	char* tok;

	tok = strtok(operation, " \n");	// take the operation's first word
	if(!strcmp(tok, "submit")) {
		tok = strtok(NULL, "\n");	// take the whole job
		submit(tok);
	} else if(!strcmp(tok, "status")) {
		tok = strtok(NULL, "\n");	// take the JobID to string
		int jobid = atoi(tok);
		status(jobid);
	} else if(!strcmp(operation, "status-all")) {
		int time_duration = -1;
		tok = strtok(NULL, "\n");		// if there is a time duration set, take it
		if(tok) {
			time_duration = atoi(tok);
			// printf("time_duration --------------> %d\n", time_duration);
		}
		status_all(time_duration);
	} else if(!strcmp(operation, "show-active")) {
		show_active();
	} else if(!strcmp(operation, "show-pools")) {
    	show_pools();
	} else if(!strcmp(operation, "show-finished")) {
    	show_finished();
	} else if(!strcmp(operation, "suspend")) {
    	tok = strtok(NULL, "\n");	// take the JobID to string
		int jobid = atoi(tok);
		suspend(jobid);
	} else if(!strcmp(operation, "resume")) {
    	tok = strtok(NULL, "\n");	// take the JobID to string
		int jobid = atoi(tok);
		resume(jobid);
	} 
}

int check_children() {
	int status, ret = 0;
	pid_t pid = waitpid(-1, &status, WNOHANG);

	if(pid && pid != -1) {	// if a child has exited
		// printf("pool: PID: %d has exited\n", pid);
		int i;
		for(i = 0; i < num_of_local_jobs; i++)	// mark that the exited child is not running anymore
			if(job_arr[i].pid == pid && job_arr[i].running == 1){
				job_arr[i].running = 0;
				num_of_running_jobs--;			// one less job running in pool
				ret = 1;
				break;
			}
	}

	return ret;
}

void terminate(int signum) {
	signal(SIGTERM, terminate);		// re-establish disposition of the signal SIGTERM

	exited_children += check_children();	// before doing anything, check the children one last time

	int i, still_in_progress = 0;
	for(i = 0; i < num_of_local_jobs; i++) {
		// if a job is currently running or suspended, it means that it is still in progress (not yet finished)
		if(job_arr[i].running == 1 || job_arr[i].running == -1) {
			still_in_progress++;
			kill(job_arr[i].pid, SIGTERM);	// send sigterm to the still-in-progress child
			exited_children += check_children();
		}
	}

	// free dynamically-allocated memory
    free(msgbuf);
    free(pool_in);
    free(pool_out);
    free(path);
	free(job_arr);

	// printf("STILL IN PROGRESS =========================== %d\n", still_in_progress);
	exit(still_in_progress);
}