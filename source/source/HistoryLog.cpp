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

#include "HistoryLog.hpp"
#include "WhiteBoard.hpp"
#include "TelnetProtocol.hpp"
#include "NetSprite.hpp"

using namespace std;

HistoryLog::HistoryLog(WhiteBoard *newWhiteBoard)
{
    int logLines = NGSettings::getHistoryLines();
    string blankLine;//a blank line to add to the history buffer

    whiteBoard = newWhiteBoard;
    addedToScene = false;
    hidden = false;

    for (int i = 0; i < TelnetProtocol::WINDOW_WIDTH; i++)
        blankLine.push_back(' ');

    for (int i = 0; i < logLines; i++)
    {
        history.push_back(new DisplayRow(whiteBoard, TelnetProtocol::WINDOW_WIDTH, i));
        historyBuf.push_back(new string(blankLine));
    }//for i
}//constructor

HistoryLog::~HistoryLog(void)
{
    if (!addedToScene)
    {
        for (unsigned int i = 0; i < history.size(); i++)
            delete history.at(i);
        history.clear();
    }//if !addedToScene

    for (unsigned int i = 0; i < historyBuf.size(); i++)
        delete historyBuf.at(i);
    historyBuf.clear();
}//destructor

void HistoryLog::setGraphicsScene(QGraphicsScene *theScene)
{
    DisplayRow *oneRow = NULL;//the current window row
    NetSprite *currentSprite = NULL;//the current sprite to add to the scene

    //### Verify that we've only been added to one scene ###
    if (addedToScene)
    {
        cout << "NethackFX::setGraphicsScene(): was already called!" << endl;
        throw 1;
    }//if addedToScene

    //### Add each sprite to the scene ###
    for (uint8_t y = 0; y < history.size(); y++)
    {
        oneRow = history.at(y);

        for (uint8_t x = 0; x < oneRow->size(); x++)
        {
            currentSprite = oneRow->getNetSprite(x);
            theScene->addItem(currentSprite);
        }//for x
    }//for y

    addedToScene = true;
}//setGraphicsScene

void HistoryLog::removeFromScene(QGraphicsScene *theScene)
{
    DisplayRow *oneRow = NULL;//the current window row
    NetSprite *currentSprite = NULL;//the current sprite to add to the scene

    if (addedToScene)
    {
        for (uint8_t y = 0; y < history.size(); y++)
        {
            oneRow = history.at(y);

            for (uint8_t x = 0; x < oneRow->size(); x++)
            {
                currentSprite = oneRow->getNetSprite(x);
                theScene->removeItem(currentSprite);
            }//for x
        }//for y

        addedToScene = false;
    }//if addedToScene
}//removeFromScene

