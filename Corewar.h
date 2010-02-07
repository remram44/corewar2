#ifndef COREWAR_H
#define COREWAR_H

#include "Program.h"
#include "Field.h"
#include <QMainWindow>
#include <QDialog>
#include <QListWidget>
#include <QSpinBox>
#include <QTimer>

class Corewar;

class NewGameDialog : public QDialog {

    Q_OBJECT

private:
    QListWidget *m_pProgramList;
    QSpinBox *m_pSize;

public:
    NewGameDialog(Corewar *cw);

private slots:
    void addProgram();
    void removeProgram();
    void submit();

signals:
    void newGame(unsigned int size, QStringList programs);

};

struct Process {

    unsigned int instructionPointer;
    unsigned short owner;
    bool running;
    QString name;

};

class Corewar : public QMainWindow {

    Q_OBJECT

private:
    NewGameDialog *m_pNewGameDialog;
    Field *m_pField;
    QList<Process*> m_lProcesses;
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
    void newGame(unsigned int size, QStringList programs);
    void tick();

};

#endif
