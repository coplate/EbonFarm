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

#include "NetSprite.hpp"
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include "SGRAttribute.hpp"
#include "WhiteBoard.hpp"
#include "NethackFX.hpp"
#include "TelnetWindow.hpp"
#include "TelnetProtocol.hpp"
#include "XtermEscape.hpp"

using namespace std;

NetSprite::NetSprite(QGraphicsItem* parent,
                     WhiteBoard *newWhiteBoard,
                     int newXPos,
                     int newYPos) : QGraphicsRectItem(parent)
{
    TelnetProtocol *telnetPro = newWhiteBoard->getTelnetPro();
    unsigned int spriteWidth = 0;//The dimensions of each sprite in pixels
    unsigned int spriteHeight = 0;

    whiteBoard = newWhiteBoard;
    xPos = newXPos;
    yPos = newYPos;

    theAttributes = new SGRAttribute;
    telnetChar = ' ';
    isChanged = true;
    thePixmap = NULL;
    haveGraphic = false;
    useGraphic = false;
    telnetWindow = telnetPro->getTelnetWindow();

    spriteWidth = NGSettings::getSpriteWidth();
    spriteHeight = NGSettings::getSpriteHeight();

    setRect(0, 0, spriteWidth, spriteHeight);
    setPos(xPos * spriteWidth, yPos * spriteHeight);
}//constructor

NetSprite::~NetSprite(void)
{
    delete theAttributes;
    theAttributes = NULL;

    thePixmap = NULL;
}//destructor

void NetSprite::paint(QPainter *painter,
                      const QStyleOptionGraphicsItem *option,
                      QWidget *widget)
{
    //### Get rid of compiler warnings about unused parameters ###
    if (option)
    {
    }
    if (widget)
    {
    }

    //### Draw the sprite ###
    if ((useGraphic) && (haveGraphic))
        painter->drawPixmap(0, 0, *thePixmap);
    else
        painter->drawPixmap(0, 0, displayText);
}//paint

QRectF NetSprite::boundingRect(void)
{
    unsigned int spriteWidth = NGSettings::getSpriteWidth();
    unsigned int spriteHeight = NGSettings::getSpriteHeight();

    return QRectF(0, 0, spriteWidth, spriteHeight);
}//boundingRect

void NetSprite::requestRedraw(void)
{
    update(NetSprite::boundingRect());
}//requestRedraw

void NetSprite::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
}//mousePressEvent

void NetSprite::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    NethackFX *netFX = whiteBoard->getNetFX();
    int spriteWidth = NGSettings::getSpriteWidth();
    int spriteHeight = NGSettings::getSpriteHeight();
    int mouseX = static_cast<int>(event->pos().x());
    int mouseY = static_cast<int>(event->pos().y());
    int historyLines = NGSettings::getHistoryLines();

    if ((mouseX > 0) && (mouseX < spriteWidth))
    {
        if ((mouseY > 0) && (mouseY < spriteHeight))
        {
            if (event->modifiers() & Qt::ShiftModifier)
            {
                //### Send a what is command to the server ###
                if (event->button() == Qt::LeftButton)
                    netFX->sendWhatIs(xPos, yPos - historyLines, telnetChar, theAttributes,
                                      false);

                //### Map an unknown character to a graphic ###
                else if (event->button() == Qt::RightButton)
                    netFX->sendWhatIs(xPos, yPos - historyLines, telnetChar, theAttributes,
                                      true);
            }//if ShiftModifier
        }//if mouseY && mouseY
    }//if mouseX && mouseX

    event->accept();
}//mouseReleaseEvent()

void NetSprite::setChar(uint8_t newValue,
                        SGRAttribute *writeAttributes)
{
    telnetChar = newValue;
    theAttributes->setValues(writeAttributes);

    changeDisplayGraphic();//must be before changeDisplayChar()
    changeDisplayChar();

    isChanged = true;
}//setChar

