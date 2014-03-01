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

#include "XtermEscape.hpp"
#include "WhiteBoard.hpp"
#include "GraphicsSettings.hpp"
#include "MainWindow.hpp"

using namespace std;

XtermEscape::XtermEscape(TelnetWindow *newWindow,
                         WhiteBoard *newWhiteBoard,
                         bool newDebug)
{
    theWindow = newWindow;
    whiteBoard = newWhiteBoard;
    debugMessages = newDebug;

    resetFSM();
}//constructor

XtermEscape::~XtermEscape(void)
{
    resetParameters();
}//destructor

void XtermEscape::resetFSM(void)
{
    resetParameters();

    myState = NGXS_START;
    topScrolling = 0;
    bottomScrolling = 0;
    alternateBuffer = false;
    savedCursorX = 1;
    savedCursorY = 1;
    useTileNumber = false;
    tileNumber = 0;
    showedWindowMsg = false;
}//resetFSM

NGX_State XtermEscape::run(uint8_t currentByte)
{
    switch (myState)
    {
        case NGXS_START:
            resetParameters();
            runStart(currentByte);
            break;

        case NGXS_ERROR:
            myState = NGXS_START;
            resetParameters();
            runStart(currentByte);
            break;

        case NGXS_G0_CHAR_SET: runG0CharSet(currentByte);
            break;

        case NGXS_G1_CHAR_SET: runG1CharSet(currentByte);
            break;

        case NGXS_CSI: runCSI(currentByte);
            break;

        case NGXS_CSI_DEC: runCSIDEC(currentByte);
            break;

        case NGXS_CSI_PARAM: runCSIParam(currentByte);
            break;

        case NGXS_OSC: runOSC(currentByte);
            break;

        case NGXS_OSC_PARAM: runOSCParam(currentByte);
            break;

        default:
            cout << " XtermEscape::run(): encountered an unknown state!" << endl;
            myState = NGXS_ERROR;
            break;
    }//switch myState

    return myState;
}//run

//<ESC>
void XtermEscape::runStart(uint8_t currentByte)
{
    switch (currentByte)
    {
        //G0 Character Set
        case '(': myState = NGXS_G0_CHAR_SET;
            break;

        //G1 Character Set
        case ')': myState = NGXS_G1_CHAR_SET;
            break;

        //Control Sequence Introducer
        case '[': myState = NGXS_CSI;
            break;

        //Operating system command
        case ']':
            parameters.push_back(new QByteArray);
            myState = NGXS_OSC;
            break;

        //Save Cursor
        case '7':
            if (debugMessages)
                cout << " Xterm Save Cursor" << endl;
            savedCursorX = theWindow->getCursorX();
            savedCursorY = theWindow->getCursorY();
            myState = NGXS_START;
            break;

        //Restore Cursor
        case '8':
            if (debugMessages)
                cout << " Xterm Restore Cursor" << endl;
            theWindow->setCursorX(savedCursorX);
            theWindow->setCursorY(savedCursorY);
            myState = NGXS_START;
            break;

        //Application Keypad
        case '=':
            cout << " XtermEscape::runStart(): Xterm Application Keypad unhandled." << endl;
            myState = NGXS_START;
            break;

        //Normal Keypad
        case '>':
            cout << " XtermEscape::runStart(): Xterm Normal Keypad unhandled." << endl;
            myState = NGXS_START;
            break;

        //Reverse Index
        //case 'M':
            //cout << "XtermEscape::runStart(): Xterm Reverse Index unhandled." << endl;
            //myState = NGXS_START;
            //break;

        default:
            cout << " XtermEscape::runStart(): unknown byte: "
                 << static_cast<int>(currentByte) << endl;
            myState = NGXS_ERROR;
            break;
    }//switch currentByte
}//runStart

//ESC ]
void XtermEscape::runOSC(uint8_t currentByte)
{
    //### Find the type of OSC command ###
    if (isASCIIDigit(currentByte))
        parameters.at(parameters.size() - 1)->push_back(currentByte);

    //### Parameters are seperated by ; ###
    else if (currentByte == ';')
    {
        parameters.push_back(new QByteArray);
        myState = NGXS_OSC_PARAM;
    }//else if currentByte

    else
    {
        cout << " XtermEscape::runOSC(): unknown byte: "
                << static_cast<int>(currentByte) << endl;
        myState = NGXS_ERROR;
    }//else currentByte
}//runOSC

void XtermEscape::runOSCParam(uint8_t currentByte)
{
    if (currentByte == 7)//BEL
        handleOSC();

    else if (currentByte == 27)//ESC
    {
        cout << " XtermEscape::runOSCParam(): unhandled string terminator ESC" << endl;
        myState = NGXS_ERROR;
    }//else if currentByte

    else if (currentByte == ';')
        parameters.push_back(new QByteArray);

    else
        parameters.at(parameters.size() - 1)->push_back(currentByte);
}//runOSCParam

