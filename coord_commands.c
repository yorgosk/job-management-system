#include "coord_commands.h"

int submit(char* operation) {
	/* CREATE A NEW POOL IF NEEDED */
	if(need_new_pool) create_pool();
	/* SEND THE OPERATION TO THE MOST RECENTLY CREATED POOL */
	send_pool(operation);

	num_of_jobs++;	// we have one more submitted job, in total
	pool_arr[num_of_pools-1].num_of_jobs++;
	// check whether the next time we will submit a job we are going to need a new pool
	if(pool_arr[num_of_pools-1].num_of_jobs == jobs_pool) need_new_pool = 1;
	else need_new_pool = 0;

	return 0;
}

int status(char* operation) {
	char *tok, *temp_op = NULL;
	
	temp_op = malloc(strlen(operation)+1);
	strcpy(temp_op, operation);	// so that we don't lose operation while tokenizing
	tok = strtok(temp_op, " ");	// take "status"
	tok = strtok(NULL, "\n");		// take <JobID> as a string
	
	int jobid = atoi(tok), pool = ceil((float)jobid/(float)jobs_pool);

	if(jobid > num_of_jobs) {	// recognise bad JobID
		strcpy(msgbuf, "JobID not found. Please, try again.");
		send_console();
		return 0;				// then return
	}
	// printf("coord: looking for status of JobID = %d in Pool no.%d\n", jobid, pool);
	if (pool_arr[pool-1].active) {	// if pool is active, then pass the question to the pool
		// printf("coord: sending pool: %s\n", operation);
		send_pool(operation);
	} else {	// if pool is not active, then the job must have finished
		/* formulate the response to the question */
		char response[40];
		strcpy(response, "JobID ");
		strcat(response, tok);
		strcat(response, " Status: Finished");
		strcpy(msgbuf, response);
		send_console();	// send the response to the console
	}

	free(temp_op);

	return 0;
}

int status_all(char* operation) {
	int pool_in_fd, pool_out_fd, i;
	char *op = NULL;
	op = malloc(strlen(operation)+1);
	strcpy(op, operation);		// keep original operation
	/* tell console to wait for a batch of answers */
	strcpy(msgbuf, "wait-batch");
	send_console();

	for(i = 0; i < num_of_pools; i++) {
		check_pools();			// before moving forward, check the pools one more time

		if(pool_arr[i].active) {
			// printf("coord: sending %s op to pool no.%d\n", op, i);
			/* open pool_in fifo */
		    while((pool_in_fd = open(pool_arr[i].pool_in, O_WRONLY)) < 0);
			/* send operation to the pool */
		    strcpy(pool_msgbuf, op);
		    while(write(pool_in_fd, pool_msgbuf, MSGSIZE) < 0);
		    /* open pool_out fifo */
			while((pool_out_fd = open(pool_arr[i].pool_out, O_RDONLY)) < 0);
			int done = 0;
			while(!done) {
				/* wait for the pool's response */
				while(read(pool_out_fd, pool_msgbuf, MSGSIZE) < 0);
				if(strcmp(pool_msgbuf, "-") != 0 && strcmp(pool_msgbuf, "pool-batch-over")){
				   	// printf("coord: status_all: pool no.%d says: %s\n", pool_arr[i].id, pool_msgbuf);
				   	strcpy(msgbuf, pool_msgbuf);
				   	send_console();
				} else if(!strcmp(pool_msgbuf, "pool-batch-over")) done = 1;
				strcpy(pool_msgbuf, "-");
			}
		}
	}
	free(op);
	/* tell console that the batch of answers is over, so it can move on */
	strcpy(msgbuf, "batch-over");
	send_console();

	return 0;
}

int show_active(char* operation) {
	int pool_in_fd, pool_out_fd, i, job_index;
	char *op = NULL;
	op = malloc(strlen(operation)+1);
	strcpy(op, operation);		// keep original operation
	/* tell console to wait for a batch of answers */
	strcpy(msgbuf, "wait-batch");
	send_console();
	strcpy(msgbuf, "Active jobs:");
	send_console();

	job_index = 0;
	for(i = 0; i < num_of_pools; i++) {
		check_pools();			// before moving forward, check the pools one more time

		if(pool_arr[i].active) {	// a pool that is not active can't have active jobs
			// printf("coord: sending %s op to pool no.%d\n", op, i);
			/* open pool_in fifo */
		    while((pool_in_fd = open(pool_arr[i].pool_in, O_WRONLY)) < 0);
			/* send operation to the pool */
		    strcpy(pool_msgbuf, op);
		    while(write(pool_in_fd, pool_msgbuf, MSGSIZE) < 0);
		    /* open pool_out fifo */
			while((pool_out_fd = open(pool_arr[i].pool_out, O_RDONLY)) < 0);
			int done = 0;
			while(!done) {
				/* wait for the pool's response */
				while(read(pool_out_fd, pool_msgbuf, MSGSIZE) < 0);
				if(strcmp(pool_msgbuf, "-") != 0 && strcmp(pool_msgbuf, "pool-batch-over")){
				   	// printf("coord: status_all: pool no.%d says: %s\n", pool_arr[i].id, pool_msgbuf);
					char response[40], index[10];
				   	job_index++;	// one more active job
				   	sprintf(index, "%d. ", job_index);
				   	strcpy(response, index);
				   	strcat(response, pool_msgbuf);

				   	strcpy(msgbuf, response);
				   	send_console();
				} else if(!strcmp(pool_msgbuf, "pool-batch-over")) done = 1;
				strcpy(pool_msgbuf, "-");
			}
		}
	}
	free(op);
	/* tell console that the batch of answers is over, so it can move on */
	strcpy(msgbuf, "batch-over");
	send_console();

	return 0;
}

