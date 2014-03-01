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

#include "SGRAttribute.hpp"
#include "TelnetWindow.hpp"

using namespace std;

SGRAttribute::SGRAttribute(void)
{
    setDefault();
}//constructor

SGRAttribute::~SGRAttribute(void)
{
}//destructor

void SGRAttribute::setDefault(void)
{
    bold = false;
    underlined = false;
    inverse = false;
    invisible = false;

    foregroundColor = NGSA_WHITE;
    backgroundColor = NGSA_BLACK;
}//setDefault

void SGRAttribute::setValues(SGRAttribute *source)
{
    bold = source->bold;
    inverse = source->inverse;
    invisible = source->invisible;
    underlined = source->underlined;
    foregroundColor = source->foregroundColor;
    backgroundColor = source->backgroundColor;
}//setValues

void SGRAttribute::setBold(void)
{
    bold = true;
}//setBold

void SGRAttribute::setInverse(void)
{
    inverse = true;
}//setInverse

void SGRAttribute::setInvisible(void)
{
    invisible = true;
}//setInvisible

void SGRAttribute::setUnderlined(void)
{
    underlined = true;
}//setUnderlined

void SGRAttribute::clearBold(void)
{
    bold = false;
}//clearBold

void SGRAttribute::clearInverse(void)
{
    inverse = false;
}//clearInverse

void SGRAttribute::clearInvisible(void)
{
    invisible = false;
}//clearInvisible

void SGRAttribute::clearUnderlined(void)
{
    underlined = false;
}//clearUnderlined

void SGRAttribute::setForeground(NGS_Attribute color)
{
    foregroundColor = color;
}//setForeground

void SGRAttribute::setBackground(NGS_Attribute color)
{
    backgroundColor = color;
}//setBackground

bool SGRAttribute::getBold(void)
{
    return bold;
}//getBold

bool SGRAttribute::getInverse(void)
{
    return inverse;
}//getInverse

bool SGRAttribute::getInvisible(void)
{
    return invisible;
}//getInvisible

bool SGRAttribute::getUnderlined(void)
{
    return underlined;
}//getUnderlined

NGS_Attribute SGRAttribute::getForeground(void)
{
    return foregroundColor;
}//getForeground

NGS_Attribute SGRAttribute::getBackground(void)
{
    return backgroundColor;
}//getBackground