void XtermEscape::handleOSC(void)
{
    QByteArray *firstParam = NULL;//the first OSC parameter
    int oscType = 0;//the type of OSC command to run

    //### Verify that we received at least two parameters ###
    if (parameters.size() < 2)
    {
        cout << " XtermEscape::handleOSC(): expected two or more parameters" << endl;
        myState = NGXS_ERROR;
    }//if size()

    //### Extract the OSC command type ###
    if (myState != NGXS_ERROR)
    {
        firstParam = parameters.at(0);
        if (!extractInt(firstParam, oscType))
        {
            cout << " XtermEscape::handleOSC(): expected integer as first parameter" << endl;
            myState = NGXS_ERROR;
        }//if !extractInt()
    }//if myState

    if (myState != NGXS_ERROR)
    {
        switch (oscType)
        {
            //Change window title
            case 2:
                cout << " XtermEscape::handleOSC(): unhandled set window title to ";
                for (int i = 0; i < parameters.at(1)->size(); i++)
                    cout << parameters.at(1)->at(i);
                cout << endl;
                myState = NGXS_START;
            break;

            default:
                cout << " XtermEscape::handleOSC(): unknown control " << oscType << endl;
                myState = NGXS_ERROR;
            break;
        }//switch oscType
    }//if myState
}//handleOSC

//<ESC> <[>   (Called CSI)
void XtermEscape::runCSI(uint8_t currentByte)
{
    //DEC
    if (currentByte == '?')
        myState = NGXS_CSI_DEC;

    //parameter
    else if (isASCIIDigit(currentByte))
    {
        myState = NGXS_CSI_PARAM;
        runCSIParam(currentByte);
    }//else if isASCIIDigit()

    //Select Graphic Rendition (default)
    else if (currentByte == 'm')
        runSGRCharAttributes();

    //Set cursor position (default is 1,1)
    else if (currentByte == 'H')
        setCursorPosition();

    //Erase in Line
    else if (currentByte == 'K')
        eraseInLine();

    //Cursor Forward
    else if (currentByte == 'C')
        cursorForward();

    //Cursor Up
    else if (currentByte == 'A')
        cursorUp();

    //Erase in display
    else if (currentByte == 'J')
        eraseInDisplay();

    //Cursor Backward
    else if (currentByte == 'D')
        cursorBackward();

    //Delete lines
    else if (currentByte == 'M')
        deleteLines();

    //unknown
    else
    {
        cout << " XtermEscape::runCSI(): unknown byte: "
             << static_cast<int>(currentByte) << endl;
        myState = NGXS_ERROR;
    }//else currentByte
}//runCSI

////<ESC> <[>   (Called CSI, runCSIParam will be given an ASCII digit the first time it's called)
void XtermEscape::runCSIParam(uint8_t currentByte)
{
    //### Add received digits to the current parameter ###
    if (isASCIIDigit(currentByte))
    {
        if (parameters.size() == 0)
            parameters.push_back(new QByteArray);

        parameters.at(parameters.size() - 1)->push_back(currentByte);
    }//if isASCIIDigit()

    //### Parameters are seperated by ; ###
    else if (currentByte == ';')
        parameters.push_back(new QByteArray);

    //### Other chars are control codes ###
    else
    {
        switch (currentByte)
        {
            //Cursor Up Ps Times (default = 1))
            case 'A': cursorUp();
                break;

            //Cursor Down Ps Times (default = 1)
            case 'B': cursorDown();
                break;

            //Line Position Absolute
            case 'd': linePosition();
                break;

            //Cursor Character Absolute
            case 'G': cursorCharAbsolute();
                break;

            //Cursor position
            case 'H': cursorPosition();
                break;

            //Erase in Display
            case 'J': eraseInDisplay();
                break;

            //Erase in Line
            case 'K': eraseInLine();
                break;

            //Set Reset Mode
            case 'l': runCSIResetMode();
                break;

            //SGR Character Attributes
            case 'm': runSGRCharAttributes();
                break;

            //Delete Ps Character(s)
            case 'P': deleteCharacters();
                break;

            //Set scrolling region
            case 'r': runCSISetScrolling();
                break;

            //Erase Ps Character(s)
            case 'X': deleteCharacters();
                break;

            //Set Nethack Tile
            case 'z': setNethackTile();
                break;

            default:
                cout << " XtermEscape::runCSIParam(): unknown byte: "
                     << static_cast<int>(currentByte) << endl;
                myState = NGXS_ERROR;
                break;
        }//switch currentByte
    }//else isASCIIDigit
}//runCSIParam

void XtermEscape::cursorUp(void)
{
    int numPlaces = getCursorMoveDist();//the number of places to move the cursor

    if (myState != NGXS_ERROR)
    {
        if (debugMessages)
            cout << " Xterm Cursor Up: " << numPlaces << endl;

        theWindow->moveCursorY(numPlaces * -1);
        myState = NGXS_START;
    }//if myState
}//cursorUp

