#include "console_header.h"

/* Global Variables */
char *jms_in, *jms_out, *msgbuf;

int main(int argc, char* argv[]) {
    printf("Welcome!\n");
	/* SET UP BASED ON COMMAND LINE'S ARGUMENTS */
    if (argc < 5) {     // check number of arguments, the operation file argument is optional
        fprintf(stderr, "ERROR! Insufficient number of arguments!\n");
        exit(EXIT_FAILURE);
    }
    // initialize variables
    int wflag = 0, rflag = 0, oflag = 0, i, ret = 0, shutdown = 0;
    char *operations_file = NULL;
    jms_in = NULL;
    jms_out = NULL;
    msgbuf = NULL;

    msgbuf = malloc(MSGSIZE);
    // memset to avoid memory-leaks because "Syscall param write(buf) points to uninitialised byte(s)"
    memset(msgbuf, '\0', MSGSIZE);

    for(i = 1; i < argc-1; i += 2) {    // match variables with parameters' values
    	if(!wflag && !strcmp(argv[i], "-w")) {
    		wflag = 1;
    		jms_in = malloc(strlen(argv[i+1])+1);
    		strcpy(jms_in, argv[i+1]);
    	} else if(!rflag && !strcmp(argv[i], "-r")) {
    		rflag = 1;
    		jms_out = malloc(strlen(argv[i+1])+1);
    		strcpy(jms_out, argv[i+1]);
    	} else if(!oflag && !strcmp(argv[i], "-o")) {
    		oflag = 1;
    		operations_file = (char*)malloc(strlen(argv[i+1])+1);
    		strcpy(operations_file, argv[i+1]);
    	} else {
            fprintf(stderr, "ERROR! Bad argument formation!\n");
            exit(EXIT_FAILURE);
        }
    }

    if (!wflag || !rflag ) {   // test if all the neccessary flags have been included
        fprintf(stderr, "ERROR! Arguments missing!\n");
        exit(EXIT_FAILURE);
    }

    // printf("jms_in = %s, jms_out = %s\n", jms_in, jms_out);
    // if(oflag) printf("operations_file = %s\n", operations_file);
    /* INITIATE COMMUNICATION WITH JMS_COORDINATOR */
    handshake_with_coord();

    /* TAKE OPERATIONS FROM FILE */
    if(oflag)
        if((ret = get_operations_from_file(operations_file)) < 0)
            fprintf(stderr, "ERROR! Bad input from operations file!\n");

    shutdown = ret;     // the file might have told us to shut-down
    /* TAKE OPERATIONS FROM TERMINAL */
    char* operation = NULL;   // forces getline to allocate with malloc
    size_t len = 0;     // ignored when line=NULL
    ssize_t read;

    if(!shutdown)
        printf("\nEnter an operation below:\n");      // if we are still going
    while(!shutdown && (read = getline(&operation, &len, stdin)) != -1) {
        printf("processing...\n");
        send_coord(operation);
        shutdown = listen_coord();
        printf("\n");       // for aesthetic reasons
    }
    free(operation);    // free memory allocated by getline
    printf("Console terminating...\nGoodbye!\n");

    // free dynamically-allocated memory
    free(jms_in);
    free(jms_out);
    free(msgbuf);
    free(operations_file);

	return 0;
}