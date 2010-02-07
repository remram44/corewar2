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
    unsigned int i;
    for(i = 0; i < m_iWidth * m_iHeight; i++)
        m_aCells[i] = 0;
}

void Field::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setPen(QPen(QColor(0, 0, 0)));
    unsigned int x, y;
    for(y = 0; y < m_iHeight; y++)
    {
        for(x = 0; x < m_iWidth; x++)
        {
            unsigned char *color = colors[m_aCells[y*m_iWidth + x]];
            painter.setBrush(QColor(color[0], color[1], color[2]));
            painter.drawRect(QRect(x*10, y*10, (x+1)*10, (y+1)*10));
        }
    }
}
