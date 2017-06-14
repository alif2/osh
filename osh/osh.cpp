#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "command.h"
#include "osh_const.h"
#include "parse.h"

using namespace osh;

int main(int argc, char** argv) {
    // Command structure
    Command *head = NULL;

    int i = getopt(argc, argv, "t");
    switch(i) {
        case 't':
            break;
        default:
            printf("osh> ");
            break;
    }
    
    // Bounded input loop to avoid infinite forks
    for(int i = 0; i < 25; i++) {
        int cmdcode = GetCommandChain(&head);

        if(cmdcode != status_success) {
            fprintf(stderr, "Error reading input\n");
            exit(1);
        }

        // Ignore blank lines
        if(head->file == NULL || strlen(head->file) < 1) {
            continue;
        }

        // Exit command signals end of input
        if(strcmp(head->file, "exit") == 0) {
            exit(0);
        }

        // Bounded parse loop to avoid infinite loops
        int j = 0;
        int fd[2];
        while(head != NULL && j < 25) {
            int pipe_in = 0;

            if(head->symbolType == RedirectOut ||
               head->symbolType == RedirectIn || 
               head->symbolType == RedirectOutAppend || 
               head->symbolType == Pipe) {
                if(head->next == NULL || head->next->file == NULL) {
                    printf("Invalid NULL command\n");
                    break;
                }

                if((head->symbolType == RedirectOut || head->symbolType == RedirectOutAppend) && 
                    head->next->symbolType == Pipe) {
                    printf("Ambiguous output redirect.\n");
                    break;
                }

                if(head->symbolType == Pipe && head->next->symbolType == RedirectIn) {
                    printf("Ambiguous input redirect.\n");
                    break;
                }
            }

            if(head->prev != NULL && head->prev->symbolType != (long int) NULL) {
                // Current process executes only on success of previous one
                if(head->prev->symbolType == ExecuteOnSuccess) {
                    if(head->prev->status != 0) {
                        head->status = head->prev->status;
                        break;
                    }
                }

                // Current process executes only on failure of previous one
                else if(head->prev->symbolType == ExecuteOnFailure) {
                    if(head->prev->status == 0) {
                        head->status = head->prev->status;
                        break;
                    }
                }

                // Save pipe input file handle
                else if(head->prev->symbolType == Pipe) {
                    pipe_in = fd[0];
                }
            }

            if(head->symbolType == Pipe) {
                if(pipe(fd) < 0) {
                    fprintf(stderr, "Pipe failed\n");
                    exit(1);
                }
            }

            pid_t cpid = fork();
            if(cpid < 0) {
                fprintf(stderr, "Fork Failed\n");
                exit(1);
            }

            // Child process creates file handles and executes requested process
            else if(cpid == 0) {
                // 2+ for avoiding stdin/out
                if(pipe_in > 1) {
                    if(dup2(pipe_in, STDIN_FILENO) < 0) exit(1);
                    close(pipe_in);
                }

                if(head->symbolType != Null) {
                    char* redir = head->next->file;

                    if(head->symbolType == RedirectIn) {
                        head->inFileHandle = open(redir, O_RDONLY);
                        if(dup2(head->inFileHandle, STDIN_FILENO) < 0) exit(1);
                        close(head->inFileHandle);

                        if(head->next != NULL && head->next->symbolType == RedirectOut) {
                            head->next->outFileHandle = open(head->next->next->file, 
                                O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
                                
                            if(dup2(head->next->outFileHandle, STDOUT_FILENO) < 0) exit(1);
                            close(head->next->outFileHandle);
                        }

                        if(head->next != NULL && head->next->symbolType == RedirectOutAppend) {
                            head->next->outFileHandle = open(head->next->next->file,
                                O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
                            
                            if(dup2(head->next->outFileHandle, STDOUT_FILENO) < 0) exit(1);
                            close(head->next->outFileHandle);
                        }
                    }

                    else if(head->symbolType == RedirectOut) {
                        head->outFileHandle = open(redir,
                        O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

                        if(dup2(head->outFileHandle, STDOUT_FILENO) < 0) exit(1);
                        close(head->outFileHandle);
                    }

                    else if(head->symbolType == RedirectOutAppend) {
                        head->outFileHandle = open(redir,
                            O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

                        if(dup2(head->outFileHandle, STDOUT_FILENO) < 0) exit(1);
                        close(head->outFileHandle);
                    }

                    else if(head->symbolType == Pipe) {
                        if(dup2(fd[1], STDOUT_FILENO) < 0) exit(1);
                        close(fd[1]);
                    }
                }

                execvp(head->file, head->arglist);
                
                // Successful execution will not return
                fprintf(stderr, "Exec Failed\n");
                exit(1);
            }

            // Parent process waits for child to complete
            else {
                int status;
                wait(&status);
                head->status = status;

                if(fd[1] > 1) close(fd[1]);
            }

            // Advance past filename for redirect tokens to avoid execution
            if(head->symbolType == RedirectIn) {
                if(head->next->symbolType == RedirectOut || head->next->symbolType == RedirectOutAppend) {
                    head = head->next;
                }

                head = head->next;
            }

            else if(head->symbolType == RedirectOut || head->symbolType == RedirectOutAppend) {
                head = head->next;
            }

            head = head->next;
            j++;
        }
    }

    DeleteCommandChain(head);
    return 0;
}