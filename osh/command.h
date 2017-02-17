#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include "osh_enum.h"

#ifndef COMMAND_H
#define COMMAND_H

namespace osh {
    // Command contains the parsed command. These structures are chained in a 
    // doubly linked list 
    typedef struct Command {
        char *file;             // file to execute 
        char **arglist;         // argument list to executable
        SymbolType symbolType;  // command seperator 

        FILE *inFilePtr;        // file pointer to input stream 
        FILE *outFilePtr;       // file pointer to output stream 
        FILE *errorFilePtr;     // file pointer to error stream 
        int inFileHandle;       // file handle to input stream 
        int outFileHandle;      // file handle to output stream
        int errorFileHandle;    // file handle to error stream 
        int status;             // exit code of the commnad

        struct Command *next, *prev;   
    } Command;
}

#endif /* COMMAND_H */