#ifndef _SIGNAL_HANDLERS_H_
#define _SIGNAL_HANDLERS_H_

//#include <sys/types.h>
//#include <stdbool.h> 

void initialize_signal_handlers();

/*

/*
* sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
*     a child job terminates (becomes a zombie), or stops because it
*     received a SIGSTOP or SIGTSTP signal. The handler reaps all
*     available zombie children, but doesn't wait for any other
*     currently running children to terminate.
* Citation: Bryant and O’Hallaron, Computer Systems: A Programmer’s Perspective, Third Edition

void sigchld_handler(int sig);
*/


/*
* sigint_handler - The kernel sends a SIGINT to the shell whenver the
*    user types ctrl-c at the keyboard.  Catch it and send it along
*    to the foreground job.
* Citation: Bryant and O’Hallaron, Computer Systems: A Programmer’s Perspective, Third Edition

void sigint_handler(int sig);

*/

/*
* sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
*     the user types ctrl-z at the keyboard. Catch it and suspend the
*     foreground job by sending it a SIGTSTP.
* Citation: Bryant and O’Hallaron, Computer Systems: A Programmer’s Perspective, Third Edition

void sigtstp_handler(int sig);

*/

/*
* setup_handler - wrapper for the sigaction function
*
* Citation: Bryant and O’Hallaron, Computer Systems: A Programmer’s Perspective, Third Edition

typedef void handler_t(int);
handler_t *setup_handler(int signum, handler_t *handler);
*/

#endif
