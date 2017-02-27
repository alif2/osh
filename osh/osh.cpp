#include <cstdlib>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "command.h"
#include "parse.h"
#include "osh_const.h"

using namespace osh;

int main(int argc, char** argv) {
    // Command structure
    Command *head = NULL;
    
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
        
        // Bounded parse loop to avoid infinite loop
        int j = 0;
        int fd[2];
        while(head != NULL && j < 25) {    
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

            // Child process
            else if(cpid == 0) {
                if(head->prev != NULL && head->prev->symbolType == Pipe) {
                    if(dup2(fd[0], STDIN_FILENO) < 0) exit(1);
                    close(fd[0]);
                }

                if(head->symbolType != Null) {
                    char* redir = head->next->file;

                    if(head->symbolType == RedirectIn) {
                        head->inFileHandle = open(redir, O_RDONLY);
                        if(dup2(head->inFileHandle, STDIN_FILENO) < 0) exit(1);
                        close(head->inFileHandle);
                    }

                    else if(head->symbolType == RedirectOut) {
                        head->outFileHandle = open(redir, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
                        if(dup2(head->outFileHandle, STDOUT_FILENO) < 0) exit(1);
                        close(head->outFileHandle);
                    }

                    else if(head->symbolType == RedirectOutAppend) {
                        head->outFileHandle = open(redir, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
                        if(dup2(head->outFileHandle, STDOUT_FILENO) < 0) exit(1);
                        close(head->outFileHandle);
                    }

                    else if(head->symbolType == Pipe) {
                        if(dup2(fd[1], STDOUT_FILENO) < 0) exit(1);
                        close(fd[1]);
                    }
                }

                execvp(head->file, head->arglist);
                fprintf(stderr, "Exec Failed\n");
                exit(1);
            }

            // Parent process
            else {
                int status;
                wait(&status);
                head->status = status;

                if(fd[1] > 1) close(fd[1]);
            }

            // Advance past filename for redirect tokens to avoid execution
            if(head->symbolType == RedirectOut ||
               head->symbolType == RedirectIn ||
               head->symbolType == RedirectOutAppend) {

                head = head->next;
            }

            head = head->next;
            j++;
        }
    }

    DeleteCommandChain(head);
    return 0;
}