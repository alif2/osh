#include <cstdlib>
#include "command.h"
#include "parse.h"

using namespace osh;

int main(int argc, char** argv) {
    // Command structure
    Command *head = NULL;

    // get the command chain   
    if(GetCommandChain(&head) != 1) {
        DumpCommandChain(head);
        DeleteCommandChain(head);
    }

    return 0;
}