void XtermEscape::cursorDown(void)
{
    int numPlaces = getCursorMoveDist();//the number of places to move the cursor

    if (myState != NGXS_ERROR)
    {
        if (debugMessages)
            cout << " Xterm Cursor Down: " << numPlaces << endl;

        theWindow->moveCursorY(numPlaces);
        myState = NGXS_START;
    }//if myState
}//cursorDown

void XtermEscape::cursorForward(void)
{
    int numPlaces = getCursorMoveDist();//the number of places to move the cursor

    if (myState != NGXS_ERROR)
    {
        if (debugMessages)
            cout << " Xterm Cursor Forward: " << numPlaces << endl;

        theWindow->moveCursorX(numPlaces);
        myState = NGXS_START;
    }//if myState
}//cursorForward

void XtermEscape::cursorBackward(void)
{
    int numPlaces = getCursorMoveDist();//the number of places to move the cursor

    if (myState != NGXS_ERROR)
    {
        if (debugMessages)
            cout << " Xterm Cursor Backward: " << numPlaces << endl;

        theWindow->moveCursorX(numPlaces * -1);
        myState = NGXS_START;
    }//if myState
}//cursorBackward

int XtermEscape::getCursorMoveDist(void)
{
    QByteArray *oneParameter = NULL;//a single parameter from the list
    int result = 0;//the number of spaces to move

    //### No parameters means move one position ###
    if (parameters.size() == 0)
        result = 1;

    //### The movement distance is given in a parameter ###
    else if (parameters.size() == 1)
    {
        oneParameter = parameters.at(0);

        if (!extractInt(oneParameter, result))
        {
            cout << " XtermEscape::getCursorMoveDist(): couldn't convert "
                 << arrayToStr(oneParameter) << " to an integer" << endl;
            myState = NGXS_ERROR;
        }//if !extractInt
    }//else if size()

    //### Invalid number of parameters ###
    else
    {
        cout << "XtermEscape::getCursorMoveDist(): expected either 0 or 1 parameters" << endl;
        myState = NGXS_ERROR;
    }//else size()

    return result;
}//getCursorMoveDist

void XtermEscape::setNethackTile(void)
{
    MainWindow *mainWindow = whiteBoard->getMainWindow();
    GraphicsSettings *graphicsSettings = mainWindow->getGraphicsSettings();
    QByteArray *oneParameter = NULL;//a single parameter from the list
    int commandType = 0;//the type of Nethack tile command to execute
    bool serverTilesEnabled = graphicsSettings->serverTilesEnabled();

    //### We should have at least one parameter ###
    if (parameters.size() == 0)
    {
        cout << " XtermEscape::setNethackTile(): expected a parameter!" << endl;
        myState = NGXS_ERROR;
    }//if size()

    else
    {
        //extract the type of command
        oneParameter = parameters.at(0);
        if (!extractInt(oneParameter, commandType))
        {
            cout << " XtermEscape::setNethackTile(): couldn't convert "
                 << arrayToStr(oneParameter) << " to an integer" << endl;
            myState = NGXS_ERROR;
        }//if !extractInt()

        //We'll return to the starting state if we don't encounter an error
        myState = NGXS_START;

        //Run the specified command type
        if (serverTilesEnabled)
        {
            if (commandType == 0){
                startGlyph();

           } else if (commandType == 1){
                endGlyph();

            }else if (commandType == 2)
            {
                if (!showedWindowMsg)
                {
                    cout << "XtermEscape::setNethackTile(): Set window unhandled" << endl;
                    cout << "This message will only appear once" << endl;
                    showedWindowMsg = true;
                }//if !showedWindowMsg
            }//else if commandType
            else if (commandType == 3){
                //3 == server finished updating screen
                if( debugMessages){
                    cout << "server finished updating screen[Tiles] " << commandType << endl;
                }
                whiteBoard->getMainWindow()->alertTilesFinished();
            }
            else
            {
                cout << "XtermEscape::setNethackTile(): invalid command type " << commandType << endl;
                myState = NGXS_ERROR;
            }//else commandType
        }//if serverTilesEnabled
        else
            useTileNumber = false;
    }//else size()
}//setNethackTile

void XtermEscape::startGlyph(void)
{
    QByteArray *oneParameter = NULL;//a single parameter from the list

    //### Verify that a tile number was given ###
    if (parameters.size() != 2)
    {
        cout << "XtermEscape::startGlyph(): expected a tile number" << endl;
        myState = NGXS_ERROR;
    }//if size()

    //### Extract the tile number ###
    if (myState != NGXS_ERROR)
    {
        oneParameter = parameters.at(1);
        if (!extractInt(oneParameter, tileNumber))
        {
            cout << " XtermEscape::startGlyph(): couldn't convert "
                 << arrayToStr(oneParameter) << " to an integer" << endl;
            myState = NGXS_ERROR;
        }//if !extractInt()
    }//if myState

    //### Use the glyph ###
    if (myState != NGXS_ERROR)
    {
        if (debugMessages)
            cout << " Xterm Start Glyph: " << tileNumber << endl;

        useTileNumber = true;
    }//if myState
}//startGlyph

