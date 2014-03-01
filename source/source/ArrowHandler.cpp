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

#include "ArrowHandler.hpp"
#include "TelnetProtocol.hpp"
#include "WhiteBoard.hpp"

using namespace std;

ArrowHandler::ArrowHandler(WhiteBoard *newWhiteBoard)
{
    whiteBoard = newWhiteBoard;
    bufferedArrow = false;
    prevArrow = 0;

    arrowTimer.setSingleShot(true);
    connect(&arrowTimer, SIGNAL(timeout()),
            this, SLOT(sendBufferedArrow()));
}//constructor

ArrowHandler::~ArrowHandler(void)
{
}//destructor

bool ArrowHandler::sendCommand(int keypress)
{
    TelnetProtocol *telnetPro = whiteBoard->getTelnetPro();
    bool result = false;//true if keypress is an arrow key

    if (bufferedArrow)
    {
        //### KEY_RIGHT ###
        if (keypress == Qt::Key_Right)
        {
            if (prevArrow == Qt::Key_Up)
            {
                telnetPro->sendKeystroke(NGSettings::getUpRightKey());
                bufferedArrow = false;
            }//if getScancode()
            else if (prevArrow == Qt::Key_Down)
            {
                telnetPro->sendKeystroke(NGSettings::getDownRightKey());
                bufferedArrow = false;
            }//if getScancode()
            else
            {
                sendBufferedArrow();
                newBufferedArrow(keypress);
            }//else getScancode()

            result = true;
        }//if keypress

        //### KEY_LEFT ###
        else if (keypress == Qt::Key_Left)
        {
            if (prevArrow == Qt::Key_Up)
            {
                telnetPro->sendKeystroke(NGSettings::getUpLeftKey());
                bufferedArrow = false;
            }//if getScancode()
            else if (prevArrow == Qt::Key_Down)
            {
                telnetPro->sendKeystroke(NGSettings::getDownLeftKey());
                bufferedArrow = false;
            }//if getScancode()
            else
            {
                sendBufferedArrow();
                newBufferedArrow(keypress);
            }//else getScancode()

            result = true;
        }//if keypress

        //### KEY_UP ###
        else if (keypress == Qt::Key_Up)
        {
            if (prevArrow == Qt::Key_Right)
            {
                telnetPro->sendKeystroke(NGSettings::getUpRightKey());
                bufferedArrow = false;
            }//if getScancode()
            else if (prevArrow == Qt::Key_Left)
            {
                telnetPro->sendKeystroke(NGSettings::getUpLeftKey());
                bufferedArrow = false;
            }//if getScancode()
            else
            {
                sendBufferedArrow();
                newBufferedArrow(keypress);
            }//else getScancode()

            result = true;
        }//if keypress

        //### KEY_DOWN ###
        else if (keypress == Qt::Key_Down)
        {
            if (prevArrow == Qt::Key_Right)
            {
                telnetPro->sendKeystroke(NGSettings::getDownRightKey());
                bufferedArrow = false;
            }//if getScancode()
            else if (prevArrow == Qt::Key_Left)
            {
                telnetPro->sendKeystroke(NGSettings::getDownLeftKey());
                bufferedArrow = false;
            }//if getScancode()
            else
            {
                sendBufferedArrow();
                newBufferedArrow(keypress);
            }//else getScancode()

            result = true;
        }//if keypress

        //### This is not an arrow key ###
        else
            sendBufferedArrow();
    }//if bufferedArrow

    //### No previous arrow is buffered ###
    else
    {
        if ((keypress == Qt::Key_Up)
            || (keypress == Qt::Key_Down)
            || (keypress == Qt::Key_Left)
            || (keypress == Qt::Key_Right))
        {
            newBufferedArrow(keypress);
            result = true;
        }//if keypress || keypress
    }//else bufferedArrow

    return result;
}//sendCommand


void ArrowHandler::newBufferedArrow(int newArrow)
{
    prevArrow = newArrow;
    bufferedArrow = true;
    arrowTimer.start(ARROW_TIME);
}//newBufferedArrow

void ArrowHandler::sendBufferedArrow(void)
{
    if (bufferedArrow)
    {
        TelnetProtocol *telnetPro = whiteBoard->getTelnetPro();

        switch (prevArrow)
        {
            case Qt::Key_Up: telnetPro->sendKeystroke(NGSettings::getUpKey());
                break;

            case Qt::Key_Down: telnetPro->sendKeystroke(NGSettings::getDownKey());
                break;

            case Qt::Key_Right: telnetPro->sendKeystroke(NGSettings::getRightKey());
                break;

            case Qt::Key_Left: telnetPro->sendKeystroke(NGSettings::getLeftKey());
                break;

            default:
                cout << "ArrowHandler::sendBufferedArrow(): prevArrow is not an arrow key!" << endl;
                throw 1;
                break;
        }//switch prevArrow

        arrowTimer.stop();
        bufferedArrow = false;
    }//if bufferedArrow
}//sendBufferedArrow
