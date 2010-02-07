#include "Corewar.h"

#include <QApplication>
#include <QMenuBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>

Corewar::Corewar()
{
    setWindowTitle(tr("Corewar"));
    m_pNewGameDialog = new NewGameDialog(this);
    connect(m_pNewGameDialog, SIGNAL(newGame(unsigned int, QStringList)),
        this, SLOT(newGame(unsigned int, QStringList)));
    {
        QMenu *file = menuBar()->addMenu(tr("&Game"));
        connect(file->addAction(tr("&New game...")), SIGNAL(triggered()),
            m_pNewGameDialog, SLOT(show()));
        file->addSeparator();
        connect(file->addAction(tr("&Quit")), SIGNAL(triggered()),
            qApp, SLOT(quit()));
    }

    m_pTickTimer = new QTimer(this);
    m_pTickTimer->setSingleShot(false);
    m_pTickTimer->setInterval(5);
    connect(m_pTickTimer, SIGNAL(timeout()), this, SLOT(tick()));
    m_pPaintTimer = new QTimer(this);
    m_pPaintTimer->setSingleShot(false);
    m_pPaintTimer->setInterval(500);
    connect(m_pPaintTimer, SIGNAL(timeout()), this, SLOT(repaint()));
}

void Corewar::newGame(unsigned int size, QStringList programs)
{
    // Free the memory
    for(int i = 0; i < m_lProcesses.size(); i++)
        delete m_lProcesses.at(i);

    QList<Program*> compiled;

    // Load each program
    try {
        for(int i = 0; i < programs.size(); i++)
            compiled << new Program(programs.at(i).toLocal8Bit());
    }
    catch(SyntaxError &e)
    {
        QMessageBox::warning(this, tr("Error loading program"), e.what());
        return ;
    }

    // Create the grid
    m_pField = new Field(size);
    setCentralWidget(m_pField);

    // The total space between programs in memory
    double space = size;
    for(int i = 0; i < compiled.size(); i++)
        space -= compiled.at(i)->bytecode().size();
    if(space < 0.0)
    {
        QMessageBox::warning(this, tr("Error setting up memory"),
            tr("The memory size is too small to be able to load all the "
                "programs. Please remove programs or increase the size of the "
                "memory."));
        return ;
    }

    // Space between two following programs in memory
    space /= compiled.size();

    // Copy all programs in memory, regularily spaced
    double pos = 0;
    for(int i = 0; i < compiled.size(); i++)
    {
        const std::vector<Cell> &bytecode = compiled.at(i)->bytecode();
        unsigned int start = (unsigned int)pos;
        Process *proc = new Process;
        proc->instructionPointer = start;
        proc->owner = i+1;
        proc->running = true;
        QFileInfo fi(programs.at(i));
        proc->name = fi.baseName();
        m_lProcesses << proc;
        for(unsigned int j = 0; j < bytecode.size(); j++)
        {
            m_pField->cell(start+j) = bytecode[j];
            m_pField->owner(start+j) = i+1;
        }
        pos += bytecode.size() + space;
    }

    // Start the timers
    m_pTickTimer->start();
    m_pPaintTimer->start();

    // Free the memory
    for(int i = 0; i < compiled.size(); i++)
        delete compiled.at(i);
}

unsigned int Corewar::findCell(EOpType type, int op, unsigned int ip)
{
    switch(type)
    {
    case IMMEDIATE:
    default:
        // Shouldn't happen
        QMessageBox::critical(this, "Fatal error",
            "OOPS : findCell on IMMEDIATE operand!\n");
        qApp->quit();
        return 0;
        break;
    case ADDRESS:
        return (op + ip) % m_pField->length();
        break;
    case DEREFERENCE:
        op = readValue(ADDRESS, (op + ip) % m_pField->length(), ip);
        return (op + ip) % m_pField->length();
        break;
    }
}

int Corewar::readValue(EOpType type, int op, unsigned int ip)
{
    if(type == IMMEDIATE)
        return op;
    else
    {
        unsigned int cell = findCell(type, op, ip);
        if(m_pField->cell(cell).instr == DAT)
            return m_pField->cell(cell).op1;
        else
            return 0;
    }
}

void Corewar::writeValue(EOpType type, int op, unsigned int ip, int value,
    unsigned short owner)
{
    unsigned int cell = findCell(type, op, ip);
    m_pField->cell(cell).instr = DAT;
    m_pField->cell(cell).op1 = value;
    m_pField->owner(cell) = owner;
}

void Corewar::mov(EOpType type1, int op1, EOpType type2, int op2,
    unsigned int ip, unsigned short owner)
{
    unsigned int dest = findCell(type2, op2, ip);
    if(type1 == IMMEDIATE)
    {
        m_pField->cell(dest).instr = DAT;
        m_pField->cell(dest).op1 = op1;
        m_pField->owner(dest) = owner;
    }
    else
    {
        m_pField->cell(dest) = m_pField->cell(findCell(type1, op1, ip));
        m_pField->owner(dest) = owner;
    }
}