void NetSprite::changeDisplayGraphic(void)
{
    NethackFX *netFX = whiteBoard->getNetFX();
    TelnetProtocol *telnetPro = whiteBoard->getTelnetPro();
    XtermEscape *escHandler = telnetPro->getEscHandler();
    int tileNumber = escHandler->getTileNumber();

    //### Use the graphic indicated by the server ###
    if (escHandler->getUseTileNumber())
    {
        thePixmap = netFX->getImage(tileNumber);
        if (thePixmap == NULL)
        {
            cout << "NetSprite::changeDisplayGraphic(): the server gave us an invalid glyph: " << tileNumber << endl;
            haveGraphic = false;
        }//if thePixmap
        else
            haveGraphic = true;
    }//if getUseTileNumber()

    //### Find the graphic for this character ###
    else
    {
        thePixmap = netFX->findImage(telnetChar, theAttributes);
        if (thePixmap == NULL)
            haveGraphic = false;
        else
            haveGraphic = true;
    }//else getUseTileNumber()

    //### Schedule a redraw ###
    if ((useGraphic) && (haveGraphic))
        requestRedraw();
}//changeDisplayGraphic

void NetSprite::changeDisplayChar(void)
{
    QFont *telnetFont = NGSettings::getTelnetFont();
    unsigned int spriteWidth = NGSettings::getSpriteWidth();
    unsigned int spriteHeight = NGSettings::getSpriteHeight();

    QString convertedChar;//the QString version of telnetChar
    QImage charBuffer(spriteWidth, spriteHeight, QImage::Format_ARGB32);
    QPainter painter(&charBuffer);//performs the drawing
    QRectF charRect;//the bounding rectangle for the character
    string converter;//converts newValue to a QString
    int fontOffset = NGSettings::getFontOffset();

    //### Color the char buffer black ###
    charBuffer.fill(qRgb(0, 0, 0));

    //### Extract the telnet character ###
    converter.push_back(telnetChar);
    convertedChar = QString(converter.c_str());

    //### Set the font color ###
    switch (theAttributes->getForeground())
    {
        case NGSA_BLACK: painter.setPen(Qt::black);
            break;

        case NGSA_RED: painter.setPen(Qt::red);
            break;

        case NGSA_GREEN: painter.setPen(Qt::green);
            break;

        case NGSA_YELLOW: painter.setPen(Qt::yellow);
            break;

        case NGSA_BLUE: painter.setPen(Qt::blue);
            break;

        case NGSA_MAGENTA: painter.setPen(Qt::magenta);
            break;

        case NGSA_CYAN: painter.setPen(Qt::cyan);
            break;

        case NGSA_WHITE: painter.setPen(Qt::white);
            break;

        default:
            cout << "NetSprite::getHTML(): unknown charColor" << endl;
            throw 1;
    }//switch getForeground()

    //### Set the character's bounding box ###
    charRect = boundingRect();
    charRect.setY(charRect.y() + fontOffset);

    //### Draw the character ###
    painter.setFont(*telnetFont);
    painter.drawText(charRect, Qt::AlignCenter, convertedChar);
    painter.end();

    //### Set the display text ###
    displayText = QPixmap::fromImage(charBuffer, Qt::ColorOnly);
    if ((!useGraphic) || (!haveGraphic))
        requestRedraw();
}//changeDisplayChar

void NetSprite::setGraphicsMode(bool graphicsOn)
{
    if (useGraphic != graphicsOn)
    {
        useGraphic = graphicsOn;
        requestRedraw();
    }//if graphicsOn
}//setGraphicsMode

uint8_t NetSprite::getChar(void)
{
    return telnetChar;
}//getChar

SGRAttribute* NetSprite::getAttributes(void)
{
    return theAttributes;
}//getAttributes

bool NetSprite::getIsChanged(void)
{
    bool result = false;//true if this char was changed since the previous call

    result = isChanged;
    isChanged = false;

    return result;
}//getIsChanged
