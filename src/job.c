#include "../include/job.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 

bool add_job(job_t *jobs, int max_jobs, pid_t pid, job_state_t state, const char *cmd_line) {
    for (int i = 0; i < max_jobs; i++) {
        if (jobs[i].pid == 0) {
            jobs[i].pid = pid;
            jobs[i].state = state;
            jobs[i].cmd_line = strdup(cmd_line);
            if (!jobs[i].cmd_line) {
                // Handle memory allocation failure
                return false;
            }
            return true;  // Job added successfully
        }
    }
    return false;  // No empty slot found, job not added
}

bool delete_job(job_t *jobs, int max_jobs, pid_t pid) {
    for (int i = 0; i < max_jobs; i++) {
        if (jobs[i].pid == pid) {
            free(jobs[i].cmd_line);
            jobs[i].pid = 0;
            return true;
        }
    }
    return false;
}



void free_jobs(job_t *jobs, int max_jobs) {
    for (int i = 0; i < max_jobs; i++) {
        if (jobs[i].pid != 0) {
            free(jobs[i].cmd_line);  // Free the command line for each job
        }
    }
}



void update_job_status(job_t *jobs, int max_jobs, pid_t pid, job_state_t new_state) {
    for (int i = 0; i < max_jobs; i++) {
        if (jobs[i].pid == pid) {
            jobs[i].state = new_state;  // Update the job's state
            break;  // Exit the loop once the job is found and updated
        }
    }
}