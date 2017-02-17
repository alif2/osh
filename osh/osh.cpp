#include <cstdlib>
#include <sys/wait.h>

#include "command.h"
#include "parse.h"
#include "osh_const.h"

using namespace osh;

int main(int argc, char** argv) {
    // Command structure
    Command *head = NULL;

    // Bounded input loop to avoid infinite forks
    for(int i = 0; i <= 25; i++) { 
        int cmdcode = GetCommandChain(&head);

        if(cmdcode != status_success) {
            printf("Error reading input");
            exit(1);
        }
        
        // Exit command signals end of input
        if(strcmp(head->file, "exit") == 0) {
            exit(0);
        }
        
        if(head->prev->symbolType != (long int) NULL) {
            if(head->prev->symbolType == ExecuteOnSuccess) {
                if(head->prev->status != 0) {
                    head->status = head->prev->status;
                }
            }

            else if(head->prev->symbolType == ExecuteOnFailure) {
                if(head->prev->status == 0) {
                    head->status = head->prev->status;
                }
            }
        }
        
        if(head->symbolType != (long int) NULL) {
            if(head->symbolType == RedirectIn) {
                
            }
            
            else if(head->symbolType == RedirectOut) {
                head->next->file;
            }
            
            else if(head->symbolType == RedirectOutAppend) {
                
            }
            
            else if(head->symbolType == Pipe) {
                
            }
        }
        
        //DumpCommandChain(head);
        
        pid_t cpid = fork();
        if(cpid < 0) {
            fprintf(stderr, "Fork Failed\n");
            exit(2);
        }
        
        else if(cpid == 0) {
            execvp(head->file, head->arglist);
            fprintf(stderr, "Exec Failed\n");
            exit(3);
        }
        
        else {
            int status;
            wait(&status);
            head->status = status;
        }
    }
    
    DeleteCommandChain(head);
    return 0;
}