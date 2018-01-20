#include "coord_functions.h"

void setup_environment() {
	/* create our directory */
	if((mkdir(path, S_IRWXU)) != 0) {
		perror("coordinator: mkdir problem\n");
		exit(EXIT_FAILURE);
	}
}

void make_coord_console_pipes() {
	/* MAKE FIFOS FOR COMMUNICATION WITH JMS-CONSOLE */
    /* make jms_in fifo, to read the console's messages */
    if(mkfifo(jms_in, 0666) == -1) {
        if(errno != EEXIST) {
            perror("coordinator: jms_in mkfifo problem\n");
            exit(EXIT_FAILURE);
        }
    }
    /* make jms_out fifo, to send (write) messages to the console */
    if(mkfifo(jms_out, 0666) == -1) {
        if(errno != EEXIST) {
            perror("coordinator: jms_out mkfifo problem\n");
            exit(EXIT_FAILURE);
        }
    }
}

void handshake_with_console() {
	int jms_in_fd, jms_out_fd, hand_shake = 0;
	/* make the pipes for communication with the console */
	make_coord_console_pipes();
    /* open jms_in fifo */
    while((jms_in_fd = open(jms_in, O_RDONLY | O_NONBLOCK)) < 0);
    /* wait for hand-shake request from console */
    while(!hand_shake) {
        while(read(jms_in_fd, msgbuf, MSGSIZE) < 0);
        if(!strcmp(msgbuf, "hey_coord")) {
            printf("hand_shake request : \"%s\" received\n", msgbuf);
            fflush(stdout);
            hand_shake = 1;
        }
    }
    /* open jms_out fifo */
    while((jms_out_fd = open(jms_out, O_WRONLY | O_NONBLOCK)) < 0);
    /* respond to hand_shake request */
    strcpy(msgbuf, "hey_console");
    while(write(jms_out_fd, msgbuf, MSGSIZE) < 0);
}

int listen_console() {
	int jms_in_fd, ret = 0;
	/* open jms_in fifo */
    while((jms_in_fd = open(jms_in, O_RDONLY | O_NONBLOCK)) < 0);
	/* stand-by for operation from console */
    while(read(jms_in_fd, msgbuf, MSGSIZE) < 0);
    if(strcmp(msgbuf, "-") != 0) {
	    ret = manage_operation(msgbuf);
	    strcpy(msgbuf, "-");
    }

	return ret;
}

void check_pools() {
	int status;
	pid_t pid = waitpid(-1, &status, WNOHANG);

	if(pid && pid != -1) {	// if a child-pool has exited
		printf("coord: PID: %d has exited\n", pid);
		int i;
		for(i = 0; i < num_of_pools; i++)	// mark that the exited child-pool is not running anymore
			if(pool_arr[i].pid == pid){
				pool_arr[i].active = 0;
				if(WIFEXITED(status)) pool_arr[i].exit_status = WEXITSTATUS(status);
				active_pools--;				// one less active pool
				break;
			}
	}
}

void send_console() {
	int jms_out_fd;
	/* open jms_out fifo */
    while((jms_out_fd = open(jms_out, O_WRONLY)) < 0);
    /* send the appropriate response to the console */
    while(write(jms_out_fd, msgbuf, MSGSIZE) < 0);
}

int manage_operation(char* operation) {
	check_pools();			// before doing anything, check the pools one more time

	int ret = 0;
	char* temp = NULL;      // temporary string, so that I don't lose the initial operation while tokenizing
    temp = (char*)malloc(strlen(operation)+1);
    strcpy(temp, operation);
    if (!strcmp(operation, "\n")) {
        free(temp);
        return 0;   // if just "enter" was pressed, return so we can loop again (in main)
    }
    
    char* op;
    op = strtok(temp, " \n");   // '\n' for bye which is followed by an immediate 'enter'
    if(!strcmp(op, "submit")) {
        ret = submit(operation);
    } else if (!strcmp(op, "status") && num_of_pools) {		// certain operations are pointless if there are no pools (no job has been submitted yet)
        ret = status(operation);
    } else if (!strcmp(op, "status-all") && num_of_pools) {
        ret = status_all(operation);
    } else if (!strcmp(op, "show-active") && num_of_pools) {
        ret = show_active(operation);
    } else if (!strcmp(op, "show-pools") && num_of_pools) {
        ret = show_pools(operation);
    } else if (!strcmp(op, "show-finished") && num_of_pools) {
        ret = show_finished(operation);
    } else if (!strcmp(op, "suspend") && num_of_pools) {
        ret = suspend(operation);
    } else if (!strcmp(op, "resume") && num_of_pools) {
        ret = resume(operation);
    } else if (!strcmp(op, "shutdown")) {
        ret = shutdown(operation);
    } else {
        // printf("coord: Operation: %s not recognised. Try again.\n", op);
        strcpy(msgbuf, "Bad operation. Please, try again.");
        send_console();
    }
    free(temp); // free memory allocated for temporary string

    return ret;
}

