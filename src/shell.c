#include "../include/shell.h"
#include "../include/signal_handlers.h"
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // For fork(), execve()
#include <sys/wait.h> // For waitpid()

extern char **environ;
msh_t *shell = NULL;
volatile pid_t foreground_pid = -1;  // Definition with initial value
volatile sig_atomic_t is_fg_running = 0;  // Definition with initial value


msh_t *alloc_shell(int max_jobs, int max_line, int max_history) {
    const int MAX_LINE = 1024;
    const int MAX_JOBS = 16;
    const int MAX_HISTORY = 10;

    // Allocate memory for the shell
    shell = malloc(sizeof(msh_t));
    if (shell == NULL) {
        return NULL;
    }

    shell->max_jobs = (max_jobs == 0) ? MAX_JOBS : max_jobs;
    shell->max_line = (max_line == 0) ? MAX_LINE : max_line;
    shell->max_history = (max_history == 0) ? MAX_HISTORY : max_history;

    shell->jobs = malloc(shell->max_jobs * sizeof(job_t));
    if (shell->jobs == NULL) {
        free(shell);
        shell = NULL; 
        return NULL;
    }

    // Initialize the jobs array to a known state (undefined)
    for (int i = 0; i < shell->max_jobs; i++) {
        shell->jobs[i].cmd_line = NULL;
        shell->jobs[i].state = UNDEFINED;
        shell->jobs[i].pid = 0;
        shell->jobs[i].jid = 0;
    }

    initialize_signal_handlers();

    return shell;

}




char *parse_tok(char *line, int *job_type) {
    static char *current = NULL;
    static char *next_start = NULL;

    if (line != NULL) {
        current = line;
        next_start = line;
    }

    if (!current || !*current || job_type == NULL) {
        if (job_type != NULL) *job_type = -1; // No more jobs.
        return NULL;
    }

    // Use next_start to preserve leading whitespace for each command.
    char *start = next_start;

    while (isspace((unsigned char)*current)) {
        current++;
    }

    // If only whitespace is left, return NULL.
    if (*current == '\0') {
        *job_type = -1;
        return NULL;
    }

    // Find the end of the command and determine the job type.
    char *end = current;
    *job_type = 1; // Default to foreground job.
    
    while (*end && *end != ';' && *end != '&') {
        end++;
    }
    if (*end == '&') {
        *job_type = 0; // Background job.
        
    }

    // Terminate the current command and prepare for the next.
    if (*end) {
        *end = '\0';
        next_start = end + 1;
        next_start = end;
    }

    current = next_start;
    while (isspace((unsigned char)*current)) {
        current++;
    }

    return start;
}





// For now, ignore is_builtin. We will come back to the is_builtin in a future assignment.
char **separate_args(char *line, int *argc, bool *is_builtin) {
    // Check if line is empty or contains only whitespace
    char *p = line;
    while (*p != '\0') {
        if (!isspace((unsigned char)*p)) {
            break;
        }
        p++;
    }

    // If line is empty or contains only whitespace, return NULL
    if (*p == '\0') {
        *argc = 0; // No arguments
        return NULL;
    }

    // Initial capacity for arguments array
    size_t args_capacity = 10;
    char **args = malloc(args_capacity * sizeof(char*));
    if (!args) return NULL; // Memory allocation failed

    *argc = 0; // Reset argument count
    char *token = strtok(line, " \t\n");
    while (token != NULL) {
        if (*argc >= args_capacity) {
            args_capacity *= 2; // Increase capacity
            args = realloc(args, args_capacity * sizeof(char*));
            if (!args) return NULL; // Memory allocation failed
        }
        args[*argc] = strdup(token); // Duplicate and store token
        (*argc)++; // Increment argument count
        token = strtok(NULL, " \t\n"); // Get next token
    }
    // Ensure the array is NULL-terminated
    if (*argc >= args_capacity) {
        args = realloc(args, (args_capacity + 1) * sizeof(char*));
    }
    args[*argc] = NULL; // NULL-terminate the array

    // is_builtin handling to be implemented later
    (void)is_builtin; // Ignore unused parameter warning for now

    return args;

}

/* I tried so many ways. The only one which sucessfully output a time satisfying 
the requirements of testcase 2 and testcase 3 is to check the foreground jobs and 
background jobs separately. I am checking the foreground jobs in the function evaluate()
and the background jobs in the function exit_shell. However, I am confused about why
the testcase 3 could have different output when I run the it with exactly the same code.
Generally, the testcase 3 would pass. However, it also failed a few times. And the time could be
different. 

And it was mentioned in Ed that we can ignore the testcase 21.
*/

