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

#include "NGSettings.hpp"
#include "ImageLoader.hpp"

using namespace std;

QFont* NGSettings::telnetFont;

const string NGSettings::DATA_PATH = "data/";

unsigned int NGSettings::spriteWidth = ImageLoader::DEFAULT_SPRITE_SIZE;
unsigned int NGSettings::spriteHeight = ImageLoader::DEFAULT_SPRITE_SIZE;
unsigned int NGSettings::historyLines = 0;

int NGSettings::fontOffset = 0;

char NGSettings::upLeftKey = ' ';
char NGSettings::upKey = ' ';
char NGSettings::upRightKey = ' ';
char NGSettings::rightKey = ' ';
char NGSettings::downLeftKey = ' ';
char NGSettings::downKey = ' ';
char NGSettings::downRightKey = ' ';
char NGSettings::leftKey = ' ';
bool NGSettings::useNumpad = true;

NGSettings::NGSettings(void)
{
    //### Set up the telnet font ###
    telnetFont = new QFont;
    updateFontFromConfig();

    //### Set up the keypad ###
    useNumpad = ConfigWriter::loadBool("game_config.txt", "Numpad Keys");
    setKeys();

    reloadHistoryLines();
}//constructor

NGSettings::~NGSettings(void)
{
    delete telnetFont;
    telnetFont = NULL;
}//destructor

void NGSettings::setSpriteSize(unsigned int newWidth,
                               unsigned int newHeight)
{
    if ((newWidth == 0) || (newHeight == 0))
        throw 1;

    spriteWidth = newWidth;
    spriteHeight = newHeight;
}

void NGSettings::updateFontFromConfig()
{
    string fontName;//The font family to use
    int fontSize = 0;//the size of the font in points
    bool useCustomFont = ConfigWriter::loadBool("game_config.txt", "Use Custom Font");

    if (useCustomFont)
    {
        fontName = ConfigWriter::loadString("game_config.txt", "Custom Font Name");
        fontSize = ConfigWriter::loadInt("game_config.txt", "Custom Font Size");

        telnetFont->setPointSize(fontSize);
    }
    else
        fontName = ConfigWriter::loadString("game_config.txt", "Font Name");

    telnetFont->setFamily(QString::fromStdString(fontName));
    setFontSize();
}

void NGSettings::setFontSize()
{
    int minSpriteDimension = 0;//the sprite height or width, whichever is smaller
    int fontSize = 0;//the size of the font in points
    bool useCustomFont = ConfigWriter::loadBool("game_config.txt", "Use Custom Font");

    if (spriteWidth < spriteHeight)
        minSpriteDimension = spriteWidth;
    else
        minSpriteDimension = spriteHeight;

    if (minSpriteDimension >= 32)
	{
		fontOffset = ConfigWriter::loadInt("game_config.txt", "Font Offset Large");
        fontSize = ConfigWriter::loadInt("game_config.txt", "Font Size 32");
	}

    else if (minSpriteDimension >= 24)
	{
		fontOffset = ConfigWriter::loadInt("game_config.txt", "Font Offset Large");
        fontSize = ConfigWriter::loadInt("game_config.txt", "Font Size 24");
	}

    else if (minSpriteDimension >= 20)
	{
		fontOffset = ConfigWriter::loadInt("game_config.txt", "Font Offset Large");
        fontSize = ConfigWriter::loadInt("game_config.txt", "Font Size 20");
	}

    else if (minSpriteDimension >= 16)
	{
		fontOffset = ConfigWriter::loadInt("game_config.txt", "Font Offset 16");
        fontSize = ConfigWriter::loadInt("game_config.txt", "Font Size 16");
	}

	else if (minSpriteDimension >= 12)
    {
        fontOffset = ConfigWriter::loadInt("game_config.txt", "Font Offset 16");
        fontSize = ConfigWriter::loadInt("game_config.txt", "Font Size 12");
    }

    else
    {
        fontOffset = ConfigWriter::loadInt("game_config.txt", "Font Offset 16");
        fontSize = ConfigWriter::loadInt("game_config.txt", "Font Size Tiny");
    }

    if (!useCustomFont)
    {
        //We can make the font a tad larger for tall, narrow tilesets
        if (spriteHeight >= spriteWidth * 1.5)
            fontSize += static_cast<int>((static_cast<float>(fontSize) * 0.2f));

        telnetFont->setPointSize(fontSize);
    }
}//setFontSize

void NGSettings::setKeys(void)
{
    if (useNumpad)
    {
        upLeftKey = '7';
        upKey = '8';
        upRightKey = '9';
        rightKey = '6';
        downRightKey = '3';
        downKey = '2';
        downLeftKey = '1';
        leftKey = '4';
    }
    else
    {
        upLeftKey = 'y';
        upKey = 'k';
        upRightKey = 'u';
        rightKey = 'l';
        downRightKey = 'n';
        downKey = 'j';
        downLeftKey = 'b';
        leftKey = 'h';
    }
}//setKeys

QFont* NGSettings::getTelnetFont(void)
{
    return telnetFont;
}//getTelnetFont

char NGSettings::getUpLeftKey(void)
{
    return upLeftKey;
}//getUpLeftKey

char NGSettings::getUpKey(void)
{
    return upKey;
}//getUpKey

char NGSettings::getUpRightKey(void)
{
    return upRightKey;
}//getUpRightKey

char NGSettings::getRightKey(void)
{
    return rightKey;
}//getRightKey

char NGSettings::getDownRightKey(void)
{
    return downRightKey;
}//getDownRightKey

char NGSettings::getDownKey(void)
{
    return downKey;
}//getDownKey

char NGSettings::getDownLeftKey(void)
{
    return downLeftKey;
}//getDownLeftKey

char NGSettings::getLeftKey(void)
{
    return leftKey;
}//getLeftKey

bool NGSettings::getUseNumpad(void)
{
    return useNumpad;
}//getUseNumpad

int NGSettings::getFontOffset(void)
{
    return fontOffset;
}//getFontOffset

unsigned int NGSettings::getSpriteWidth(void)
{
    return spriteWidth;
}

unsigned int NGSettings::getSpriteHeight(void)
{
    return spriteHeight;
}

void NGSettings::reloadHistoryLines(void)
{
    historyLines = ConfigWriter::loadInt("game_config.txt", "History Size");
}

unsigned int NGSettings::getHistoryLines(void)
{
    return historyLines;
}