void XtermEscape::endGlyph(void)
{
    //### Verify that no parameter was given ###
    if (parameters.size() != 1)
    {
        cout << "XtermEscape::endGlyph(): got an unexpected parameter" << endl;
        myState = NGXS_ERROR;
    }//if size()

    useTileNumber = false;
}//endGlyph

void XtermEscape::deleteLines(void)
{
    QByteArray *oneParameter = NULL;//a single parameter from the list
    int numDelete = 0;//the number of lines to delete

    //### No parameters means delete 1 line ###
    if (parameters.size() == 0)
    {
        if (debugMessages)
            cout << " Xterm Delete Lines: 1" << endl;
        theWindow->deleteLines(1);
        myState = NGXS_START;
    }//if size()

    //### Delete the specified number of characters ###
    else if (parameters.size() == 1)
    {
        //extract the number of chars to delete
        oneParameter = parameters.at(0);
        if (!extractInt(oneParameter, numDelete))
        {
            cout << " XtermEscape::deleteLines(): couldn't convert "
                 << arrayToStr(oneParameter) << " to an integer" << endl;
            myState = NGXS_ERROR;
        }//if !extractInt()

        //perform the deletion
        if (myState != NGXS_ERROR)
        {
            if (debugMessages)
                cout << " Xterm Delete Lines: " << numDelete << endl;
            theWindow->deleteLines(numDelete);
            myState = NGXS_START;
        }//if myState
    }//else if size()

    //### Invalid number of parameters ###
    else
    {
        cout << " XtermEscape::deleteLines(): expected 0 or 1 parameters" << endl;
        myState = NGXS_ERROR;
    }//else size()
}//deleteLines

void XtermEscape::deleteCharacters(void)
{
    QByteArray *oneParameter = NULL;//a single parameter from the list
    int numDelete = 0;//the number of characters to delete

    //### No parameters means delete 1 char ###
    if (parameters.size() == 0)
    {
        if (debugMessages)
            cout << " Xterm Delete Characters: 1" << endl;
        theWindow->deleteCharacters(1);
        myState = NGXS_START;
    }//if size()

    //### Delete the specified number of characters ###
    else if (parameters.size() == 1)
    {
        //extract the number of chars to delete
        oneParameter = parameters.at(0);
        if (!extractInt(oneParameter, numDelete))
        {
            cout << " XtermEscape::deleteCharacters(): couldn't convert "
                 << arrayToStr(oneParameter) << " to an integer" << endl;
            myState = NGXS_ERROR;
        }//if !extractInt()

        //perform the deletion
        if (myState != NGXS_ERROR)
        {
            if (debugMessages)
                cout << " Xterm Delete Characters: " << numDelete << endl;
            theWindow->deleteCharacters(numDelete);
            myState = NGXS_START;
        }//if myState
    }//else if size()

    //### Invalid number of parameters ###
    else
    {
        cout << " XtermEscape::deleteCharacters(): expected 0 or 1 parameters" << endl;
        myState = NGXS_ERROR;
    }//else size()
}//deleteCharacters

void XtermEscape::cursorCharAbsolute(void)
{
    QByteArray *oneParameter = NULL;//a single parameter from the list
    int column = 0;//the column to move to

    //### No parameters means move to column 1 ###
    if (parameters.size() == 0)
    {
        if (debugMessages)
            cout << " Xterm Cursor Character Absolute 1" << endl;
        theWindow->setCursorX(1);
        myState = NGXS_START;
    }//if size()

    //### Move the cursor to the specified column ###
    else if (parameters.size() == 1)
    {
        //extract the column number
        oneParameter = parameters.at(0);
        if (!extractInt(oneParameter, column))
        {
            cout << " XtermEscape::cursorCharAbsolute(): couldn't convert "
                 << arrayToStr(oneParameter) << " to an integer" << endl;
            myState = NGXS_ERROR;
        }//if !intToStr()

        //move to that location
        if (myState != NGXS_ERROR)
        {
            if (debugMessages)
                cout << " Xterm Cursor Character Absolute " << column << endl;
            theWindow->setCursorX(column);
            myState = NGXS_START;
        }//if myState
    }//else if size()

    //### Invalid number of parameters ###
    else
    {
        cout << " XtermEscape::cursorCharAbsolute(): expected either 0 or 1 parameters" << endl;
        myState = NGXS_ERROR;
    }//else size()
}//cursorCharAbsolute

