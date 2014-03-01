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

#include "TelnetWindow.hpp"
#include "WhiteBoard.hpp"
#include "NGSettings.hpp"
#include "NetSprite.hpp"
#include "TelnetProtocol.hpp"
#include "ImageLoader.hpp"

using namespace std;

TelnetWindow::TelnetWindow(WhiteBoard *newWhiteBoard)
{
    whiteBoard = newWhiteBoard;

    writeX = 0;
    writeY = 0;
    windowWidth = 0;
    windowHeight = 0;
    displayChanged = true;
    allowEraseAll = true;
    firstEraseAll = true;
    theScene = NULL;
}//constructor

TelnetWindow::~TelnetWindow(void)
{
    clearWindow(theWindow);
}//destructor

void TelnetWindow::clearWindow(vector <DisplayRow*> aWindow)
{
    //### Remove the sprites from the scene ###
    if (theScene != NULL)
    {
        for (unsigned int i = 0; i < aWindow.size(); i++)
            aWindow.at(i)->removeFromScene(theScene);
    }//if theScene

    //### Delete the sprites ###
    for (unsigned int i = 0; i < aWindow.size(); i++)
        delete aWindow.at(i);
    aWindow.clear();

    //### Remember that any new sprites don't belong to a scene ###
    theScene = NULL;
}//clearWindow

bool TelnetWindow::initialize(uint8_t newWidth,
                              uint8_t newHeight)
{
    DisplayRow *oneRow = NULL;//a single row from the display window
    int historyLines = NGSettings::getHistoryLines();
    bool result = true;//false on errors

    //### Verify that the window dimensions aren't zero
    if ((newWidth == 0) || (newHeight == 0))
    {
        cout << "Window width and height must be non-zero" << endl;
        result = false;
    }//if newWidth || newHeight

    //### Verify that the window dimensions are < 255 ###
    if (result)
    {
        if ((newWidth >= 255) || (newHeight >= 255))
        {
            cout << "Window width and height must be less than 255" << endl;
            result = false;
        }//if newWidth || newHeight
    }//if result

    //### Create the window ###
    if (result)
    {
        writeX = 0;
        writeY = 0;
        windowWidth = newWidth;
        windowHeight = newHeight;
        displayChanged = true;

        clearWindow(theWindow);

        for (unsigned y = 0; y < windowHeight; y++)
        {
            oneRow = new DisplayRow(whiteBoard, windowWidth, y + historyLines);
            theWindow.push_back(oneRow);
        }//for y
    }//if result

    return result;
}//initialize

void TelnetWindow::resetWindow(QGraphicsScene *newScene)
{
    std::vector <DisplayRow*> oldWindow;
    NetSprite *destSprite = NULL;//a single sprite from destRow
    NetSprite *sourceSprite = NULL;//a single sprite from sourceRow
    DisplayRow *destRow = NULL;//a row from the display window we're creating
    DisplayRow *sourceRow = NULL;//a row from the old display window
    int historyLines = NGSettings::getHistoryLines();

    //### Save the window contents ###
    oldWindow = theWindow;
    theWindow.clear();

    //### Create the new window ###
    for (unsigned y = 0; y < windowHeight; y++)
    {
        sourceRow = oldWindow.at(y);
        destRow = new DisplayRow(whiteBoard, windowWidth, y + historyLines);
        theWindow.push_back(destRow);

        for (unsigned int x = 0; x < destRow->size(); x++)
        {
            sourceSprite = sourceRow->getNetSprite(x);
            destSprite = destRow->getNetSprite(x);

            destSprite->setChar(sourceSprite->getChar(), sourceSprite->getAttributes());
        }//for x
    }//for y

    //### Delete the old window ###
    clearWindow(oldWindow);

    //### Add the window to the graphics scene ###
    setGraphicsScene(newScene);

    //### Redraw the sprites ###
    for (unsigned int y = 0; y < theWindow.size(); y++)
    {
        for (unsigned int x = 0; x < windowWidth; x++)
            notifyFxChange(x, y);
    }//for y
}//resetWindow

uint8_t TelnetWindow::getByte(uint8_t xPos,
                              uint8_t yPos)
{
    DisplayRow *oneRow = NULL;//the row containing the desired byte

    if (xPos >= windowWidth)
    {
        xPos = windowWidth - 1;
        whiteBoard->getTelnetPro()->showBoundsDialog();
    }//if xPos

    if (yPos >= windowHeight)
    {
        yPos = windowHeight - 1;
        whiteBoard->getTelnetPro()->showBoundsDialog();
    }//if yPos

    oneRow = theWindow.at(yPos);

    return oneRow->getChar(xPos);
}//getByte

