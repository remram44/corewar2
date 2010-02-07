#include "Program.h"
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <QObject>

SyntaxError::SyntaxError(const QString &w)
  : m_sWhat(w)
{
}

const QString &SyntaxError::what() const
{
    return m_sWhat;
}

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
            cell.instr = ADD;
        else if(instr == "sub")
            cell.instr = SUB;
        else if(instr == "mov")
            cell.instr = MOV;
        else if(instr == "ife")
            cell.instr = IFE;
        else if(instr == "ifl")
            cell.instr = IFL;
        else if(instr == "jmp")
        {
            cell.instr = JMP;
            nb_ops = 1;
        }
        else if(instr == "dat")
            cell.instr = DAT;

        if(cell.instr == DAT)
        {
            // Read the value
            b = line.find_first_not_of(" \t", e);
            if(b == std::string::npos)
            {
                throw SyntaxError(QObject::tr("%1:%2: missing value of dat")
                    .arg(filename).arg(line_number));
            }
            e = line.find_first_of(" \t", b);
            if(e == std::string::npos)
                e = line.size();
            std::istringstream iss(line.substr(b, e-b));
            int value;
            if(iss.eof() || (iss >> value, false) || iss.fail() || !iss.eof())
            {
                throw SyntaxError(QObject::tr("%1:%2: incorrect operand value "
                    "for dat").arg(filename).arg(line_number));
            }
            cell.op1 = value;
        }
        else for(unsigned int i = 1; i <= nb_ops; i++)
        {
            // Read the operands
            EOpType type;
            int value;
            b = line.find_first_not_of(" \t", e);
            if(b == std::string::npos)
            {
                throw SyntaxError(QObject::tr("%1:%2: missing operand %3\n")
                    .arg(filename).arg(line_number).arg(i));
            }
            e = line.find_first_of(" \t", b);
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
                throw SyntaxError(QObject::tr("%1:%2: missing operand type for "
                    "operand %3").arg(filename).arg(line_number).arg(i));
                break;
            }
            std::istringstream iss(line.substr(b+1, e-b-1));
            if(iss.eof() || (iss >> value, false) || iss.fail() || !iss.eof())
            {
                throw SyntaxError(QObject::tr("%1:%2: incorrect operand value "
                    "for operand %3").arg(filename).arg(line_number).arg(i));
            }
            if(i == 1)
            {
                cell.type1 = type;
                cell.op1 = value;
            }
            else
            {
                cell.type2 = type;
                cell.op2 = value;
            }
        }
        m_aBytecode.push_back(cell);
    }
}
