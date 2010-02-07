#ifndef PROGRAM_H
#define PROGRAM_H

class SyntaxError {

};

class Program {

public:
    Program(const char *filename) throw(SyntaxError);

};

#endif