void Corewar::tick()
{
    unsigned int running = 0;
    bool finished = true;
    for(int i = 0; i < m_lProcesses.count(); i++)
    {
        if(!m_lProcesses.at(i)->running)
            continue;

        unsigned int ip = m_lProcesses.at(i)->instructionPointer;
        // The real owner is the owner of the cell we are executing
        unsigned short owner = m_pField->owner(ip);

        // finished is true until we find two running processes with different
        // owners
        if(finished)
        {
            if(running == 0)
                running = owner;
            else if(owner != running)
                finished = false;
        }

        Cell &cell = m_pField->cell(ip);
        switch(cell.instr)
        {
        case DAT:
            // Executing a DAT : fail ;-)
            m_lProcesses.at(i)->running = false;
            break;
        case ADD:
            {
                if(cell.type2 == IMMEDIATE)
                    // ADD to a constant : fail ;-)
                    m_lProcesses.at(i)->running = false;
                int value = readValue(cell.type1, cell.op1, ip);
                value += readValue(cell.type2, cell.op2, ip);
                writeValue(cell.type2, cell.op2, ip, value, owner);
            }
            break;
        case SUB:
            {
                if(cell.type2 == IMMEDIATE)
                    // SUB to a constant : fail ;-)
                    m_lProcesses.at(i)->running = false;
                int value = readValue(cell.type1, cell.op1, ip);
                value += readValue(cell.type2, cell.op2, ip);
                writeValue(cell.type2, cell.op2, ip, value, owner);
            }
            break;
        // Note: MOV can copy both DATs and instructions
        case MOV:
            {
                if(cell.type2 == IMMEDIATE)
                    // MOV to a constant : fail ;-)
                    m_lProcesses.at(i)->running = false;
                mov(cell.type1, cell.op1, cell.type2, cell.op2, ip, owner);
            }
            break;
        case IFE:
            {
                int val1 = readValue(cell.type1, cell.op1, ip);
                int val2 = readValue(cell.type2, cell.op2, ip);
                if(val1 == val2)
                    ; // Executes the next line
                else
                    // Jump over next line
                    m_lProcesses.at(i)->instructionPointer++;
            }
            break;
        case IFL:
            {
                int val1 = readValue(cell.type1, cell.op1, ip);
                int val2 = readValue(cell.type2, cell.op2, ip);
                if(val1 < val2)
                    ; // Executes the next line
                else
                    // Jump over next line
                    m_lProcesses.at(i)->instructionPointer++;
            }
            break;
        case JMP:
            switch(cell.type1)
            {
            case IMMEDIATE:
                // Jump to a constant : fail ;-)
                m_lProcesses.at(i)->running = false;
                break;
            case ADDRESS:
                m_lProcesses.at(i)->instructionPointer += readValue(IMMEDIATE,
                    cell.op1, ip) - 1;
                break;
            case DEREFERENCE:
                m_lProcesses.at(i)->instructionPointer += readValue(ADDRESS,
                    cell.op1, ip) - 1;
            }
        }

        // Next instruction
        m_lProcesses.at(i)->instructionPointer++;
    }

    if(finished)
    {
        m_pTickTimer->stop();
        m_pPaintTimer->stop();
        repaint();
        if(running != 0)
            QMessageBox::information(this, tr("Game finished"),
                tr("\"%1\" has won!").arg(m_lProcesses.at(running-1)->name));
        else
            QMessageBox::information(this, tr("Game finished"),
                tr("No program survived :("));
    }
}

NewGameDialog::NewGameDialog(Corewar *cw)
  : QDialog(cw)
{
    QVBoxLayout *l = new QVBoxLayout;
    {
        QHBoxLayout *l2 = new QHBoxLayout;
        m_pProgramList = new QListWidget;
        l2->addWidget(m_pProgramList);
        {
            QVBoxLayout *l3 = new QVBoxLayout;
            QPushButton *a = new QPushButton(tr("Add program..."));
            connect(a, SIGNAL(clicked()), this, SLOT(addProgram()));
            l3->addWidget(a);
            QPushButton *r = new QPushButton(tr("Remove program"));
            connect(r, SIGNAL(clicked()), this, SLOT(removeProgram()));
            l3->addWidget(r);
            {
                QHBoxLayout *l4 = new QHBoxLayout;
                l4->addWidget(new QLabel(tr("Size:")));
                m_pSize = new QSpinBox;
                m_pSize->setMaximum(16384);
                m_pSize->setValue(4096);
                l4->addWidget(m_pSize);
                l3->addLayout(l4);
            }
            l2->addLayout(l3);
        }
        l->addLayout(l2);
    }
    {
        QHBoxLayout *l2 = new QHBoxLayout;
        QPushButton *ok = new QPushButton(tr("Ok"));
        connect(ok, SIGNAL(clicked()), this, SLOT(submit()));
        l2->addWidget(ok);
        QPushButton *cancel = new QPushButton(tr("Cancel"));
        connect(cancel, SIGNAL(clicked()), this, SLOT(hide()));
        l2->addWidget(cancel);
        l->addLayout(l2);
    }
    setLayout(l);
}

void NewGameDialog::addProgram()
{
    QStringList files = QFileDialog::getOpenFileNames(this,
        tr("Choose programs"));
    if(!files.empty())
        m_pProgramList->addItems(files);
}

void NewGameDialog::removeProgram()
{
    delete m_pProgramList->takeItem(m_pProgramList->currentRow());
}

void NewGameDialog::submit()
{
    QStringList programs;
    for(int i = 0; i < m_pProgramList->count(); i++)
        programs << m_pProgramList->item(i)->text();
    emit newGame(m_pSize->value(), programs);
    hide();
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    Corewar cw;
    cw.show();
    return app.exec();
}
