#include "Field.h"

#include <cmath>
#include <QPainter>

unsigned char colors[][3] = {
    {0, 0, 0},
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255},
    {255, 255, 0},
    {255, 0, 255},
    {0, 255, 255},
    {255, 255, 255}
    };

Field::Field(unsigned int size)
  : m_iSize(size)
{
    m_iWidth = (unsigned int)sqrt(size);
    m_iHeight = (size-1)/m_iWidth + 1;
    setFixedSize(m_iWidth * 10, m_iHeight * 10);
    m_aCells.resize(m_iWidth * m_iHeight);
    m_aCellsOwner.resize(m_iWidth * m_iHeight);
    for(unsigned int i = 0; i < m_iWidth * m_iHeight; i++)
    {
        m_aCells[i].instr = DAT;
        m_aCells[i].op1 = 0;
        m_aCellsOwner[i] = 0;
    }
}

void Field::drawPointer(unsigned int pointer, unsigned short owner)
{
    Pointer p = {pointer, owner};
    m_lPointers.push_back(p);
}

void Field::clearPointers()
{
    m_lPointers.clear();
}

void Field::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setPen(QPen(QColor(0, 0, 0)));
    for(unsigned int y = 0; y < m_iHeight; y++)
    {
        for(unsigned int x = 0; x < m_iWidth; x++)
        {
            unsigned char *color = colors[m_aCellsOwner[y*m_iWidth + x]];
            painter.setBrush(QColor(color[0], color[1], color[2]));
            painter.drawRect(QRect(x*10, y*10, 10, 10));
        }
    }

    painter.setBrush(Qt::NoBrush);
    std::list<Pointer>::iterator pointer = m_lPointers.begin();
    for(; pointer != m_lPointers.end(); pointer++)
    {
        unsigned char *color = colors[pointer->owner];
        painter.setPen(QColor(color[0], color[1], color[2]));
        unsigned int y = pointer->pointer/m_iWidth;
        unsigned int x = pointer->pointer - y*m_iWidth;
        painter.drawRect(QRect(x*10, y*10, 10, 10));
    }
}
