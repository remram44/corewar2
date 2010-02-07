#ifndef COREWAR_H
#define COREWAR_H

#include "Program.h"

#include <QMainWindow>
#include <QDialog>

class Corewar;

class NewGameDialog : public QDialog {

    Q_OBJECT

public:
    NewGameDialog(Corewar *cw);

private slots:
    void submit();

};

class Corewar : public QMainWindow {

    Q_OBJECT

private:
    NewGameDialog *m_pNewGameDialog;

public:
    Corewar();

private slots:
    void newGame();

};

#endif
