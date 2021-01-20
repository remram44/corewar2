#ifndef PROGRAM_H
#define PROGRAM_H

#include <vector>
#include <QString>

class SyntaxError {

private:
    const QString m_sWhat;

public:
    SyntaxError(const QString &w);
    virtual const QString &what() const;

};

enum EInstruction {
    DAT,
    ADD,
    SUB,
    MOV,
    IFE,
    IFL,
    JMP,
    FORK
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
struct Cell {

    EInstruction instr :4;
    EOpType      type1 :2;
    EOpType      type2 :2;
    int op1;
    int op2;

};

class Program {

private:
    std::vector<Cell> m_aBytecode;

public:
    Program(const char *filename, bool forkAllowed);
    inline const std::vector<Cell> &bytecode() const
    {
        return m_aBytecode;
    }

};

#endif
