#ifndef FIELD_H
#define FIELD_H

#include "Program.h"

#include <QWidget>
#include <QPaintEvent>

class Field : public QWidget {

    Q_OBJECT

private:
    unsigned int m_iWidth;
    unsigned int m_iHeight;
    unsigned int m_iSize;
    std::vector<unsigned short> m_aCells;

public:
    Field(unsigned int size);

protected:
    void paintEvent(QPaintEvent *event);

};

#endif