void XtermEscape::cursorPosition(void)
{
    QByteArray *oneParameter = NULL;//a single parameter from the list
    int row = 0;//the row position to move to
    int column = 0;//the column position to move to

    //### No parameters means move to 1,1 ###
    if (parameters.size() == 0)
    {
        if (debugMessages)
            cout << " Xterm Cursor Position 1,1" << endl;

        theWindow->setCursorX(1);
        theWindow->setCursorY(1);
        myState = NGXS_START;
    }//if size()

    //### Move to the specified location ###
    else if (parameters.size() == 2)
    {
        //extract the row
        oneParameter = parameters.at(0);
        if (!extractInt(oneParameter, row))
        {
            cout << " XtermEscape::cursorPosition(): couldn't convert "
                 << arrayToStr(oneParameter) << " to an integer" << endl;
            myState = NGXS_ERROR;
        }//if !intToStr()

        //extract the column
        oneParameter = parameters.at(1);
        if (myState != NGXS_ERROR)
        {
            if (!extractInt(oneParameter, column))
            {
                cout << " XtermEscape::cursorPosition(): couldn't convert "
                     << arrayToStr(oneParameter) << " to an integer" << endl;
                myState = NGXS_ERROR;
            }//if !intToStr()
        }//if myState

        //apply the change
        if (myState != NGXS_ERROR)
        {
            if (debugMessages)
                cout << " Xterm Cursor Position " << column << "," << row << endl;

            theWindow->setCursorX(column);
            theWindow->setCursorY(row);
            myState = NGXS_START;
        }//if myState
    }//else if size()

    //### Invalid number of parameters ###
    else
    {
        cout << " XtermEscape::cursorPosition(): expected either 0 or 2 parameters" << endl;
        myState = NGXS_ERROR;
    }//else size()
}//cursorPosition

void XtermEscape::eraseInLine(void)
{
    QByteArray *oneParameter = NULL;//a single parameter to examine

    //### No parameters means Erase to Right ###
    if (parameters.size() == 0)
    {
        if (debugMessages)
            cout << " Xterm Erase In Line: Erase To Right" << endl;
        theWindow->eraseToRight();
        myState = NGXS_START;
    }//if size()

    else if (parameters.size() == 1)
    {
        oneParameter = parameters.at(0);

        //### Erase to Right ###
        if (arrayToStr(oneParameter) == "0")
        {
            if (debugMessages)
                cout << " Xterm Erase In Line: Erase To Right" << endl;
            theWindow->eraseToRight();
            myState = NGXS_START;
        }//if getAsString

        else
        {
            cout << "XtermEscape::eraseInLine(): unknown parameter" << arrayToStr(oneParameter) << endl;
            myState = NGXS_ERROR;
        }//else getAsString();
    }//else if size()

    //### Only one parameter allowed ###
    else
    {
        cout << " XtermEscape::eraseInLine(): expecting at most one parameter." << endl;
        myState = NGXS_ERROR;
    }//else size()
}//eraseInLine

void XtermEscape::linePosition(void)
{
    QByteArray *oneParameter = NULL;//a single parameter to examine
    int row = 0;//the line position to move to

    //### Verify that we have at least one parameter ###
    if (parameters.size() == 0)
    {
        cout << " XtermEscape::linePosition(): no parameters given!" << endl;
        myState = NGXS_ERROR;
    }//if size()

    //### One parameter means row is given ###
    else if (parameters.size() == 1)
    {
        //### Extract the row position ###
        oneParameter = parameters.at(0);
        if (!extractInt(oneParameter, row))
        {
            cout << " XtermEscape::linePosition(): couldn't convert "
                 << arrayToStr(oneParameter) << " to an integer" << endl;
            myState = NGXS_ERROR;
        }//if !strToInt()

        //### Move to that location ###
        if (myState != NGXS_ERROR)
        {
            if (debugMessages)
                cout << " Xterm Line Position Absolute " << row << endl;
            theWindow->setCursorY(row);
            myState = NGXS_START;
        }//if myState
    }//else if size()

    //### Can have multiple parameters!? ###
    else
    {
        cout << " XtermEscape::linePosition(): expecting at most two parameters." << endl;
        myState = NGXS_ERROR;
    }//else size()
}//linePosition

void XtermEscape::eraseInDisplay(void)
{
    QByteArray *oneParameter = NULL;//a single parameter from the list

    //### No parameters means erase below ###
    if (parameters.size() == 0)
    {
        if (debugMessages)
            cout << " Xterm Erase In Display: Erase Below" << endl;
        theWindow->eraseBelow();
        myState = NGXS_START;
    }//if size()

    //### One parameter indicates an erasure type ###
    else if (parameters.size() == 1)
    {
        oneParameter = parameters.at(0);

        //### Erase Below ###
        if (arrayToStr(oneParameter) == "0")
        {
            if (debugMessages)
                cout << " Xterm Erase In Display: Erase Below" << endl;
            theWindow->eraseBelow();
            myState = NGXS_START;
        }//if getAsString()

        //### Erase All ###
        else if (arrayToStr(oneParameter) == "2")
        {
            if (debugMessages)
                cout << " Xterm Erase In Display: Erase All" << endl;


            theWindow->eraseAll();
            myState = NGXS_START;
        }//if getAsString()

        //### Unknown! ###
        else
        {
            cout << " XtermEscape::eraseInDisplay(): unknown parameter "
                << arrayToStr(oneParameter) << endl;
            myState = NGXS_ERROR;
        }//else getAsString()
    }//else size()

    //### Can have at most one parameter ###
    else
    {
        cout << " XtermEscape::eraseInDisplay(): expected at most 1 parameter" << endl;
        myState = NGXS_ERROR;
    }//else size()
}//eraseInDisplay

