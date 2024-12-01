#ifndef _JOB_H_
#define _JOB_H_

#include <sys/types.h>
#include <stdbool.h> 

typedef enum job_state{FOREGROUND, BACKGROUND, SUSPENDED, UNDEFINED} job_state_t;

// Represents a job in a shell.
typedef struct job {
    char *cmd_line;     // The command line for this specific job.
    job_state_t state;  // The current state for this job
    pid_t pid;          // The process id for this job
    int jid;            // The job number for this job
}job_t;

void update_job_status(job_t *jobs, int max_jobs, pid_t pid, job_state_t new_state);

/*
* add_job - adds a new job to the array
*
* *jobs - job to add
*
* *max_jobs - serve as a reminder if the number of jobs has been exceeded
*
* pid = pid
*
* state - FOREGROUND / BACKGROUND
*
* *cmd_line = command line
*
* Returns: True if added, False if not
*/
bool add_job(job_t *jobs, int max_jobs, pid_t pid, job_state_t state, const char *cmd_line);




/*
* delete_job - delete a job in array
*
* *jobs - job to delete
*
* *max_jobs - serve as a reminder if the number of jobs has been exceeded
*
* pid = pid
*
* Return: True if delete, False if not
*/
bool delete_job(job_t *jobs, int max_jobs, pid_t pid);




/*
* free_jobs - clear up
*
* *jobs - job to add
*
* *max_jobs - serve as a reminder whether the number of jobs has been exceeded
*
* Returns: none (void)
*/
void free_jobs(job_t *jobs, int max_jobs);

#endif