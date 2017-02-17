#ifndef PARSE_H
#define PARSE_H

namespace osh {
    int trim(char *line, size_t *length);
    int allocateBuffer(void **buffer, size_t *currentSize, size_t newSize, size_t datasize);
    int allocateAndCopyString(char** destination, char *source) ;
    int isCommandBreaker(char ch);
    int allocateNewCommand(Command **command) ;
    int deleteCommand(Command *command);
    void DumpCommand(Command* command);
    int AddCommand(Command **head, Command *add);
    void DumpCommandChain(Command *head);
    int GetComandChainLength(Command *head);
    int DeleteCommandChain(Command *head);
    int GetCommandBreaker(char **line, SymbolType *symbol);
    int GetNextCommandString(char **line, char **commandString, size_t *commandBufferLength, size_t *commandLength);
    int ParseCommand(Command **command, char *commandLine, SymbolType symtype);
    int GetCommandChain(Command **head);
}

#endif /* PARSE_H */