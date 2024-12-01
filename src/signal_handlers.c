#include "../include/signal_handlers.h"
#include "../include/shell.h"
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>  // For waitpid and associated macros
#include <sys/types.h> // For pid_t

//extern msh_t *shell;

//volatile pid_t foreground_pid = 0;  // Definition with initial value
//volatile sig_atomic_t is_fg_running = 0;  // Definition with initial value

void sigchld_handler(int sig) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
        if(pid == foreground_pid) {  // Use '==' for comparison
            if (WIFEXITED(status) || WIFSIGNALED(status)) {
                is_fg_running = 0;
                foreground_pid = 0;  // Consider setting this to a non-valid PID instead of 0, such as -1
                delete_job(shell->jobs, shell->max_jobs, pid);
            }
            // The handling for WIFSTOPPED and WIFCONTINUED should be outside this if block
        } 

        if (WIFSTOPPED(status)) {
            update_job_status(shell->jobs, shell->max_jobs, pid, SUSPENDED);
        } else if (WIFCONTINUED(status)) {
            update_job_status(shell->jobs, shell->max_jobs, pid, BACKGROUND);
        }
    }

    if (pid < 0 && errno != ECHILD) {
        perror("waitpid error");
    }
}



void sigint_handler(int sig) {
    if (foreground_pid > 0) {
        kill(-foreground_pid, SIGINT);
        foreground_pid = -1;
    }
}


void sigtstp_handler(int sig) {
    if (foreground_pid > 0) {
        kill(-foreground_pid, SIGTSTP);  // Send SIGTSTP to the foreground process group
    }
}


typedef void handler_t(int);
handler_t *setup_handler(int signum, handler_t *handler)
{
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0) {
        perror("Signal error");
        exit(1);
    }
    return (old_action.sa_handler);
}



void initialize_signal_handlers() {

    // sigint handler: Catches SIGINT (ctrl-c) signals.
    setup_handler(SIGINT,  sigint_handler);   /* ctrl-c */
    // sigtstp handler: Catches SIGTSTP (ctrl-z) signals.
    setup_handler(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    // sigchld handler: Catches SIGCHILD signals.
    setup_handler(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */
}