void HistoryLog::updateLogic(const string &newLine,
                             bool showGraphics)
{
    TelnetProtocol *telnetPro = whiteBoard->getTelnetPro();
    TelnetWindow *telnetWindow = telnetPro->getTelnetWindow();
    DisplayRow *source = NULL;//the history line to copy from
    DisplayRow *dest = NULL;//the history line to write to
    string *bufDest = NULL;//the historyBuf line to write to
    int charPos = 0;//the index of a character to examine
    int nonSpaceIndex = 0;//the index of the last non-space character in oldHistoryLine
    int logLines = NGSettings::getHistoryLines();
    uint8_t oneChar = ' ';//a single character to copy
    bool foundNonSpace = false;//true if the line we're examining has a non-space character
    bool historyReplaced = false;//true if the oldHistoryLine contents were overwritten

    //### Verify that newLine is the entire first line from the telnet window ###
    if (newLine.size() != static_cast<unsigned int>(TelnetProtocol::WINDOW_WIDTH))
    {
        cout << "HistoryLog::updateLogic(): newLine != WINDOW_WIDTH" << endl;
        throw 1;
    }//if size()

    if (showGraphics)//don't record history when graphics mode disabled
    {
        if (oldHistoryLine.size() > 0)
        {
            //### Find the last non-space char in oldHistoryLine ###
            nonSpaceIndex = oldHistoryLine.size() - 1;
            foundNonSpace = false;

            while ((!foundNonSpace) && (nonSpaceIndex >= 0))
            {
                if (oldHistoryLine.at(nonSpaceIndex) != ' ')
                    foundNonSpace = true;
                else
                    nonSpaceIndex--;
            }//while !foundNonSpace && nonSpaceIndex

            //### Verify that a non-space char was found ###
            if (!foundNonSpace)
            {
                cout << "NethackFX::addHistoryLine(): oldHistoryLine contained only spaces" << endl;
                throw 1;
            }//if !foundNonSpace

            //### See if the history text was overwritten ###
            charPos = 0;
            while ((charPos <= nonSpaceIndex) && (!historyReplaced))
            {
                if (telnetWindow->charChanged(charPos, 0))
                    historyReplaced = true;
                charPos++;
            }//while charPos && !historyReplaced

            //### We have a new history line, add it to the user history ###
            if (historyReplaced)
            {
                //Shuffle the history up one line
                for (int y = 1; y < logLines; y++)
                {
                    dest = history.at(y - 1);
                    bufDest = historyBuf.at(y - 1);
                    source = history.at(y);

                    for (unsigned int x = 0; x < source->size(); x++)
                    {
                        oneChar = source->getChar(x);
                        dest->setChar(x, oneChar, &logAttributes);
                        bufDest->at(x) = oneChar;
                    }//for x
                }//for y

                //Add the oldHistoryLine to the history
                dest = history.at(logLines - 1);
                bufDest = historyBuf.at(logLines - 1);

                for (unsigned int i = 0; i < newLine.size(); i++)
                {
                    oneChar = oldHistoryLine.at(i);
                    dest->setChar(i, oneChar, &logAttributes);
                    bufDest->at(i) = oneChar;
                }//for i
            }//if historyReplaced

            //Clear the old line
            oldHistoryLine.clear();
        }//if size()

        //### Only buffer lines that are not blank and don't start with '#' ###
        if (newLine.at(0) != '#')
        {
            charPos = 0;
            foundNonSpace = false;

            while ((!foundNonSpace) && (static_cast<unsigned int>(charPos) < newLine.size()))
            {
                if (newLine.at(charPos) != ' ')
                {
                    oldHistoryLine.append(newLine);
                    foundNonSpace = true;
                }//if newLine

                charPos++;
            }//while !foundNonSpace && charPos
        }//if newLine
    }//if showGraphics

    //### Clear the changed flag from the first line ###
    for (int i = 0; i < TelnetProtocol::WINDOW_WIDTH; i++)
        telnetWindow->charChanged(i, 0);
}//updateLogic

void HistoryLog::show(void)
{
    DisplayRow *dest = NULL;//the current row to show
    string *bufSource = NULL;//a single line from the history buffer
    char oneChar = ' ';//the current character to show

    if (hidden)
    {
        hidden = false;

        for (unsigned int y = 0; y < history.size(); y++)
        {
            dest = history.at(y);
            bufSource = historyBuf.at(y);

            for (unsigned int x = 0; x < dest->size(); x++)
            {
                oneChar = bufSource->at(x);
                dest->setChar(x, oneChar, &logAttributes);
            }//for x
        }//for y
    }//if hidden
}//show

void HistoryLog::hide(void)
{
    DisplayRow *oneRow = NULL;//the current history row to clear

    if (!hidden)
    {
        hidden = true;

        for (unsigned int y = 0; y < history.size(); y++)
        {
            oneRow = history.at(y);

            for (unsigned int x = 0; x < oneRow->size(); x++)
                oneRow->setChar(x, ' ', &logAttributes);
        }//for y
    }//if !hidden
}//hide

void HistoryLog::clearHistory(void)
{
    DisplayRow *oneRow = NULL;//the current history row to clear
    string *oneBufRow = NULL;//the current history buffer row to clear

    for (unsigned int y = 0; y < history.size(); y++)
    {
        oneRow = history.at(y);
        oneBufRow = historyBuf.at(y);

        for (unsigned int x = 0; x < oneRow->size(); x++)
        {
            oneRow->setChar(x, ' ', &logAttributes);
            oneBufRow->at(x) = ' ';
        }//for x
    }//for y

    oldHistoryLine.clear();
}//clearHistory
