#ifndef OSH_ENUM_H
#define OSH_ENUM_H

namespace osh {
    // Symbol type represents the command separator
    typedef enum SymbolType {
        RedirectIn,             // <
        RedirectOut,            // >
        RedirectOutAppend,      // >>
        ExecuteOnSuccess,       // && - exec on success
        ExecuteOnFailure,       // || - exec on failure 
        Pipe,                   // | 
        Null,                   // end of string (null character encountered)
        NewLine,                // end of command due to new line
        Semicolon,              // ;
    } SymbolType;
}

#endif /* OSH_ENUM_H */