bool SGRAttribute::acceptAttribute(int parameter,
                                   TelnetWindow *theWindow,
                                   bool debugMessages)
{
    bool result = true;

    switch (parameter)
    {
        case NGC_NORMAL:
            if (debugMessages)
                cout << " Xterm SGR Character Attribute: Normal" << endl;
            theWindow->setDefaultAttribute();
            break;

        case NGC_BOLD:
            if (debugMessages)
                cout << " Xterm SGR Character Attribute: Bold" << endl;
            theWindow->setBold();
            break;

        case NGC_UNDERLINED:
            if (debugMessages)
                cout << " Xterm SGR Character Attribute: Underlined" << endl;
            theWindow->setUnderlined();
            break;

        case NGC_BLINK:
            if (debugMessages)
                cout << " Xterm SGR Character Attribute: Blink" << endl;
            theWindow->setBold();
            break;

        case NGC_INVERSE:
            if (debugMessages)
                cout << " Xterm SGR Character Attribute: Inverse" << endl;
            theWindow->setInverse();
            break;

        case NGC_INVISIBLE:
            if (debugMessages)
                cout << " Xterm SGR Character Attribute: Invisible" << endl;
            theWindow->setInvisible();
            break;

        case NGC_NORMAL2:
            if (debugMessages)
                cout << " Xterm SGR Character Attribute: Normal (2)" << endl;
            theWindow->clearBold();
            break;

        case NGC_NOT_UNDERLINED:
            if (debugMessages)
                cout << " Xterm SGR Character Attribute: Underlined" << endl;
            theWindow->clearUnderlined();
            break;

        case NGC_STEADY:
            if (debugMessages)
                cout << " Xterm SGR Character Attribute: Steady" << endl;
            theWindow->clearBold();
            break;

        case NGC_POSITIVE:
            if (debugMessages)
                cout << " Xterm SGR Character Attribute: Positive" << endl;
            theWindow->clearInverse();
            break;

        case NGC_VISIBLE:
            if (debugMessages)
                cout << " Xterm SGR Character Attribute: Visible" << endl;
            theWindow->clearInvisible();
            break;

        case NGC_FOREGROUND_BLACK:
            if (debugMessages)
                cout << " Xterm SGR Character Attribute: Foreground Black" << endl;
            theWindow->setForeground(NGSA_BLACK);
            break;

        case NGC_FOREGROUND_RED:
            if (debugMessages)
                cout << " Xterm SGR Character Attribute: Foreground Red" << endl;
            theWindow->setForeground(NGSA_RED);
            break;

        case NGC_FOREGROUND_GREEN:
            if (debugMessages)
                cout << " Xterm SGR Character Attribute: Foreground Green" << endl;
            theWindow->setForeground(NGSA_GREEN);
            break;

        case NGC_FOREGROUND_YELLOW:
            if (debugMessages)
                cout << " Xterm SGR Character Attribute: Foreground Yellow" << endl;
            theWindow->setForeground(NGSA_YELLOW);
            break;

        case NGC_FOREGROUND_BLUE:
            if (debugMessages)
                cout << " Xterm SGR Character Attribute: Foreground Blue" << endl;
            theWindow->setForeground(NGSA_BLUE);
            break;

        case NGC_FOREGROUND_MAGENTA:
            if (debugMessages)
                cout << " Xterm SGR Character Attribute: Foreground Magenta" << endl;
            theWindow->setForeground(NGSA_MAGENTA);
            break;

        case NGC_FOREGROUND_CYAN:
            if (debugMessages)
                cout << " Xterm SGR Character Attribute: Foreground Cyan" << endl;
            theWindow->setForeground(NGSA_CYAN);
            break;

        case NGC_FOREGROUND_WHITE:
            if (debugMessages)
                cout << " Xterm SGR Character Attribute: Foreground White" << endl;
            theWindow->setForeground(NGSA_WHITE);
            break;

        case NGC_FOREGROUND_DEFAULT:
            if (debugMessages)
                cout << " Xterm SGR Character Attribute: Foreground Default" << endl;
            theWindow->setForeground(NGSA_WHITE);
            break;

        case NGC_BACKGROUND_BLACK:
            if (debugMessages)
                cout << " Xterm SGR Character Attribute: Background Black" << endl;
            theWindow->setBackground(NGSA_BLACK);
            break;

        case NGC_BACKGROUND_RED:
            if (debugMessages)
                cout << " Xterm SGR Character Attribute: Background Red" << endl;
            theWindow->setBackground(NGSA_RED);
            break;

        case NGC_BACKGROUND_GREEN:
            if (debugMessages)
                cout << " Xterm SGR Character Attribute: Background Green" << endl;
            theWindow->setBackground(NGSA_GREEN);
            break;

        case NGC_BACKGROUND_YELLOW:
            if (debugMessages)
                cout << " Xterm SGR Character Attribute: Background Yellow" << endl;
            theWindow->setBackground(NGSA_YELLOW);
            break;

        case NGC_BACKGROUND_BLUE:
            if (debugMessages)
                cout << " Xterm SGR Character Attribute: Background Blue" << endl;
            theWindow->setBackground(NGSA_BLUE);
            break;

        case NGC_BACKGROUND_MAGENTA:
            if (debugMessages)
                cout << " Xterm SGR Character Attribute: Background Magenta" << endl;
            theWindow->setBackground(NGSA_MAGENTA);
            break;

        case NGC_BACKGROUND_CYAN:
            if (debugMessages)
                cout << " Xterm SGR Character Attribute: Background Cyan" << endl;
            theWindow->setBackground(NGSA_CYAN);
            break;

        case NGC_BACKGROUND_WHITE:
            if (debugMessages)
                cout << " Xterm SGR Character Attribute: Background White" << endl;
            theWindow->setBackground(NGSA_WHITE);
            break;

        case NGC_BACKGROUND_DEFAULT:
            if (debugMessages)
                cout << " Xterm SGR Character Attribute: Background Default" << endl;
            theWindow->setBackground(NGSA_WHITE);
            break;

        default:
            cout << "CharAttributeFSM::acceptAttribute(): unknown parameter " << parameter << endl;
            result = false;
            break;
    }//switch parameter

    return result;
}//acceptAttribute