SGRAttribute* TelnetWindow::getAttributes(uint8_t xPos,
                                          uint8_t yPos)
{
    DisplayRow *oneRow = NULL;//the row containing the desired attributes

    if (xPos >= windowWidth)
    {
        xPos = windowWidth - 1;
        whiteBoard->getTelnetPro()->showBoundsDialog();
    }//if xPos

    if (yPos >= windowHeight)
    {
        yPos = windowHeight - 1;
        whiteBoard->getTelnetPro()->showBoundsDialog();
    }//if yPos

    oneRow = theWindow.at(yPos);

    return oneRow->getAttributes(xPos);
}//getAttributes

uint8_t TelnetWindow::getCursorX(void)
{
    return writeX;
}//getCursorX

uint8_t TelnetWindow::getCursorY(void)
{
    return writeY;
}//getCursorY

void TelnetWindow::setCursorX(uint8_t newValue)
{
    if (newValue == 0)
        cout << "TelnetWindow::setCursorX(): should be at least 1!" << endl;
    else
        writeX = newValue - 1;

    if (writeX >= windowWidth)
    {
        writeX = windowWidth - 1;
        whiteBoard->getTelnetPro()->showBoundsDialog();
    }//if writeX
}//setCursorX

void TelnetWindow::setCursorY(uint8_t newValue)
{
    if (newValue == 0)
        cout << "TelnetWindow::setCursorY(): should be at least 1!" << endl;
    else
        writeY = newValue - 1;

    if (writeY >= windowHeight)
    {
        writeY = windowHeight -1;
        whiteBoard->getTelnetPro()->showBoundsDialog();
    }//if writeY
}//setCursorY

void TelnetWindow::moveCursorX(int amount)
{
    int testPos = writeX + amount;

    if (testPos < 0)
        writeX = 0;

    else if (testPos >= windowWidth)
        writeX = windowWidth - 1;

    else
        writeX = testPos;
}//moveCursorX

void TelnetWindow::moveCursorY(int amount)
{
    int testPos = writeY + amount;

    if (testPos < 0)
        writeY = 0;

    if (testPos >= windowHeight)
    {
        writeY = windowHeight - 1;
        whiteBoard->getTelnetPro()->showBoundsDialog();
    }//if testPos

    else
        writeY = testPos;
}//moveCursorY

void TelnetWindow::writeByte(uint8_t oneByte)
{
    DisplayRow *oneRow = NULL;//the window row to write to

    oneRow = theWindow.at(writeY);
    oneRow->setChar(writeX, oneByte, &writeAttribute);

    writeX++;
    if (writeX >= windowWidth)
    {
        writeX = 0;
        writeY++;

        if (writeY >= windowHeight)
        {
            writeY = windowHeight - 1;
            whiteBoard->getTelnetPro()->showBoundsDialog();
        }//if writeY
    }//if writeX

    displayChanged = true;
}//writeByte

bool TelnetWindow::getDisplayChanged(void)
{
    bool result = displayChanged;

    displayChanged = false;

    return result;
}//getDisplayChanged

bool TelnetWindow::charChanged(uint8_t x,
                               uint8_t y)
{
    DisplayRow *oneRow = theWindow.at(y);
    NetSprite *oneSprite = oneRow->getNetSprite(x);

    return oneSprite->getIsChanged();
}//charChanged

void TelnetWindow::setGraphicsScene(QGraphicsScene *newScene)
{
    DisplayRow *oneRow = NULL;//the current window row
    NetSprite *currentSprite = NULL;//the current sprite to add to the scene

    //### The window can only be added to one scene ###
    if (theScene != NULL)
    {
        cout << "TelnetWindow::setGraphicsScene(): was already called!" << endl;
        throw 1;
    }//if theScene

    theScene = newScene;

    //### Add each sprite to the scene ###
    for (uint8_t y = 0; y < theWindow.size(); y++)
    {
        oneRow = theWindow.at(y);

        for (uint8_t x = 0; x < windowWidth; x++)
        {
            currentSprite = oneRow->getNetSprite(x);
            theScene->addItem(currentSprite);
        }//for x
    }//for y
}//setGraphicsScene

void TelnetWindow::setGraphicsMode(bool useGraphics,
                                   uint8_t topRow,
                                   uint8_t numRows)
{
    DisplayRow *currentRow = NULL;//the current window row
    NetSprite *currentSprite = NULL;//the current sprite to set graphics mode

    for (unsigned int y = topRow; y < topRow + numRows; y++)
    {
        currentRow = theWindow.at(y);

        for (unsigned int x = 0; x < windowWidth; x++)
        {
            currentSprite = currentRow->getNetSprite(x);
            currentSprite->setGraphicsMode(useGraphics);
        }//for x
    }//for y
}//setGraphicsMode

void TelnetWindow::notifyFxChange(int x,
                                  int y)
{
    DisplayRow *spriteRow = theWindow.at(y);
    NetSprite *oneSprite = spriteRow->getNetSprite(x);

    oneSprite->changeDisplayGraphic();
}//notifyFxChange

uint8_t TelnetWindow::getWidth(void)
{
    return windowWidth;
}//getWidth

