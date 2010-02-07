#include "Corewar.h"

#include <QApplication>
#include <QMenuBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>

Corewar::Corewar()
{
    setWindowTitle(tr("Corewar"));
    m_pNewGameDialog = new NewGameDialog(this);
    connect(m_pNewGameDialog, SIGNAL(newGame(unsigned int)),
        this, SLOT(newGame(unsigned int)));
    {
        QMenu *file = menuBar()->addMenu(tr("&Game"));
        connect(file->addAction(tr("&New game...")), SIGNAL(triggered()),
            m_pNewGameDialog, SLOT(show()));
        file->addSeparator();
        connect(file->addAction(tr("&Quit")), SIGNAL(triggered()),
            qApp, SLOT(quit()));
    }
}

void Corewar::newGame(unsigned int size)
{
    m_pField = new Field(size);
    setCentralWidget(m_pField);
    // TODO : start a game
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
    emit newGame(m_pSize->value());
    hide();
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    Corewar cw;
    cw.show();
    return app.exec();
}
