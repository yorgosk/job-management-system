# Project: Project2 - Job Management System (JMS)

Project Developer: Georgios Kamaras - sdi1400058

Course: K24 Systems Programming, Spring 2017

Date: 26/04/2017

Development Platform:
*	GNU/Linux Ubuntu 16.04
*	gcc version 5.4.0 20160609 (Ubuntu 5.4.0-6ubuntu1~16.04.4)

## Included Files
*	makefile
*	console_header.h (my console's main library-file, links all of the console program's modules
						together)
*	console_main.c (my console program's main function)
*	console_functions.h console_functions.c (various functions that I use throughout my console's
												program)
*	coord_header.h (my coordinator's main library-file, links all of the coordinator program's modules
						together)
*	coord_main.c (my coordinator program's main function)
*	coord_functions.h coord_functions.c (various functions that I use throughout my coordinator's
											program)
*	coord_commands.h coord_commands.c (the implementation of the commands that my JMS can accept, from
										the coordinator's perspective)
*	pool_header.h (my pool's main library-file, links all of the pool program's modules together)
*	pool_main.c (my pool program's main function)
*	pool_functions.h pool_functions.c (various functions that I use throughout my pool's program)
*	pool_commands.h pool_commands.c (the implementation of the commands that my JMS can accept, from
										the pool's perspective)
*	jms_script.sh (a bash script for basic statistics and manipulation of my JMS's concluded and ongoing
					processes and their results)
*	(and README)

## Compilation
Use ```make``` command

## Cleaning
Use ```make clean``` command

## Usage
*	(1) First, use ```./jms_coord <desired-flags>``` command to run the JMS's coordinator
	e.g. ```./jms_coord -l mypath -n 3 -w jms_out -r jms_in```,
*	(2) then, use ```./jms_console <desire-flag>``` command to run the JMS's console
	e.g. ```./jms_console -r jms_out -w jms_in -o inputs/myinput2.txt```.
*	At any time, use ```./jms_script``` command to interact with the results of the instructions passed to JMS
	e.g. ```./jms_script.sh -l path -c purge``` (making the file executable using chmod may be needed first)

## Technical Details
*	For my implementation I followed all of the exercise's specifications both in paper and on Piazza.
	My goal was to develop a Job Management System (JMS) able to manage a large amount of jobs and grant
	access to all the essential information about their execution at any time, all that in an efficient
	way in terms of memory and time consumption. To achive this goal, I implemented all the functionality
	described in the exercise's specifications.
*	My JMS, basically, consists of two programs. The Coordinator (jms_coord) and the Console (jms_console).
	The Coordinator is the core of the whole application, so I expect that it is the first of the two
	programs that the user is going to run. The Console is a minimalistic user's interface with the
	Coordinator. The user types in the Console whatever he wants the JMS to do for him and it is through the
	Console that he takes whatever feedback the JMS has to give him in his command. As recommended in piazza,
	this happens in the following sequence: the user enters something (input1) in the Console, the Coordinator processes it, and returns some message concerning the user's input1, which the user can see through his console. Then, the user can enter something again (input2).
	My application is also accompanied by a bash script which the user can run at any time, to take certain
	information about the JMS's output concerning the various jobs that he may have submitted.
*	When it comes to JMS coordinator (jms_coord) and JMS console (jms_console) implementation, as tasked,
	I implemented the "submit <job>", "status <JobID>", "status-all [time-duration]", "show-active",
	"show-pools", "show-finished", "suspend <JobID>", "resume <JobID>" and "shutdown" commands-operations.
	I tried to follow the project's specifications as closely as possible, taking the liberty to do what
	I considered to be the best for the overall functionality of the application, wherever they seemed vague
	or incomplete.
	NOTE: Most of the above commands-operations ("status <JobID>", "status-all [time-duration]", "show-active",
		"show-pools", "show-finished", "suspend <JobID>", "resume <JobID>") cannot work if there have been no
		jobs submitted to the JMS. In such a case, by design, the JMS is going to treat them as "bad
		operations", meaning "unacceptable" ones.
	In general, the information "flows" as following; the user enters a commands in the console. The console
	forwards this command to the coordinator. The coordinator processes the command and then assigns it in
	the most proper way to his children, which are the pools. The pools process the command and start
	formulating an answer-output for the user and sending it to the coordinator. The coordinator takes the
	output and forwards it to the console. Whenever the answer-output to the user's command has ended the
	user can enter his next command. All the above is implemented through a protocol that I designed and
	implemented, which defines how each user's request is taken, processed and answered in my JMS. For more
	details regarding my protocol, see my "Appendix I - Protocols" section.
*	When it comes to the JMS bash script (jms_script.sh), as requested by the project's specifications, it
	implements 3 basic commands. The user provides a <path> (the path where he requested the jms_coord to
	store the outputs from the various jobs execution) and a commands. This commands can be; "list", which I
	have implemented as an execution of the "ls -l $path" command, or "size [n]", which I have implemented
	using the "du -a --max-depth=1 $path | sort -n [| tail -n$n]" command, or "purge", which I have
	implemented as "rm -rf $path".
	WARNING: Obviously, the user has to be very careful with the <path> in which he applies the "purge"
			command, as it's results are irreversible.

## Appendix I - Protocols
*	Console-Coordinator Communication Protocol (jms.consocoord.pipes)
	1) Hand-shake: when the coordinator begins running he enters a hand-shake process in which he waits
	for a console to establish a "good" (defined roles and message priority) connection with him. Likewise,
	when a console begins running, it's first order of business is to establish a "good" connection with
	the JMS's coordinator.
	2) Responding to a command: The user enters his command to the console and the console forwards the
	command to the coordinator. The coordinator identifies the command and forwards it to his pools. It
	the waits for the pools to respond/start responding to the command, in order to forward the pools'
	responses to the console. From the coordinator's side, the response waiting-forwarding system is
	tailored to fit each command's expected needs. In overall, there are to kinds of responses to a command;
	a "single-phrase-response" (commands: "submit <job>", "status <JobID>", "suspend <JobID>", "resume
	<JobID>" and "shutdown") or a "batch-response" (commands: "status-all [time-duration]", "show-active",
	"show-pools" and "show-finished"). When it comes to handling a "batch-response", the coordinator
	messages the console to wait a "batch-response" and the console enters a loop-mode where it prints
	whatever it reads from the coordinator. When the answer is over, the coordinator messages the console
	regarding the end of the "batch-response", and the console can ask the user to enter his next command.

*	Coordinator-Pool Communication Protocol (jms.coordpool.pipes)
	1) Hand-shake: when the coordinator creates a new child-pool he enters a hand-shake process in which
	he waits for a pool (the just-created-pool) to establish a "good" (defined roles and message priority)
	connection with him. Likewise, when a pool begins running, it's first order of business is to establish
	a "good" connection with the JMS's coordinator.
	2) Responding to a command: In extension to what described in the same section of the jms.consocoord.pipes,
	there are to kinds of responses to a command; a "single-phrase-response" or a "batch-response". The
	successful communication of a "batch-response" between the coordinator and the pool is achieved in the
	same way that it is achieved between the console and the pool. The pool messages the coordinator regarding
	the beginning of this response, so that it can begin to forward answers to the console, and when the
	response is over, the pool messages the coordinator regarding the end of this response. There are commands
	that in order to answer them, the coordinator needs to gather details/information/resources from many
	different pools. In such commands, the coordinator enters a question-response communication with each of
	his active pools sequentially. Because of the overall simplicity of the information flow mechanism with
	which I implement these functionality, and the speed that it guarantees, I didn't considered necessary to
	do something more complicated.

## Appendix II - External Sources
*	For the creation of a date-time string for the pools' children-jobs results, I consulted the links:
	http://stackoverflow.com/questions/10917491/building-a-date-string-in-c and
	https://linux.die.net/man/3/strftime
*	For the redirection of the pools' children-jobs output in files, I consulted the link:
	http://stackoverflow.com/questions/2605130/redirecting-exec-output-to-a-buffer-or-file


## Contact - feedback
Georgios Kamaras: sdi1400058@di.uoa.gr