void XtermEscape::setCursorPosition(void)
{
    //### No parameters means move to 1,1 ###
    if (parameters.size() == 0)
    {
        if (debugMessages)
            cout << " Xterm cursor position 1,1" << endl;

        theWindow->setCursorX(1);
        theWindow->setCursorY(1);
        myState = NGXS_START;
    }//if size()

    else
    {
        cout << " XtermEscape::setCursorPosition(): no handler for parameters!" << endl;
        myState = NGXS_ERROR;
    }//else size()
}//setCursorPosition

void XtermEscape::runCSIResetMode(void)
{
    QByteArray *param1 = NULL;//the first parameter

    //### Verify that we got either 1 or 2 parameters ###
    if ((parameters.size() < 1) || (parameters.size() > 2))
    {
        cout << " XtermEscape::runCSIResetMode(): expected either 1 or 2 parameters" << endl;
        myState = NGXS_ERROR;
    }//if size() || size()

    //### We only handle the one-parameter version so far ###
    if (myState != NGXS_ERROR)
    {
        if (parameters.size() != 1)
        {
            cout << " XtermEscape::runCSIResetMode(): no handler for 2 parameters!" << endl;
            myState = NGXS_ERROR;
        }//if size()
    }//if myState

    //### Verify that the parameter is 1 byte long ###
    if (myState != NGXS_ERROR)
    {
        param1 = parameters.at(0);

        if (param1->size() != 1)
        {
            cout << " XtermEscape::runCSIResetMode(): expected a 1-byte parameter" << endl;
            myState = NGXS_ERROR;
        }//if size()
    }//if myState

    if (myState != NGXS_ERROR)
    {
        //Keyboard Action Mode (AM)
        if (param1->at(0) == '2')
        {
            cout << " XtermEscape::runCSIResetMode(): Reset Mode Keyboard Action Mode (AM) unhandled." << endl;
            myState = NGXS_START;
        }//if param1

        //Replace Mode (IRM)
        else if (param1->at(0) == '4')
        {
            cout << " XtermEscape::runCSIResetMode(): Reset Mode Keyboard Replace Mode (IRM) unhandled." << endl;
            myState = NGXS_START;
        }//else if param1

        //Unknown
        else
        {
            cout << " XtermEscape::runCSIResetMode(): Mode " << param1->at(0) << " unhandled." << endl;
            myState = NGXS_ERROR;
        }//else param1
    }//if myState
}//runCSIResetMode

void XtermEscape::runSGRCharAttributes(void)
{
    QByteArray *oneParameter = NULL;//the current parameter to examine
    unsigned int currentParam = 0;//index of the current parameter
    int intVersion = 0;//the integer version of the current parameter

    //### No parameter means use default mode ###
    if (parameters.size() == 0)
    {
        if (debugMessages)
            cout << " Xterm SGR Character Attributes Default" << endl;
        theWindow->setDefaultAttribute();
        myState = NGXS_START;
    }//if size()

    //### Apply the specified modes ###
    else
    {
        while ((currentParam < parameters.size()) && (myState != NGXS_ERROR))
        {
            //### Extract the current parameter ###
            oneParameter = parameters.at(currentParam);
            if (!extractInt(oneParameter, intVersion))
            {
                cout << " XtermEscape::runSGRCharAttributes(): couldn't convert "
                     << arrayToStr(oneParameter) << " to an int" << endl;
                myState = NGXS_ERROR;
            }//if !strToInt()

            //### Run the sub-FSM for char attributes ###
            if (myState != NGXS_ERROR)
            {
                if (!SGRAttribute::acceptAttribute(intVersion, theWindow, debugMessages))
                {
                    cout << " XtermEscape::runSGRCharAttributes(): unknown attribute!" << endl;
                    myState = NGXS_ERROR;
                }//if !acceptAttribute()
            }//if myState

            currentParam++;
        }//while currentParam && myState

        if (myState != NGXS_ERROR)
            myState = NGXS_START;
    }//else size()
}//runSGRCharAttributes

