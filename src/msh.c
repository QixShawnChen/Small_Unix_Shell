#define _GNU_SOURCE
#include "../include/shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int main(int argc, char *argv[]) {
    int opt;
    int max_jobs = 0, max_line = 0, max_history = 0;
    int flag_error = 0;  // Flag to indicate if an error occurred

    opterr = 0;  // Disable getopt's default error messages

    while ((opt = getopt(argc, argv, "s:j:l:")) != -1) {
        char *endptr;  // Pointer to the end of the parsed number
        long val;      // To store the converted value

        switch (opt) {
            case 's':
                val = strtol(optarg, &endptr, 10);
                if (*endptr != '\0' || val <= 0) {
                    flag_error = 1;
                } else {
                    max_history = (int)val;
                }
                break;
            case 'j':
                val = strtol(optarg, &endptr, 10);
                if (*endptr != '\0' || val <= 0) {
                    flag_error = 1;
                } else {
                    max_jobs = (int)val;
                }
                break;
            case 'l':
                val = strtol(optarg, &endptr, 10);
                if (*endptr != '\0' || val <= 0) {
                    flag_error = 1;
                } else {
                    max_line = (int)val;
                }
                break;
            default:
                // Unrecognized option encountered
                flag_error = 1;
                break;
        }
    }

    // Check if there are non-option arguments after all options have been processed
    if (optind < argc || flag_error) {
        fprintf(stdout, "usage: msh [-s NUMBER] [-j NUMBER] [-l NUMBER]\n");
        return 1;
    }
    

    shell = alloc_shell(max_jobs, max_line, max_history);
    if (!shell) {
        fprintf(stdout, "Failed to allocate shell\n");
        return 1;
    }

    // Initialize signal handlers
    initialize_signal_handlers();

    char *line = NULL;
    size_t len = 0; // Change type from long int to size_t
    ssize_t nRead;  // Change type from long int to ssize_t

    /*
    while (1) {
        printf("msh> ");
        nRead = getline(&line, &len, stdin);
        if (nRead == -1) break; // Exit loop on EOF

        // Remove the newline character at the end of the line if present
        if (line[nRead - 1] == '\n') {
            line[nRead - 1] = '\0';
        }

        evaluate(line);

        free(line); // Free the allocated line
        line = NULL; // Reset line for the next getline call
    }
    */

    while (1) {
        printf("msh> ");
        nRead = getline(&line, &len, stdin);
        if (nRead == -1) break; // Exit loop on EOF

        // Remove the newline character at the end of the line if present
        if (line[nRead - 1] == '\n') {
            line[nRead - 1] = '\0';
        }

        if (evaluate(line) == -1) {
            fprintf(stderr, "Error executing command\n");
        }

        free(line); // Free the allocated line
        line = NULL; // Reset line for the next getline call
    }

    // Perform cleanup and exit
    exit_shell();
    return 0;
}