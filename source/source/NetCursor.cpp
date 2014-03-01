/*Copyright 2009-2013 David McCallum

This file is part of EbonHack.

    EbonHack is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    EbonHack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with EbonHack.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "NetCursor.hpp"
#include "WhiteBoard.hpp"
#include "TelnetProtocol.hpp"
#include "TelnetWindow.hpp"
#include <QPainter>
#include <QObject>

using namespace std;

NetCursor::NetCursor(QGraphicsItem* parent,
                     WhiteBoard *newWhiteBoard) : QGraphicsRectItem(parent)
{
    TelnetProtocol *telnetPro = newWhiteBoard->getTelnetPro();

    whiteBoard = newWhiteBoard;
    telnetWindow = telnetPro->getTelnetWindow();

    oldX = 0;
    oldY = 0;

    setRect(0, 0, NGSettings::getSpriteWidth(), NGSettings::getSpriteHeight());
    cursorColor = Qt::white;
}//constructor

NetCursor::~NetCursor(void)
{
}//destructor

void NetCursor::setCursorPos(uint8_t newX,
                             uint8_t newY)
{
	int historyLines = NGSettings::getHistoryLines();
    unsigned int spriteWidth = NGSettings::getSpriteWidth();
    unsigned int spriteHeight = NGSettings::getSpriteHeight();

	newY += historyLines;

    if ((newX != oldX) || (newY != oldY))
    {
        oldX = newX;
        oldY = newY;

        setPos(newX * spriteWidth, newY * spriteHeight);
    }//if newX || newY
}//setCursorPos

void NetCursor::paint(QPainter *painter,
                      const QStyleOptionGraphicsItem *option,
                      QWidget *widget)
{
    QRectF charRect;//the bounding rectangle for the character
    unsigned int spriteWidth = NGSettings::getSpriteWidth();
    int fontOffset = NGSettings::getFontOffset();
    int rectBottom = 0;

    //### Get rid of compiler warnings about unused parameters ###
    if (option)
    {
    }
    if (widget)
    {
    }

    //### Set the character's bounding box ###
    charRect = boundingRect();
    charRect.setY(charRect.y() + fontOffset);
    rectBottom = static_cast<int>(charRect.bottom());

    //### Draw the cursor ###
    painter->setPen(cursorColor);
    painter->fillRect(1, rectBottom - CURSOR_HEIGHT,
                      spriteWidth - 2, CURSOR_HEIGHT,
                      cursorColor);
}//paint

void NetCursor::setColor(Qt::GlobalColor newColor)
{
    cursorColor = newColor;
    update(boundingRect());
}//setRed

QRectF NetCursor::boundingRect(void)
{
    unsigned int spriteWidth = NGSettings::getSpriteWidth();
    unsigned int spriteHeight = NGSettings::getSpriteHeight();

    return QRectF(0, 0, spriteWidth, spriteHeight);
}//boundingRect
