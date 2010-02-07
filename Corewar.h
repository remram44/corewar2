#ifndef COREWAR_H
#define COREWAR_H

#include "Program.h"
#include "Field.h"
#include <QMainWindow>
#include <QDialog>
#include <QListWidget>
#include <QSpinBox>

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
    void newGame(unsigned int size);

};

class Corewar : public QMainWindow {

    Q_OBJECT

private:
    NewGameDialog *m_pNewGameDialog;
    Field *m_pField;

public:
    Corewar();

private slots:
    void newGame(unsigned int size);

};

#endif
