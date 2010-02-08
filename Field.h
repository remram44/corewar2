#ifndef FIELD_H
#define FIELD_H

#include "Program.h"

#include <QWidget>
#include <QPaintEvent>
#include <list>

class Field : public QWidget {

    Q_OBJECT

private:
    struct Pointer {
        unsigned int pointer;
        unsigned short owner;
    };

private:
    unsigned int m_iWidth;
    unsigned int m_iHeight;
    unsigned int m_iSize;
    std::vector<Cell> m_aCells;
    std::vector<unsigned short> m_aCellsOwner;
    std::list<Pointer> m_lPointers;

public:
    Field(unsigned int size);
    inline Cell &cell(unsigned int pos)
    {
        return m_aCells[pos];
    }
    inline unsigned short &owner(unsigned int pos)
    {
        return m_aCellsOwner[pos];
    }
    inline unsigned int length() const
    {
        return m_iSize;
    }
    void clearPointers();
    void drawPointer(unsigned int pointer, unsigned short owner);

protected:
    void paintEvent(QPaintEvent *event);

};

#endif
