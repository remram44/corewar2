#ifndef PROGRAM_H
#define PROGRAM_H

#include <vector>

class SyntaxError {

};

enum EInstruction {
    ADD,
    SUB,
    MOV,
    IFE,
    IFL,
    JMP
};

enum EOpType {
    IMMEDIATE,  // $ the value to be used
    ADDRESS,    // # the address of the value
    DEREFERENCE // @ the address of the pointer to the value
};

/**
 * A memory cell.
 *
 * To simplify the game, a cell is the smallest addressable entity, and it
 * contains either a number (DAT) or an instruction and its parameters.
 */
union Cell {

    int data;
    struct {
        EInstruction instr :4;
        EOpType      type1 :2;
        EOpType      type2 :2;
        int op1;
        int op2;
    } instruction;

};

class Program {

private:
    std::vector<Cell> bytecode;

public:
    Program(const char *filename) throw(SyntaxError);

};

#endif