void create_pool() {
	char *bufIn = NULL, *bufOut = NULL, num[10], jobsbuf[10];
	bufIn = malloc(20);
	// memset to avoid memory-leaks because "Syscall param write(buf) points to uninitialised byte(s)"
	memset(bufIn, '\0', 20);
	bufOut = malloc(20);
	// memset to avoid memory-leaks because "Syscall param write(buf) points to uninitialised byte(s)"
	memset(bufOut, '\0', 20);
	
	/* record the pool's info */
	num_of_pools++;	// one more pool in total
	active_pools++;	// one more active pool
	pool_arr = realloc(pool_arr, num_of_pools*sizeof(poolInfo));
	if(!pool_arr) {
		perror("coordinator: pool_arr realloc problem\n");
		exit(EXIT_FAILURE);
	}
	pool_arr[num_of_pools-1].id = num_of_pools;
	pool_arr[num_of_pools-1].active = 1;
	pool_arr[num_of_pools-1].num_of_jobs = 0;
	pool_arr[num_of_pools-1].pool_in = NULL;
	pool_arr[num_of_pools-1].pool_out = NULL;
	pool_arr[num_of_pools-1].exit_status = 0;
	
	strcpy(bufIn, "pool");
	sprintf(num, "%d", num_of_pools);
	strcat(bufIn, num);
	strcat(bufIn, "_in");
	pool_arr[num_of_pools-1].pool_in = malloc(strlen(bufIn)+1);
	strcpy(pool_arr[num_of_pools-1].pool_in, bufIn);

	strcpy(bufOut, "pool");
	sprintf(num, "%d", num_of_pools);
	strcat(bufOut, num);
	strcat(bufOut, "_out");
	pool_arr[num_of_pools-1].pool_out = malloc(strlen(bufOut)+1);
	strcpy(pool_arr[num_of_pools-1].pool_out, bufOut);

	sprintf(num, "%d", jobs_pool);
	sprintf(jobsbuf, "%d", num_of_jobs);
	
	make_pool_coord_pipes();
	/* fork-exec pool */
	int pid = fork();
	if (pid < 0) {
		perror("coordinator: fork problem\n");
		exit(EXIT_FAILURE);
	}else if(pid == 0) {	// if you are a child (we don't want exec to "overwrite" the coordinator)
		// printf("coord's child\n");
		char *argv[6] = {pool_arr[num_of_pools-1].pool_in, pool_arr[num_of_pools-1].pool_out, path, num, jobsbuf, NULL};
		execvp("./pool", argv);
	}else {
		// printf("coord: parent\n");
		pool_arr[num_of_pools-1].pid = pid;
	}
	/* handshake with the pool */
	handshake_with_pool();
	strcpy(pool_msgbuf, "-");

	free(bufIn);
	free(bufOut);
}

void make_pool_coord_pipes() {
	/* MAKE FIFOS FOR COMMUNICATION WITH POOL */
    /* make pool_in fifo, to send messages to the pool */
    if(mkfifo(pool_arr[num_of_pools-1].pool_in, 0666) == -1) {
        if(errno != EEXIST) {
            perror("coordinator: pool_in mkfifo problem\n");
            exit(EXIT_FAILURE);
        }
    }
    /* make pool_out fifo, to receive messages from the pool */
    if(mkfifo(pool_arr[num_of_pools-1].pool_out, 0666) == -1) {
        if(errno != EEXIST) {
            perror("coordinator: pool_out mkfifo problem\n");
            exit(EXIT_FAILURE);
        }
    }
}

void handshake_with_pool() {
	int pool_in_fd, pool_out_fd, hand_shake = 0;
	/* open pool_out fifo */
    while((pool_out_fd = open(pool_arr[num_of_pools-1].pool_out, O_RDONLY | O_NONBLOCK)) < 0);
    /* wait for hand-shake acceptance from pool */
    while(!hand_shake) {
        while(read(pool_out_fd, pool_msgbuf, MSGSIZE) < 0);
        if(!strcmp(pool_msgbuf, "hey_coord")) {
            printf("coord: hand_shake request: \"%s\" received\n", pool_msgbuf);
            fflush(stdout);
            hand_shake = 1;
        }
    }
    /* open pool_in fifo */
    while((pool_in_fd = open(pool_arr[num_of_pools-1].pool_in, O_WRONLY | O_NONBLOCK)) < 0);
    /* send hand_shake request to the pool */
    strcpy(pool_msgbuf, "hey_pool");
    while(write(pool_in_fd, pool_msgbuf, MSGSIZE) < 0);
}

void send_pool(char* operation) {
	int pool_in_fd, pool_out_fd;
    /* open pool_in fifo */
    while((pool_in_fd = open(pool_arr[num_of_pools-1].pool_in, O_WRONLY)) < 0);
	/* send operation to the pool */
    strcpy(pool_msgbuf, operation);
    while(write(pool_in_fd, pool_msgbuf, MSGSIZE) < 0);
    /*wait for pool's response*/
    /* open pool_out fifo */
	while((pool_out_fd = open(pool_arr[num_of_pools-1].pool_out, O_RDONLY)) < 0);
	/* wait for the pool's response */
	while(read(pool_out_fd, pool_msgbuf, MSGSIZE) < 0);
	if(strcmp(pool_msgbuf, "-") != 0){
	   	// printf("coord: pool no.%d says: %s\n", pool_arr[num_of_pools-1].id, pool_msgbuf);
	   	strcpy(msgbuf, pool_msgbuf);
	   	send_console();
	   	strcpy(pool_msgbuf, "-");
	}
}
