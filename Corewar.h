#ifndef COREWAR_H
#define COREWAR_H

#include "Program.h"
#include "Field.h"
#include <QMainWindow>
#include <QDialog>
#include <QListWidget>
#include <QSpinBox>
#include <QCheckBox>
#include <QTimer>
#include <list>
#include <vector>

class Corewar;

class NewGameDialog : public QDialog {

    Q_OBJECT

private:
    QListWidget *m_pProgramList;
    QSpinBox *m_pSize;
    QCheckBox *m_pForkAllowed;

public:
    NewGameDialog(Corewar *cw);

private slots:
    void addProgram();
    void removeProgram();
    void submit();

signals:
    void newGame(unsigned int size, QStringList programs, bool forkAllowed);

};

struct Process {

    std::list<unsigned int> instructionPointers;
    std::string name;

};

class Corewar : public QMainWindow {

    Q_OBJECT

private:
    NewGameDialog *m_pNewGameDialog;
    Field *m_pField;
    std::vector<Process*> m_aProcesses;
    QTimer *m_pTickTimer;
    QTimer *m_pPaintTimer;

private:
    unsigned int findCell(EOpType type, int op, unsigned int ip);
    int readValue(EOpType type, int op, unsigned int ip);
    void writeValue(EOpType type, int op, unsigned int ip, int value,
        unsigned short owner);
    void mov(EOpType type1, int op1, EOpType type2, int op2, unsigned int ip,
        unsigned short owner);

public:
    Corewar();

private slots:
    void newGame(unsigned int size, QStringList programs, bool forkAllowed);
    void tick();

};

#endif