void XtermEscape::runCSISetScrolling(void)
{
    QByteArray *oneParameter = NULL;//the current parameter to examine

    //### Verify that we obtained two parameters ###
    if (parameters.size() != 2)
    {
        cout << " Xterm Set Scrolling Region: expected two parameters" << endl;
        myState = NGXS_ERROR;
    }//if size()

    //### Extract the first parameter ###
    if (myState != NGXS_ERROR)
    {
        oneParameter = parameters.at(0);
        if (!extractInt(oneParameter, topScrolling))
        {
            cout << " Xterm Set Scrolling Region: couldn't convert "
                 << arrayToStr(oneParameter) << " to an int" << endl;
            myState = NGXS_ERROR;
        }//if !strToInt()
    }//if myState

    //### Extract the second parameter ###
    if (myState != NGXS_ERROR)
    {
        oneParameter = parameters.at(1);
        if (!extractInt(oneParameter, bottomScrolling))
        {
            cout << " Xterm Set Scrolling Region: couldn't convert "
                 << arrayToStr(oneParameter) << " to an int" << endl;
            myState = NGXS_ERROR;
        }//if !strToInt()
    }//if myState

    //### Output the received message ###
    if (myState != NGXS_ERROR)
    {
        cout << " XtermEscape::runCSISetScrolling(): Set scrolling region top: " << topScrolling
             << " bottom: " << bottomScrolling << " unhandled." << endl;
        myState = NGXS_START;
    }//if myState
}//runCSISetScrolling

//<ESC> <[> <?>    (Called CSI DEC)
void XtermEscape::runCSIDEC(uint8_t currentByte)
{
    QByteArray *oneParameter = NULL;//a single parameter from the parameters deque
    unsigned int paramIndex = 0;//index of the parameter to consider
    int decSet = 0;//indicates the feature to set

    //### Add received digits to the current parameter ###
    if (isASCIIDigit(currentByte))
    {
        if (parameters.size() == 0)
            parameters.push_back(new QByteArray);

        parameters.at(parameters.size() - 1)->push_back(currentByte);
    }//if isASCIIDigit

    //### Parameters are seperated by ; ###
    else if (currentByte == ';')
        parameters.push_back(new QByteArray);

    //### Other characters are control codes ###
    else
    {
        switch (currentByte)
        {
            //Reset Mode
            case 'l':
                while ((paramIndex < parameters.size()) && (myState != NGXS_ERROR))
                {
                    oneParameter = parameters.at(paramIndex);
                    if (extractInt(oneParameter, decSet))
                        runDECPMReset(decSet);
                    else
                    {
                        cout << " XtermEscape::runCSIDEC(): Couldn't convert "
                             << arrayToStr(oneParameter) << " to int" << endl;
                        myState = NGXS_ERROR;
                    }//else strToInt()

                    paramIndex++;
                }//while paramIndex && myState

                if ((myState != NGXS_START) && (myState != NGXS_ERROR))
                {

                    cout << " XtermEscape::runCSIDEC(): DEC Private Mode Reset unhandled." << endl;
                    myState = NGXS_START;
                }//if myState && myState
                break;

            //DEC Private Mode Set
            case 'h':
                while ((paramIndex < parameters.size()) && (myState != NGXS_ERROR))
                {
                    oneParameter = parameters.at(paramIndex);
                    if (extractInt(oneParameter, decSet))
                        runDECPMSet(decSet);
                    else
                    {
                        cout << " XtermEscape::runCSIDEC(): Couldn't convert "
                             << arrayToStr(oneParameter) << " to int" << endl;
                        myState = NGXS_ERROR;
                    }//else strToInt()

                    paramIndex++;
                }//while paramIndex && myState

                if (myState != NGXS_START)
                {
                    cout << " XtermEscape::runCSIDEC(): didn't get a parameter!" << endl;
                    myState = NGXS_ERROR;
                }//if myState
                break;

            default:
                cout << " XtermEscape::runCSIDEC(): unknown byte: "
                     << static_cast<int>(currentByte) << endl;
                myState = NGXS_ERROR;
                break;
        }//switch currentByte
    }//else currentByte
}//runCSIDEC

void XtermEscape::runDECPMSet(int decSet)
{
    switch (decSet)
    {
        case 1:
            cout << " XtermEscape::runCECPMSet(): DEC Private Mode Set: Application Cursor Keys DECCKM unhandled." << endl;
            myState = NGXS_START;
            break;

        case 7:
            cout << " XtermEscape::runCECPMSet(): Xterm DEC Private Mode Set: Wraparound Mode (DECAWM) unhandled." << endl;
            myState = NGXS_START;
            break;

        case 47:
            if (debugMessages)
                cout << " Xterm DEC Private Mode Set: Use Alternate Screen Buffer, unless disabled by the titleInhibit resource." << endl;
            alternateBuffer = true;
            myState = NGXS_START;
            break;

        case 1047:
            if (debugMessages)
                cout << " Xterm DEC Private Mode Set: Use Alternate Screen Buffer" << endl;
            alternateBuffer = true;
            myState = NGXS_START;
            break;

        case 1048:
            cout << " XtermEscape::runCECPMSet(): Xterm DEC Private Mode Set: Save cursor as in DECSC untested." << endl;
            savedCursorX = theWindow->getCursorX();
            savedCursorY = theWindow->getCursorY();
            myState = NGXS_START;
            break;

        case 1049:
            if (debugMessages)
                cout << " Xterm DEC Private Mode Set: Save cursor as in DECSC and use Alternate Screen Buffer, clearing it first" << endl;

            savedCursorX = theWindow->getCursorX();
            savedCursorY = theWindow->getCursorY();

            theWindow->eraseAll();
            alternateBuffer = true;
            myState = NGXS_START;
            break;

        default:
            cout << " XtermEscape::runDECPMSet(): unknown DEC SET: " << decSet << endl;
            myState = NGXS_ERROR;
            break;
    }//switch decSet
}//runDECPMSet