int show_pools(char* operation) {
	int pool_in_fd, pool_out_fd, i, pool_index;
	char *op = NULL;
	op = malloc(strlen(operation)+1);
	strcpy(op, operation);		// keep original operation
	/* tell console to wait for a batch of answers */
	strcpy(msgbuf, "wait-batch");
	send_console();
	strcpy(msgbuf, "Pool & NumOfJobs:");
	send_console();

	pool_index = 0;
	for(i = 0; i < num_of_pools; i++) {
		check_pools();			// before moving forward, check the pools one more time

		if(pool_arr[i].active) {	// a pool that is not active can't have active jobs
			// printf("coord: sending %s op to pool no.%d\n", op, i);
			/* open pool_in fifo */
		    while((pool_in_fd = open(pool_arr[i].pool_in, O_WRONLY)) < 0);
			/* send operation to the pool */
		    strcpy(pool_msgbuf, op);
		    while(write(pool_in_fd, pool_msgbuf, MSGSIZE) < 0);
		    /* open pool_out fifo */
			while((pool_out_fd = open(pool_arr[i].pool_out, O_RDONLY)) < 0);
			/* wait for the pool's response */
			while(read(pool_out_fd, pool_msgbuf, MSGSIZE) < 0);
			if(strcmp(pool_msgbuf, "-") != 0){
				// printf("coord: status_all: pool no.%d says: %s\n", pool_arr[i].id, pool_msgbuf);
				if(strcmp(pool_msgbuf, "0") != 0) {	// if pool has jobs running
					char response[40], pool_pid[10], index[10];
					pool_index++;	// one more active job
						
					sprintf(index, "%d. ", pool_index);
					strcpy(response, index);
					sprintf(pool_pid, "%d ", pool_arr[i].pid);
					strcat(response, pool_pid);
					strcat(response, pool_msgbuf);

					strcpy(msgbuf, response);
					send_console();
				}
			}
			strcpy(pool_msgbuf, "-");
		}
	}
	free(op);
	/* tell console that the batch of answers is over, so it can move on */
	strcpy(msgbuf, "batch-over");
	send_console();

	return 0;
}

int show_finished(char* operation) {
	int pool_in_fd, pool_out_fd, i, job_index;
	char *op = NULL;
	op = malloc(strlen(operation)+1);
	strcpy(op, operation);		// keep original operation
	/* tell console to wait for a batch of answers */
	strcpy(msgbuf, "wait-batch");
	send_console();
	strcpy(msgbuf, "Finished jobs:");
	send_console();

	job_index = 0;
	for(i = 0; i < num_of_pools; i++) {
		check_pools();			// before moving forward, check the pools one more time

		if(pool_arr[i].active) {	// a pool that is not active can't have active jobs
			// printf("coord: sending %s op to pool no.%d\n", op, i);
			/* open pool_in fifo */
		    while((pool_in_fd = open(pool_arr[i].pool_in, O_WRONLY)) < 0);
			/* send operation to the pool */
		    strcpy(pool_msgbuf, op);
		    while(write(pool_in_fd, pool_msgbuf, MSGSIZE) < 0);
		    /* open pool_out fifo */
			while((pool_out_fd = open(pool_arr[i].pool_out, O_RDONLY)) < 0);
			int done = 0;
			while(!done) {
				/* wait for the pool's response */
				while(read(pool_out_fd, pool_msgbuf, MSGSIZE) < 0);
				if(strcmp(pool_msgbuf, "-") != 0 && strcmp(pool_msgbuf, "pool-batch-over")){
				   	// printf("coord: status_all: pool no.%d says: %s\n", pool_arr[i].id, pool_msgbuf);
					char response[40], index[10];
				   	job_index++;	// one more active job
				   	sprintf(index, "%d. ", job_index);
				   	strcpy(response, index);
				   	strcat(response, pool_msgbuf);

				   	strcpy(msgbuf, response);
				   	send_console();
				} else if(!strcmp(pool_msgbuf, "pool-batch-over")) done = 1;
				strcpy(pool_msgbuf, "-");
			}
		} else {
			int j;
			for(j = 0; j < jobs_pool; j++) {
				char response[40], num[10];
				job_index++;	// one more active job
				sprintf(num, "%d. ", job_index);
				strcpy(response, num);
				strcat(response, "JobID ");
				sprintf(num, "%d", pool_arr[i].id+j);
				strcat(response, num);

				strcpy(msgbuf, response);
				send_console();
			}
		}
	}
	free(op);
	/* tell console that the batch of answers is over, so it can move on */
	strcpy(msgbuf, "batch-over");
	send_console();

	return 0;
}

