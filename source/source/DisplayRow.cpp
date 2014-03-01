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

#include "DisplayRow.hpp"
#include "SGRAttribute.hpp"
#include "NetSprite.hpp"
#include "WhiteBoard.hpp"

using namespace std;

DisplayRow::DisplayRow(WhiteBoard *newWhiteBoard,
                       uint8_t newSize,
                       int y)
{
    whiteBoard = newWhiteBoard;

    for (unsigned int x = 0; x < newSize; x++)
        theSprites.push_back(new NetSprite(NULL, whiteBoard, x, y));
}//constructor

DisplayRow::~DisplayRow(void)
{
    for (unsigned int i = 0; i < theSprites.size(); i++)
        delete theSprites.at(i);
    theSprites.clear();
}//destructor

void DisplayRow::removeFromScene(QGraphicsScene *theScene)
{
    for (unsigned int i = 0; i < theSprites.size(); i++)
        theScene->removeItem(theSprites.at(i));
}//removeFromScene

unsigned int DisplayRow::size(void)
{
    return theSprites.size();
}//size()

void DisplayRow::setChar(uint8_t index,
                         uint8_t value,
                         SGRAttribute *writeAttributes)
{
    theSprites.at(index)->setChar(value, writeAttributes);
}//setChar

uint8_t DisplayRow::getChar(uint8_t index)
{
    return theSprites.at(index)->getChar();
}//getChar

NetSprite* DisplayRow::getNetSprite(uint8_t index)
{
    return theSprites.at(index);
}//getNetSprite

SGRAttribute* DisplayRow::getAttributes(uint8_t index)
{
    return theSprites.at(index)->getAttributes();
}//getAttributes
