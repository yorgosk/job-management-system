#include "console_functions.h"

void handshake_with_coord() {
	int jms_in_fd, jms_out_fd, hand_shake = 0;

	while(!hand_shake) {
		/* open jms_in fifo */
	    while((jms_in_fd = open(jms_in, O_WRONLY | O_NONBLOCK)) < 0);
	    /* initiate hand-shake process with coordinator */
	    strcpy(msgbuf, "hey_coord");
	    while(write(jms_in_fd, msgbuf, MSGSIZE) < 0);
	    /* open jms_out fifo */
	    while((jms_out_fd = open(jms_out, O_RDONLY | O_NONBLOCK)) < 0);
	    /* stand-by for hand-shake response by coordinator */
	    while(!hand_shake) {
	        while(read(jms_out_fd, msgbuf, MSGSIZE) < 0);
	        if(!strcmp(msgbuf, "hey_console")) {
	            printf("hand_shake response : \"%s\" received\n", msgbuf);
	            fflush(stdout);
	            hand_shake = 1;
	        }
	    }
	}
}

int get_operations_from_file(char* filename) {
	int ret = 0;
	FILE *fp;

	fp = fopen(filename, "r");	// open the file that is expected to contain the operations
	if(fp == NULL) {
		fprintf(stderr, "ERROR! Unable to open operationsFile file named %s\n", filename);
		ret = -1;
		return ret;
	}

	char* operation = NULL;   	// forces getline to allocate with malloc
    size_t len = 0; 			// ignored when line=NULL
    ssize_t read;
    int shutdown = 0;

    while(!shutdown && (read = getline(&operation, &len, fp)) != -1) {
    	// printf("console: get_operations_from_file: operation = %s\n", operation);
    	printf("processing...\n");
        send_coord(operation);
        shutdown = listen_coord();
        printf("\n");   	// for aesthetic reasons
    }
    free(operation);  		// free memory allocated by getline

    ret = shutdown;			// the file might have told us to shut-down
	
	return ret;
}

void send_coord(char* operation) {
	int jms_in_fd;
	/* open jms_in fifo */
	while((jms_in_fd = open(jms_in, O_WRONLY)) < 0);
	/* send operation to coordinator */
	strcpy(msgbuf, operation);
	while(write(jms_in_fd, msgbuf, MSGSIZE) < 0);
}

int listen_coord() {
	int jms_out_fd, shutdown = 0;
	/* open jms_out fifo */
	while((jms_out_fd = open(jms_out, O_RDONLY)) < 0);
	/* stand-by for coordinator's response */
	if((read(jms_out_fd, msgbuf, MSGSIZE)) < 0) {
		perror("console: jms_out_fd read problem\n");
		exit(EXIT_FAILURE);
	}
	/* process what has been read from the coordinator */
	if(!strcmp(msgbuf, "wait-batch")) {	// if the coord told us to expect a group of answers
    	strcpy(msgbuf, "-");
		// printf("console: waiting batch of answers\n");
		int stop = 0;
		while(!stop) {					// until we are not done taking the group of answers
			/* open jms_out fifo */
			while((jms_out_fd = open(jms_out, O_RDONLY)) < 0);
			/* stand-by for coordinator's response */
			if((read(jms_out_fd, msgbuf, MSGSIZE)) < 0) {
				perror("console: jms_out_fd read problem\n");
				exit(EXIT_FAILURE);
			}
			if(!strcmp(msgbuf, "batch-over"))	// the coord tells us that the group of answers is over
				stop = 1;
			else {
				printf("%s\n", msgbuf);
    			strcpy(msgbuf, "-");
			}
		}
		// printf("console: batch of answers is over\n");
    	strcpy(msgbuf, "-");
    // the coord tells us the he is shutting down (we are going to do the same)
	} else if(!strcmp(msgbuf, "shutting-down")) {
		strcpy(msgbuf, "-");
		printf("waiting for shutdown\n");
		/* open jms_out fifo */
		while((jms_out_fd = open(jms_out, O_RDONLY)) < 0);
		/* stand-by for coordinator's shutdown response */
		if((read(jms_out_fd, msgbuf, MSGSIZE)) < 0) {
			perror("console: jms_out_fd read problem\n");
			exit(EXIT_FAILURE);
		}
		printf("%s\n", msgbuf);
    	strcpy(msgbuf, "-");
		
		shutdown = 1;	// so that main() function knows that we are going to shut down
	} else if(strcmp(msgbuf, "-") != 0) {
    	printf("%s\n", msgbuf);
    	strcpy(msgbuf, "-");
	}

	return shutdown;
}