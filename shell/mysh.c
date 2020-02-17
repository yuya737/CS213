#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// This is the maximum number of arguments your shell should handle for one command
#define MAX_ARGS 128

void run_command(char* arguments[], bool background){
    int rc = fork();
    if (rc < 0) {
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0) {
        // child (new process)
        execvp(arguments[0], arguments);
    } else {
        if (!background){
            int status, pid;
            int wc = waitpid(rc, &status, 0);
            int exit_status = WEXITSTATUS(status);
            printf("Child process %d exited with status %d\n", wc, exit_status);
            while ((pid=waitpid(-1,&status,WNOHANG)) > 0){
                int exit_status = WEXITSTATUS(status);
                printf("Child process %d exited with status %d\n", pid, exit_status);
            }
        } else {
            int status, pid;
            while ((pid=waitpid(-1,&status,WNOHANG)) > 0){
                int exit_status = WEXITSTATUS(status);
                printf("Child process %d exited with status %d\n", pid, exit_status);
            }

        }

    }
}


int main(int argc, char** argv) {
    // If there was a command line option passed in, use that file instead of stdin
    if(argc == 2) {
        // Try to open the file
        int new_input = open(argv[1], O_RDONLY);
        if(new_input == -1) {
            fprintf(stderr, "Failed to open input file %s\n", argv[1]);
            exit(1);
        }

        // Now swap this file in and use it as stdin
        if(dup2(new_input, STDIN_FILENO) == -1) {
            fprintf(stderr, "Failed to set new file as input\n");
            exit(2);
        }
    }

    char* line = NULL;    // Pointer that will hold the line we read in
    size_t line_size = 0; // The number of bytes available in line
    char* arguments[MAX_ARGS];

    // Loop forever
    while(true) {
        // Print the shell prompt
        printf("$ ");

        // Get a line of stdin, storing the string pointer in line
        if(getline(&line, &line_size, stdin) == -1) {
            if(errno == EINVAL) {
                perror("Unable to read command line");
                exit(2);
            } else {
                // Must have been end of file (ctrl+D)
                printf("\nShutting down...\n");

                // Exit the infinite loop
                break;
            }
        }

        int exit_flag = 0;

        char* current_position = line;

        while (true) {
            char* delim_position = strpbrk(current_position, ";\n");
            if(delim_position == NULL) {
                // There were no more delimeters.
                //printf("The last part is %s.\n", current_position);
                break;

            } else {
                // There was a delimeter. First, save it.
                // char delim = *delim_position;

                // Overwrite the delimeter with a null terminator so we can print just this fragment
                *delim_position = '\0';

                // printf("The fragment %s was found, followed by '%c'.\n", current_position, delim);

                while (current_position[0] == ' ') {
                    //printf("current position: %s\n", current_position);
                    current_position++;
                }
                // Execute a command

                printf("Received command: %s\n", current_position);



                int count = 0;
                char* found;
                while((found = strsep(&current_position, " "))!= NULL){
                    if (!strcmp(found, "&")) {
                        if (strcmp(arguments[0],"cd") == 0){
                            chdir(arguments[1]);
                        } else if ((strcmp(arguments[0],"") == 0)){
                            break;
                        } else if ((strcmp(arguments[0],"exit") == 0)){
                            //break;
                            //exit_flag = 1;
                            goto will_break;
                        } else {
                            arguments[count]=NULL;
                            run_command(arguments, true);
                            count = 0;
                            continue;
                        }
                    } else {
                        arguments[count] = found;
                        count++;
                        //printf("%d\n",count);
                    }
                }
                if (count == 0) break;
                arguments[count]=NULL;

                // for(int i = 0; i <count; i++){
                //   printf("%s\n", arguments[i]);
                // }

                if (strcmp(arguments[0],"cd") == 0){
                    chdir(arguments[1]);
                } else if ((strcmp(arguments[0],"") == 0)){
                    break;
                } else if ((strcmp(arguments[0],"exit") == 0)){
                    exit_flag = 1;
                    goto will_break;
                } else {
                    run_command(arguments, false);
                }
            }
            // Move our current position in the string to one character past the delimeter
            current_position = delim_position + 1;
        }

will_break:
        if(exit_flag){
            break;
        }

        // if (exit_flag){
        //   break;
        // }
    }

    // If we read in at least one line, free this space
    if(line!= NULL) {
        free(line);
    }
    // TODO: Execute the command instead of printing it below

    return 0;
}
