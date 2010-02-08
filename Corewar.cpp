#include "Corewar.h"

#include <QApplication>
#include <QMenuBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QTranslator>

Corewar::Corewar()
{
    setWindowTitle(tr("Corewar"));
    m_pNewGameDialog = new NewGameDialog(this);
    connect(m_pNewGameDialog, SIGNAL(newGame(unsigned int, QStringList, bool)),
        this, SLOT(newGame(unsigned int, QStringList, bool)));
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
    m_pPaintTimer->setInterval(100);
    connect(m_pPaintTimer, SIGNAL(timeout()), this, SLOT(repaint()));
}

void Corewar::newGame(unsigned int size, QStringList programs, bool forkAllowed)
{
    // Free the memory
    for(size_t i = 0; i < m_aProcesses.size(); i++)
        delete m_aProcesses[i];
    m_aProcesses.clear();

    std::list<Program*> compiled;

    // Load each program
    try {
        for(int i = 0; i < programs.size(); i++)
            compiled.push_back(
                new Program(programs.at(i).toLocal8Bit(), forkAllowed));
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
    {
        std::list<Program*>::iterator code = compiled.begin();
        for(; code != compiled.end(); code++)
            space -= (*code)->bytecode().size();
    }
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
    std::list<Program*>::iterator code = compiled.begin();
    int i = 0;
    for(; code != compiled.end(); code++, i++)
    {
        const std::vector<Cell> &bytecode = (*code)->bytecode();
        unsigned int start = (unsigned int)pos;
        Process *proc = new Process;
        proc->instructionPointers.push_back(start);
        QFileInfo fi(programs.at(i));
        proc->name = fi.baseName().toStdString();
        m_aProcesses.push_back(proc);
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
    code = compiled.begin();
    for(; code != compiled.end(); code++)
        delete *code;
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
        op = readValue(ADDRESS, op, ip);
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
    m_pField->clearPointers();

    for(size_t i = 0; i < m_aProcesses.size(); i++)
    {
        // No more threads -- this process has lost
        if(m_aProcesses[i]->instructionPointers.size() == 0)
            continue;

        // We take the first thread from the list
        // If it is still running then, we will add it to the end of the list at
        // the end of this loop
        unsigned int ip = m_aProcesses[i]->instructionPointers.front();
        m_aProcesses[i]->instructionPointers.pop_front();

        // The real owner is the owner of the cell we are executing
        unsigned short owner = m_pField->owner(ip);

        Cell &cell = m_pField->cell(ip);
        switch(cell.instr)
        {
        case DAT:
            // Executing a DAT : fail ;-)
            continue;
            break;
        case ADD:
        case SUB:
            {
                if(cell.type2 == IMMEDIATE)
                    // ADD or SUB to a constant : fail ;-)
                    continue;
                int value = readValue(cell.type2, cell.op2, ip);
                int val2 = readValue(cell.type1, cell.op1, ip);
                if(cell.instr == ADD)
                    value += val2;
                else
                    value -= val2;
                writeValue(cell.type2, cell.op2, ip, value, owner);
            }
            break;
        // Note: MOV can copy both DATs and instructions
        case MOV:
            {
                if(cell.type2 == IMMEDIATE)
                    // MOV to a constant : fail ;-)
                    continue;
                mov(cell.type1, cell.op1, cell.type2, cell.op2, ip, owner);
            }
            break;
        case IFE:
        case IFL:
            {
                int val1 = readValue(cell.type1, cell.op1, ip);
                int val2 = readValue(cell.type2, cell.op2, ip);
                if( (val1 == val2 && cell.instr == IFE)
                 || (val1  < val2 && cell.instr == IFL) )
                    ; // Executes the next line
                else
                    // Jump over next line
                    ip++;
            }
            break;
        case JMP:
        case FORK:
            {
                int value;
                switch(cell.type1)
                {
                case IMMEDIATE:
                    // JMP or FORK to a constant : fail ;-)
                    continue;
                    break;
                case ADDRESS:
                    value = readValue(IMMEDIATE, cell.op1, ip);
                    break;
                case DEREFERENCE:
                    value = readValue(ADDRESS, cell.op1, ip);
                    break;
                }

                if(cell.instr == JMP)
                    ip = (ip+value-1) % m_pField->length();
                else
                {
                    m_aProcesses[i]->instructionPointers.push_back(
                        (ip+value) % m_pField->length());
                }
            }
            break;
        }

        // Ok, so this thread is still running...

        // Next instruction
        ip = (ip+1)%m_pField->length();

        // Draw instruction pointers
        m_pField->drawPointer(ip, i+1);

        // Put it back in the list
        m_aProcesses[i]->instructionPointers.push_back(ip);
    }

    bool finished = true;
    int running = 0;
    // finished is true until we find two running processes with different
    // effective owners
    for(size_t i = 0; i < m_aProcesses.size(); i++)
    {
        std::list<unsigned int>::iterator thread;
        thread = m_aProcesses[i]->instructionPointers.begin();
        for(; thread != m_aProcesses[i]->instructionPointers.end(); thread++)
        {
            unsigned short owner = m_pField->owner(*thread);
            if(running == 0)
                running = owner;
            else if(owner != running)
            {
                finished = false;
                break;
            }
        }
    }

    // Less than 2 running programs : the end!
    if(finished)
    {
        m_pTickTimer->stop();
        m_pPaintTimer->stop();
        repaint();
        if(running != 0)
            QMessageBox::information(this, tr("Game finished"), tr("\"%1\" has "
                "won!").arg(m_aProcesses[running-1]->name.c_str()));
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
            {
                QHBoxLayout *l4 = new QHBoxLayout;
                l4->addWidget(new QLabel(tr("FORK allowed?")));
                m_pForkAllowed = new QCheckBox;
                m_pForkAllowed->setCheckState(Qt::Checked);
                l4->addWidget(m_pForkAllowed);
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
    hide();
    QStringList programs;
    for(int i = 0; i < m_pProgramList->count(); i++)
        programs << m_pProgramList->item(i)->text();
    emit newGame(m_pSize->value(), programs,
        m_pForkAllowed->checkState()==Qt::Checked);
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QTranslator translator;
    translator.load(QString("corewar2_") + QLocale::system().name());
    app.installTranslator(&translator);

    Corewar cw;
    cw.show();

    return app.exec();
}