/*
void waitfg(pid_t pid) {
    while(is_fg_running) {
        sleep(1);
    }
}
*/

void waitfg(pid_t pid) {
    while(is_fg_running) {
        sleep(1);
    }
}





int evaluate(char *line) {
    sigset_t sigchld_mask, prev_mask;
    sigemptyset(&sigchld_mask);
    sigaddset(&sigchld_mask, SIGCHLD);

    

    char *job;
    int job_type;
    //int argc;
    //char **argv;
    int exit_flag = 0;  // Flag to indicate if 'exit' command was encountered

    // Parse jobs from the command line
    job = parse_tok(line, &job_type);
    while (job != NULL) {
        int argc;
        char **argv = separate_args(job, &argc, NULL);  // Break the job into arguments

        if (argv != NULL) {
            // Check for the "exit" command
            if (strcmp(argv[0], "exit") == 0) {
                exit_flag = 1;  // Set exit flag and break from the loop
                break;
            }
            else {
                // Block SIGCHLD signals to prevent race conditions during child process setup
                if (sigprocmask(SIG_BLOCK, &sigchld_mask, &prev_mask) < 0) {
                    perror("sigprocmask - block");
                    return -1;
                }
                pid_t pid = fork();  // Fork a new process
                if (pid == 0) {  // Child process
                    // Set the child process to have its own process group
                    setpgid(0, 0);
                    // Restore the original signal mask in the child
                    sigprocmask(SIG_SETMASK, &prev_mask, NULL);
                    // Execute the command
                    //execvp(argv[0], argv);
                    execve(argv[0], argv, NULL);
                    // If execvp returns, it's an error
                    //perror("execvp");
                    exit(EXIT_FAILURE);
                } else if (pid > 0) { 
                     // Parent process
                    setpgid(pid, pid);
                    if (job_type == FOREGROUND) {
                        foreground_pid = pid;  // Track the foreground process
                        is_fg_running  = 1;
                        add_job(shell->jobs, shell->max_jobs, pid, FOREGROUND, job);
                        // Unblock SIGCHLD before waiting
                        sigprocmask(SIG_SETMASK, &prev_mask, NULL);
                        int status;
                        // Wait for the foreground process to complete
                        waitfg(pid);
                        is_fg_running = 0;
                        foreground_pid = 0;  // Reset foreground PID
                        // Re-block SIGCHLD after handling the foreground process
                    } else {
                        // For background jobs, add them to the job list without waiting
                        add_job(shell->jobs, shell->max_jobs, pid, BACKGROUND, job);
                        sigprocmask(SIG_SETMASK, &prev_mask, NULL);

                    }
                }
                else {
                    perror("fork");  // Fork failed
                }
            }

            // Free allocated memory for argv and job
            for (int i = 0; i < argc; i++) {
                free(argv[i]);
            }
            free(argv);
        }

        free(job);  // Free the allocated memory for the job
        // Get the next job
        job = parse_tok(NULL, &job_type);
    }

    // If "exit" command was encountered
    
    if (exit_flag) {
        // Wait for all background jobs to finish
        for (int i = 0; i < shell->max_jobs; i++) {
            if (shell->jobs[i].pid != 0) {
                int status;
                waitpid(shell->jobs[i].pid, &status, 0);
            }
        }
        // Perform any necessary cleanup
        exit_shell();
        // Exit the shell immediately
        exit(0);
    }


    return 1;  // Indicate successful command evaluation
}




void exit_shell() {
    if (shell != NULL) {
        bool all_done = false;
        
        // Keep looping until all background jobs are completed
        while (!all_done) {
            // Assume all jobs are done until proven otherwise
            all_done = true;

            for (int i = 0; i < shell->max_jobs; i++) {
                if (shell->jobs[i].cmd_line != NULL && shell->jobs[i].state == BACKGROUND) {
                    int child_status;
                    // Use WNOHANG to check the status of background jobs without blocking
                    pid_t result = waitpid(shell->jobs[i].pid, &child_status, WNOHANG);

                    if (result == 0) {
                        // If a background job is still running, set the flag to false
                        all_done = false;
                    }
                    else if (result > 0) {
                        // If a background job has finished, clean up
                        delete_job(shell->jobs, shell->max_jobs, result);
                    }
                }
            }
        }

        // Clean up all jobs and free the shell structure
        free_jobs(shell->jobs, shell->max_jobs);
        free(shell);
        shell = NULL; // Reset the global shell pointer to avoid dangling pointer
    }
}



