#include "Corewar.h"

#include <QApplication>
#include <QMenuBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>

Corewar::Corewar()
{
    setWindowTitle(tr("Corewar"));
    m_pNewGameDialog = new NewGameDialog(this);
    connect(m_pNewGameDialog, SIGNAL(newGame()), this, SLOT(newGame()));
    {
        QMenu *file = menuBar()->addMenu(tr("&Game"));
        connect(file->addAction(tr("&New game...")), SIGNAL(triggered()),
            m_pNewGameDialog, SLOT(show()));
        file->addSeparator();
        connect(file->addAction(tr("&Quit")), SIGNAL(triggered()),
            qApp, SLOT(quit()));
    }
    // TODO : display
}

void Corewar::newGame()
{
    // TODO : start a game
}

NewGameDialog::NewGameDialog(Corewar *cw)
  : QDialog(cw)
{
    QVBoxLayout *l = new QVBoxLayout;
    // TODO : configuration and seletion of programs
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

void NewGameDialog::submit()
{
    // TODO emit newGame(
    hide();
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    Corewar cw;
    cw.show();
    return app.exec();
}
