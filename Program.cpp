#include "Program.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <cctype>

Program::Program(const char *filename) throw(SyntaxError)
{
    // TODO : loading a program
    std::ifstream file(filename, std::ios::in);
    unsigned int line_number = 1;
    std::string line;
    std::getline(file, line);
    for(; !file.eof() && !file.fail(); std::getline(file, line), line_number++)
    {
        // Remove the commentary
        {
            size_t com = line.rfind(';');
            if(com != std::string::npos)
                line = line.substr(0, com);
        }

        Cell cell;

        // Read the instruction
        size_t b = line.find_first_not_of(" \t");
        if(b == std::string::npos)
            return ;
        size_t e = line.find_first_of(" \t", b);
        if(e == std::string::npos)
            e = line.size();
        std::string instr = line.substr(b, e-b);
        std::transform(instr.begin(), instr.end(), instr.begin(),
            (int (*)(int))std::tolower);
        unsigned int nb_ops = 2;
        if(instr == "add")
            cell.instruction.instr = ADD;
        else if(instr == "sub")
            cell.instruction.instr = SUB;
        else if(instr == "mov")
            cell.instruction.instr = MOV;
        else if(instr == "ife")
            cell.instruction.instr = IFE;
        else if(instr == "ifl")
            cell.instruction.instr = IFL;
        else if(instr == "jmp")
        {
            cell.instruction.instr = JMP;
            nb_ops = 1;
        }

        // Read the operands
        unsigned int i;
        for(i = 1; i <= nb_ops; i++)
        {
            EOpType type;
            int value;
            b = line.find_first_not_of(" \t", e);
            if(b == std::string::npos)
            {
                std::cerr << filename << ":" << line_number
                    << ": missing operand " << i << "\n";
                throw SyntaxError();
            }
            e = line.find_first_not_of(" \t", b);
            if(e == std::string::npos)
                e = line.size();
            switch(line[b])
            {
            case '$':
                type = IMMEDIATE;
                break;
            case '#':
                type = ADDRESS;
                break;
            case '@':
                type = DEREFERENCE;
                break;
            default:
                std::cerr << filename << ":" << line_number
                    << ": missing operand type for operand " << i << "\n";
                throw SyntaxError();
                break;
            }
            std::istringstream iss(line.substr(b+1, e-b-1));
            if(iss.eof() || (iss >> value, false) || iss.fail() || !iss.eof())
            {
                std::cerr << filename << ":" << line_number
                    << ": missing operand value for operand " << i << "\n";
                throw SyntaxError();
            }
            if(i == 1)
            {
                cell.instruction.type1 = type;
                cell.instruction.op1 = value;
            }
            else
            {
                cell.instruction.type2 = type;
                cell.instruction.op2 = value;
            }
        }
        bytecode.push_back(cell);
    }
}