int suspend(char* operation) {
	char *tok, *temp_op = NULL;
	
	temp_op = malloc(strlen(operation)+1);
	strcpy(temp_op, operation);	// so that we don't lose operation while tokenizing
	tok = strtok(temp_op, " ");	// take "suspend"
	tok = strtok(NULL, "\n");		// take <JobID> as a string
	
	int jobid = atoi(tok), pool = ceil((float)jobid/(float)jobs_pool);

	if(jobid > num_of_jobs) {	// recognise bad JobID
		strcpy(msgbuf, "JobID not found. Please, try again.");
		send_console();
		return 0;				// then return
	}
	
	check_pools();			// before moving forward, check the pools one more time

	// printf("coord: looking to suspend JobID = %d in Pool no.%d\n", jobid, pool);
	if (pool_arr[pool-1].active) {	// if pool is active, then pass the question to the pool
		// printf("coord: sending pool: %s\n", operation);
		send_pool(operation);
	} else {	// if pool is not active, then the job must have already finished
		/* formulate the response to the question */
		char response[40];
		strcpy(response, "JobID ");
		strcat(response, tok);
		strcat(response, " has already finished");
		strcpy(msgbuf, response);
		send_console();	// send the response to the console
	}

	free(temp_op);

	return 0;
}

int resume(char* operation) {
	char *tok, *temp_op = NULL;
	
	temp_op = malloc(strlen(operation)+1);
	strcpy(temp_op, operation);	// so that we don't lose operation while tokenizing
	tok = strtok(temp_op, " ");	// take "suspend"
	tok = strtok(NULL, "\n");		// take <JobID> as a string
	
	int jobid = atoi(tok), pool = ceil((float)jobid/(float)jobs_pool);

	if(jobid > num_of_jobs) {	// recognise bad JobID
		strcpy(msgbuf, "JobID not found. Please, try again.");
		send_console();
		return 0;				// then return
	}

	check_pools();			// before moving forward, check the pools one more time

	// printf("coord: looking to resume JobID = %d in Pool no.%d\n", jobid, pool);
	if (pool_arr[pool-1].active) {	// if pool is active, then pass the question to the pool
		// printf("coord: sending pool: %s\n", operation);
		send_pool(operation);
	} else {	// if pool is not active, then the job must have already finished
		/* formulate the response to the question */
		char response[40];
		strcpy(response, "JobID ");
		strcat(response, tok);
		strcat(response, " has already finished");
		strcpy(msgbuf, response);
		send_console();	// send the response to the console
	}

	free(temp_op);

	return 0;
}

int shutdown(char* operation) {
	char *op = NULL;
	op = malloc(strlen(operation)+1);
	strcpy(op, operation);		// keep original operation
	/* tell console to wait for a batch of answers */
	strcpy(msgbuf, "shutting-down");
	send_console();

	int i, still_in_progress = 0, status;	//, in_prog;
	for(i = 0; i < num_of_pools; i++) {	// go through all the pools
		check_pools();			// before moving forward, check the pools one more time

		if(pool_arr[i].active) {		// there is no point to terminate an already terminated pool
			if(kill(pool_arr[i].pid, SIGTERM) < 0) {		// send termination signal
				perror("coord: kill with SIGTERM problem\n");
				exit(EXIT_FAILURE);
			}
			waitpid(pool_arr[i].pid, &status, 0);	// wait for child-pool to exit
			pool_arr[i].active = 0;
			if(WIFEXITED(status)) pool_arr[i].exit_status = WEXITSTATUS(status);
			active_pools--;				// one less active pool
		}
	}

	// each pool's exit status represents how many jobs were still in progress by the time it was asked to shut down
	for(i = 0; i < num_of_pools; i++) still_in_progress += pool_arr[i].exit_status;

	/* formulate the response to the console */
	char response[60], num[10];
	strcpy(response, "Served ");
	sprintf(num, "%d", num_of_jobs);
	strcat(response, num);
	strcat(response, " jobs, ");
	sprintf(num, "%d", still_in_progress);
	strcat(response, num);
	strcat(response, " were still in progress");

	strcpy(msgbuf, response);
	send_console();	// send the response to the console

	free(op);

	return 1;
}