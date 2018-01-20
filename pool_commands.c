#include "pool_commands.h"

void submit(char* job) {
	num_of_local_jobs++;	// one more local (in this pool) job
	num_of_total_jobs++;	// one more job in total

	int argnum = 0, j, pid;
	char *tok, *opname = NULL, **oparg = NULL;

	tok = strtok(job, " \n");	// take the name of the op
	opname = malloc(strlen(tok)+1);
	strcpy(opname, tok);
	// printf("opname = %s\n", opname);

	// we form the arguments array that we are going to pass to execvp() function
	while(tok) {	// while there are arguments left
		argnum++;
		oparg = realloc(oparg, argnum*sizeof(char*));
		oparg[argnum-1] = malloc(strlen(tok)+1);
		strcpy(oparg[argnum-1], tok);
		tok = strtok(NULL, " \n");
		// printf("arg = %s, tok = %s\n", oparg[argnum-1], tok);
	}
	argnum++;
	oparg = realloc(oparg, argnum*sizeof(char*));
	oparg[argnum-1] = NULL;
	// printf("arg = %s, tok = %s\n", oparg[argnum-1], tok);

	pid = fork();
	if(pid < 0) {
		perror("pool: fork problem\n");
		exit(EXIT_FAILURE);
	} else if(pid == 0) {
		/* REDIRECT STDOUT AND STDERR */
		char completepath[100], pidstr[10], jobidstr[10], datetimestr[20], file1name[20], file2name[20], file1path[120], file2path[120];
		int pid = getpid();
		/* sources: http://stackoverflow.com/questions/10917491/building-a-date-string-in-c
					https://linux.die.net/man/3/strftime */
		time_t now = time(NULL);
		struct tm *t = localtime(&now);

		// create the names of the files where stdout and sterr will go
		strcpy(completepath, path);
		strcat(completepath, "/sdi1400058_");
		sprintf(pidstr, "%d_", pid);
		strcat(completepath, pidstr);
		sprintf(jobidstr, "%d", num_of_total_jobs);
		strcat(completepath, jobidstr);
		strcat(completepath, "_");
		strftime(datetimestr, sizeof(datetimestr)-1, "%Y%m%d_%H%M%S", t);
		strcat(completepath, datetimestr);

		if((mkdir(completepath, S_IRWXU)) != 0) {
			perror("pool: mkdir problem\n");
			exit(EXIT_FAILURE);
		}

		strcpy(file1name, "stdout_");
		strcat(file1name, jobidstr);
		strcpy(file2name, "stderr_");
		strcat(file2name, jobidstr);

		strcpy(file1path, completepath);
		strcat(file1path, "/");
		strcat(file1path, file1name);
		strcpy(file2path, completepath);
		strcat(file2path, "/");
		strcat(file2path, file2name);

		/* source: http://stackoverflow.com/questions/2605130/redirecting-exec-output-to-a-buffer-or-file */
		int fd1 = open(file1path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
		int fd2 = open(file2path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

		dup2(fd1, STDOUT_FILENO);   // make stdout go to file with file-descriptor fd1
    	dup2(fd2, STDERR_FILENO);   // make stderr go to file with file-descriptor fd2

    	close(fd1);     // file desciptors are no longer needed - the dup'ed handles are sufficient
    	close(fd2);
		// printf("pool's child: %s, %s, %s\n", completepath, file1path, file2path);

		/* CHANGE CHILD'S CODE WITH OPERATION */
		execvp(opname, oparg);
	} else {
		job_arr = realloc(job_arr, num_of_local_jobs*sizeof(jobInfo));
		job_arr[num_of_local_jobs-1].jobid = num_of_total_jobs;
		job_arr[num_of_local_jobs-1].pid = pid;
		job_arr[num_of_local_jobs-1].start_time = time(NULL);	// job's start-time is the current system-time
		job_arr[num_of_local_jobs-1].running = 1;				// job is currently running
		num_of_running_jobs++;									// one more job running in pool
		
		/* formulate answer for the coordinator */
		char answer[40], num[10];
		strcpy(answer, "JobID: ");
		sprintf(num, "%d", job_arr[num_of_local_jobs-1].jobid);
		strcat(answer, num);
		strcat(answer, ", PID: ");
		sprintf(num, "%d", job_arr[num_of_local_jobs-1].pid);
		strcat(answer, num);
		// printf("answer = %s\n", answer);
		
		strcpy(msgbuf, answer);
	}

	// printf("opname ====== %s\n", opname);
	free(opname);
	for(j = 0; j < argnum; j++) {/*printf("oparg ======= %s\n", oparg[j]);*/ free(oparg[j]);}
	free(oparg);
	// printf("pool: sending coord: %s\n", msgbuf);
	send_coord();
}

void status(int jobid) {
	int i;

	for(i = 0; i < num_of_local_jobs; i++) {
		if(job_arr[i].jobid == jobid) {
			/* formulate answer for the coordinator */
			char response[50], num[10];
			strcpy(response, "JobID ");
			sprintf(num, "%d", jobid);
			strcat(response, num);
			
			if(job_arr[i].running == -1) strcat(response, " Status: Suspended");
			else if(job_arr[i].running == 0) strcat(response, " Status: Finished");
			else {
				int timedif = time(NULL) - job_arr[i].start_time;
				char tdif[10];
				sprintf(tdif, "%d", timedif);
				strcat(response, " Status: Active (submitted before ");
				strcat(response, tdif);
				strcat(response, " seconds)");
				// printf("pool: job found: %s\n", response);
			}
			
			strcpy(msgbuf, response);
			break;
		}
	}
	// printf("pool: sending coord: %s\n", msgbuf);
	send_coord();
}

void status_all(int time_duration) {
	int i;

	for(i = 0; i < num_of_local_jobs; i++) {
		if(time_duration == -1 || time(NULL) - job_arr[i].start_time <= time_duration) {
			/* formulate answer for the coordinator */
			char response[50], num[10];
			strcpy(response, "JobID ");
			sprintf(num, "%d", job_arr[i].jobid);
			strcat(response, num);
			
			if(job_arr[i].running == -1) strcat(response, " Status: Suspended");
			else if(job_arr[i].running == 0) strcat(response, " Status: Finished");
			else {
				int timedif = time(NULL) - job_arr[i].start_time;
				char tdif[10];
				sprintf(tdif, "%d", timedif);
				strcat(response, " Status: Active (submitted before ");
				strcat(response, tdif);
				strcat(response, " seconds)");
				// printf("pool: job found: %s\n", response);
			}
			
			strcpy(msgbuf, response);
			// printf("pool: sending coord: %s\n", msgbuf);
			send_coord();
		}
	}

	strcpy(msgbuf, "pool-batch-over");
	// printf("pool: sending coord: %s\n", msgbuf);
	send_coord();
}

void show_active() {
	int i;

	for(i = 0; i < num_of_local_jobs; i++) {
		if(job_arr[i].running == 1) {
			/* formulate answer for the coordinator */
			char response[50], num[10];
			strcpy(response, "JobID ");
			sprintf(num, "%d", job_arr[i].jobid);
			strcat(response, num);
			
			strcpy(msgbuf, response);
			// printf("pool: sending coord: %s\n", msgbuf);
			send_coord();
		}
	}

	strcpy(msgbuf, "pool-batch-over");
	// printf("pool: sending coord: %s\n", msgbuf);
	send_coord();
}

void show_pools() {
	char running[10];
	sprintf(running, "%d", num_of_running_jobs);

	strcpy(msgbuf, running);
	// printf("pool: sending coord: %s\n", msgbuf);
	send_coord();
}

void show_finished() {
	int i;
	for(i = 0; i < num_of_local_jobs; i++) {
		if(job_arr[i].running == 0) {
			/* formulate answer for the coordinator */
			char response[50], num[10];
			strcpy(response, "JobID ");
			sprintf(num, "%d", job_arr[i].jobid);
			strcat(response, num);
			
			strcpy(msgbuf, response);
			// printf("pool: sending coord: %s\n", msgbuf);
			send_coord();
		}
	}

	strcpy(msgbuf, "pool-batch-over");
	// printf("pool: sending coord: %s\n", msgbuf);
	send_coord();
}

void suspend(int jobid) {
	int i;

	for(i = 0; i < num_of_local_jobs; i++) {
		if(job_arr[i].jobid == jobid) {
			/* formulate answer for the coordinator */
			char response[50], num[10];
			if(job_arr[i].running == -1) {
				strcpy(response, "JobID ");
				sprintf(num, "%d", jobid);
				strcat(response, num);
				strcat(response, " has already been suspended");
			} else if(job_arr[i].running == 0) {
				strcpy(response, "JobID ");
				sprintf(num, "%d", jobid);
				strcat(response, num);
				strcat(response, " has already finished");
			} else {
				if(kill(job_arr[i].pid, SIGSTOP) < 0){	// stop (suspend) the currently running job-process
					perror("pool: kill with SIGSTOP problem\n");
					exit(EXIT_FAILURE);
				}
				job_arr[i].running = -1;		// note the job-process as suspended
				num_of_running_jobs--;			// one less job running
				strcpy(response, "Sent suspend signal to JobID ");
				sprintf(num, "%d", jobid);
				strcat(response, num);
			}
			
			strcpy(msgbuf, response);
			break;
		}
	}
	// printf("pool: sending coord: %s\n", msgbuf);
	send_coord();
}

void resume(int jobid) {
	int i;
	
	for(i = 0; i < num_of_local_jobs; i++) {
		if(job_arr[i].jobid == jobid) {
			/* formulate answer for the coordinator */
			char response[50], num[10];
			if(job_arr[i].running == 1) {
				strcpy(response, "JobID ");
				sprintf(num, "%d", jobid);
				strcat(response, num);
				strcat(response, " is already running");
			} else if(job_arr[i].running == 0) {
				strcpy(response, "JobID ");
				sprintf(num, "%d", jobid);
				strcat(response, num);
				strcat(response, " has already finished");
			} else {
				if(kill(job_arr[i].pid, SIGCONT) < 0){	// continue (resume) the currently suspended job-process
					perror("pool: kill with SIGCONT problem\n");
					exit(EXIT_FAILURE);
				}
				job_arr[i].running = 1;		// note the job-process as running
				num_of_running_jobs++;		// one more job running
				strcpy(response, "Sent resume signal to JobID ");
				sprintf(num, "%d", jobid);
				strcat(response, num);
			}
			
			strcpy(msgbuf, response);
			break;
		}
	}
	// printf("pool: sending coord: %s\n", msgbuf);
	send_coord();
}