uint8_t TelnetWindow::getHeight(void)
{
    return windowHeight;
}//getHeight

void TelnetWindow::enableEraseAll(bool enabled)
{
    allowEraseAll = enabled;
    if (!allowEraseAll)
        firstEraseAll = true;
}//enableEraseAll

void TelnetWindow::eraseAll(void)
{
    DisplayRow *oneRow = NULL;//the current row to alter

    if ((allowEraseAll) || (firstEraseAll))
    {
        firstEraseAll = false;

        for (unsigned int yPos = 0; yPos < windowHeight; yPos++)
        {
            oneRow = theWindow.at(yPos);
            for (unsigned int xPos = 0; xPos < windowWidth; xPos++)
                oneRow->setChar(xPos, ' ', &writeAttribute);
        }//for yPos

        displayChanged = true;
    }//if allowEraseAll || firstEraseAll
}//eraseAll

void TelnetWindow::eraseBelow(void)
{
    DisplayRow *oneRow = NULL;//the current row to alter

    for (unsigned int yPos = writeY; yPos < windowHeight; yPos++)
    {
        oneRow = theWindow.at(yPos);
        oneRow->setChar(writeX, ' ', &writeAttribute);
    }//for yPos

    displayChanged = true;
}//eraseBelow

void TelnetWindow::eraseToRight(void)
{
    DisplayRow *oneRow = NULL;//the current row to alter

    oneRow = theWindow.at(writeY);
    for (unsigned int xPos = writeX; xPos < windowWidth; xPos++)
        oneRow->setChar(xPos, ' ', &writeAttribute);

    displayChanged = true;
}//eraseToRight()

void TelnetWindow::deleteLines(int numDelete)
{
    DisplayRow *oneRow = NULL;//the current row to alter
    DisplayRow *nextRow = NULL;//we'll set oneRow to nextRow
    SGRAttribute *newAttributes = NULL;//the new attributes to copy to oneRow
    int delCount = 0;//the number of characters deleted so far
    uint8_t newChar = 0;//a character to copy to oneRow

    //### Shuffle the lines up ###
    while (delCount < numDelete)
    {
        for (int i = writeY + 1; i < windowHeight; i++)
        {
            oneRow = theWindow.at(i - 1);
            nextRow = theWindow.at(i);

            for (unsigned int j = 0; j < oneRow->size(); j++)
            {
                newChar = nextRow->getChar(j);
                newAttributes = nextRow->getAttributes(j);

                oneRow->setChar(j, newChar, newAttributes);
            }//for j
        }//for i

        delCount++;
    }//while delCount

    //### Clear the last line ###
    oneRow = theWindow.at(windowHeight - 1);
    for (unsigned int i = 0; i < oneRow->size(); i++)
        oneRow->setChar(i, ' ', &writeAttribute);

    displayChanged = true;
}//deleteLines

void TelnetWindow::deleteCharacters(int numDelete)
{
    DisplayRow *oneRow = NULL;//the current row to alter
    int delCount = 0;//the number of characters deleted so far

    oneRow = theWindow.at(writeY);

    while (delCount < numDelete)
    {
        for (int i = writeX + 1; i < windowWidth; i++)
        {
            if (i < windowWidth - 1)
                oneRow->setChar(i, oneRow->getChar(i + 1), &writeAttribute);
            else
                oneRow->setChar(i, ' ', &writeAttribute);
        }//for i

        if (writeX < windowWidth - 1)
            oneRow->setChar(writeX + 1, ' ', &writeAttribute);

        delCount++;
    }//while index && delCount

    displayChanged = true;
}//deleteCharacters

void TelnetWindow::setDefaultAttribute(void)
{
    writeAttribute.setDefault();
}//setDefaultAttribute

void TelnetWindow::setBold(void)
{
    writeAttribute.setBold();
}//setBold

void TelnetWindow::setInverse(void)
{
    writeAttribute.setInverse();
}//setInverse

void TelnetWindow::setInvisible(void)
{
    writeAttribute.setInvisible();
}//setInvisible

void TelnetWindow::setUnderlined(void)
{
    writeAttribute.setUnderlined();
}//setUnderlined

void TelnetWindow::clearBold(void)
{
    writeAttribute.clearBold();
}//clearBold

void TelnetWindow::clearInverse(void)
{
    writeAttribute.clearInverse();
}//clearInverse

void TelnetWindow::clearInvisible(void)
{
    writeAttribute.clearInvisible();
}//clearInvisible

void TelnetWindow::clearUnderlined(void)
{
    writeAttribute.clearUnderlined();
}//clearUnderlined

void TelnetWindow::setForeground(NGS_Attribute color)
{
    writeAttribute.setForeground(color);
}//setForeground

void TelnetWindow::setBackground(NGS_Attribute color)
{
    writeAttribute.setBackground(color);
}//setBackground