void XtermEscape::runDECPMReset(int decSet)
{
    switch (decSet)
    {
        case 1:
            cout << " XtermEscape::runDECPMReset(): DEC Private Mode Reset: Normal Cursor Keys unhandled." << endl;
            myState = NGXS_START;
            break;

        case 47:
            if (debugMessages)
                cout << " Xterm DEC Private Mode Reset: Use Normal Screen Buffer" << endl;
            alternateBuffer = false;
            myState = NGXS_START;
            break;

        case 1047:
            if (debugMessages)
                cout << " Xterm DEC Private Mode Reset: Use Normal Screen Buffer" << endl;
            if (alternateBuffer)
                theWindow->eraseAll();
            alternateBuffer = false;
            myState = NGXS_START;
            break;

        case 1048:
            cout << " XtermEscape::runDECPMReset(): DEC Private Mode Reset: Restore cursor as in DECRC untested." << endl;

            theWindow->setCursorX(savedCursorX);
            theWindow->setCursorY(savedCursorY);
            myState = NGXS_START;
            break;

        case 1049:
            if (debugMessages)
                cout << " Xterm DEC Private Mode Reset: Use Normal Screen Buffer and restore cursor as in DECRC" << endl;

            if (alternateBuffer)
                theWindow->eraseAll();
            alternateBuffer = false;

            theWindow->setCursorX(savedCursorX);
            theWindow->setCursorY(savedCursorY);
            myState = NGXS_START;
            break;

        default:
            cout << " XtermEscape::runDECPMReset(): unknown DEC SET: " << decSet << endl;
            myState = NGXS_ERROR;
            break;
    }//switch decSet
}//runDECPMReset

void XtermEscape::runG0CharSet(uint8_t currentByte)
{
    switch (currentByte)
    {
        case 'B':
            cout << " XtermEscape::runG0CharSet(): Designate G0 Character Set United States (USASCII) unhandled." << endl;
            myState = NGXS_START;
            break;

        case '0':
            cout << " XtermEscape::runG0CharSet(): Designate G0 Character Set DEC Special Character and Line Drawing Set unhandled." << endl;
            myState = NGXS_START;
            break;

        default:
            cout << " XtermEscape::runG0CharSet(): unknown byte: "
                 << static_cast<int>(currentByte) << endl;
            myState = NGXS_ERROR;
            break;
    }//switch currentByte
}//runG0CharSet

void XtermEscape::runG1CharSet(uint8_t currentByte)
{
    switch (currentByte)
    {
        case '0':
            cout << " XtermEscape::runG1CharSet(): Designate G1 DEC Special Character and Line Drawing Set unhandled." << endl;
            myState = NGXS_START;
            break;

        default:
            cout << " XtermEscape::runG1CharSet(): unknown byte: "
                 << static_cast<int>(currentByte) << endl;
            myState = NGXS_ERROR;
            break;
    }//switch currentByte
}//runG1CharSet

bool XtermEscape::isASCIIDigit(uint8_t currentByte)
{
    bool result = false;//true if this is a digit from '0' to '9'

    if ((currentByte >= 48) && (currentByte <= 57))
        result = true;

    return result;
}//isASCIIDigit

string XtermEscape::arrayToStr(QByteArray *parameter)
{
    string result;//the string version to return

    for (int i = 0; i < parameter->size(); i++)
    {
        if (parameter->at(i) == 0)
            cout << " XtermEscape::arrayToStr(): got a NULL character." << endl;

        result.push_back(parameter->at(i));
    }//for i

    return result;
}//arrayToStr

bool XtermEscape::extractInt(QByteArray *oneParameter,
                             int &intVersion)
{
    bool result = true;//false if we couldn't convert to int

    intVersion = oneParameter->toInt(&result, 10);

    return result;
}//extractInt

void XtermEscape::resetParameters(void)
{
    for (unsigned int i = 0; i < parameters.size(); i++)
        delete parameters.at(i);
    parameters.clear();
}//resetParameters

bool XtermEscape::getUseTileNumber(void)
{
    return useTileNumber;
}//getUseTileNumber

int XtermEscape::getTileNumber(void)
{
    return tileNumber;
}//getTileNumber
