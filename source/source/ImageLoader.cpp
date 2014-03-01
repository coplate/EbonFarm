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

#include "ImageLoader.hpp"
#include "WhiteBoard.hpp"
#include "MainWindow.hpp"
#include <QImage>
#include <QColor>
#include <QPainter>

using namespace std;

ImageLoader::ImageLoader(WhiteBoard *newWhiteBoard)
{
    whiteBoard = newWhiteBoard;
    dataLoaded = false;
}//constructor

ImageLoader::~ImageLoader(void)
{
    for (unsigned int i = 0; i < imageList.size(); i++)
        delete imageList.at(i);
    imageList.clear();
}//destructor

bool ImageLoader::loadImages(void)
{
    bool result = true;

    if (!dataLoaded)
    {
        //### Load monster sprites ###
        if (!loadFile("monsters.txt"))
            result = false;

        //### Load object sprites ###
        if (result)
        {
            if (!loadFile("objects.txt"))
                result = false;
        }//if result

        //### Load miscellaneous sprites ###
        if (result)
        {
            if (!loadFile("other.txt"))
                result = false;
        }//if result

        if (result)
            dataLoaded = true;
    }//if !dataLoaded

    return result;
}//loadSprites

bool ImageLoader::loadFile(const string &fileName)
{
    ifstream infile;//the sprite file to read
    map<char, QColor*>::iterator mapIter;//points to an element in fileColors
    string oneLine;//a single line from the file
    string pathAndName;//the path to the file, followed by the file name
    bool result = true;//false on loading error
    bool finished = false;//true if we've finished loading from this file

    pathAndName = NGSettings::DATA_PATH;
    pathAndName.append(fileName);

    //### Open the file for reading ###
    infile.open(pathAndName.c_str());
    if (!infile.good())
    {
        cout << "ImageLoader: Couldn't open " << pathAndName << " for reading"  << endl;
        result = false;
    }//if !open()

    //### Load the color scheme from file ###
    if (result)
    {
        if (!loadColors(infile))
            result = false;
    }//if result

    //### Load the tiles ###
    while ((result) && (!finished))
    {
        //### Read a line from file ###
        getline(infile, oneLine);
        if (!infile.good())
        {
            if (infile.eof())
                finished = true;
            else
            {
                cout << "ImageLoader: error reading from " << pathAndName << endl;
                result = false;
            }//else eof()
        }//if !good()

        //### Check for end of file ###
        if (result)
        {
            if (oneLine.size() == 0)
            {
                getline(infile, oneLine);
                if (infile.eof())
                    finished = true;
                else
                {
                    cout << "ImageLoader::loadFile(): expected tile name" << endl;
                    result = false;
                }//else eof()
            }//if size()
        }//if result

        //### Load this tile ###
        if ((result) && (!finished))
        {
            if (!loadTile(infile, oneLine))
                result = false;
        }//if result && !finished
    }//while result && !finished

    //### Close file ###
    if (infile.is_open())
        infile.close();

    return result;
}//loadFile

bool ImageLoader::loadTile(ifstream &infile,
                           string &header)
{
    QImage *oneImage = NULL;//a single image from the file
    string tileName;//descriptive name for this tile
    string oneLine;//a single line from the file
    unsigned int headerPos = 0;//index of the current character in header
    unsigned int yPos = 0;//y-coordinate to draw to
    bool result = true;//false on file access error

    //### Create the image ###
    oneImage = new QImage(DEFAULT_SPRITE_SIZE, DEFAULT_SPRITE_SIZE, QImage::Format_ARGB32);

    //### Scan to the opening bracket ###
    while ((headerPos < header.size()) && (header.at(headerPos) != '('))
        headerPos++;

    //### Read the sprite name ###
    headerPos++;
    while ((headerPos < header.size()) && (header.at(headerPos) != ')'))
    {
        tileName.push_back(header.at(headerPos));
        headerPos++;
    }//while headerPos && headerPos

    //### Verify that a name was found ###
    if (tileName.size() == 0)
    {
        cout << "ImageLoader::loadTile(): unable to find tile name" << endl;
        result = false;
    }//if size()

    //### Read the { line ###
    if (result)
    {
        getline(infile, oneLine);
        if (!infile.good())
        {
            cout << "ImageLoader::loadTile(): " << tileName << " expecting {" << endl;
            result = false;
        }//if !good()
    }//if result

    //### Verify that the line has exactly one char ###
    if (result)
    {
        if (oneLine.size() != 1)
        {
            cout << "ImageLoader::loadTile: " << tileName
                << " expecting { to be a line with one char" << endl;
            result = false;
        }//if size()
    }//if result

    //### Verify that it's actually a { ###
    if (result)
    {
        if (oneLine.at(0) != '{')
        {
            cout << "ImageLoader::loadTile: " << tileName
                << " expecting {, got " << oneLine << endl;
            result = false;
        }//if oneLine
    }//if result

    //### Load the graphic ###
    while ((result) && (yPos < DEFAULT_SPRITE_SIZE))
    {
        //### Read a line from the file ###
        getline(infile, oneLine);
        if (!infile.good())
        {
            whiteBoard->showMessage("ImageLoader::loadTile: " + QString::fromStdString(tileName)
                                    + " error reading from file");
            result = false;
        }//if !good()

        //### Load a line of pixels ###
        if (result)
        {
            if (loadGraphicsLine(oneLine, oneImage, yPos))
                yPos++;
            else
                result = false;
        }//if result
    }//while result

    //### Read the closing } ###
    if (result)
        getline(infile, oneLine);

    //### Store this image in the list ###
    if (result)
    {
        if (storeImage(tileName, oneImage))
            oneImage = NULL;
        else
            result = false;
    }//if result

    //### Delete the image if we ran into errors ###
    if (!result)
    {
        delete oneImage;
        oneImage = NULL;
    }//if !result

    return result;
}//loadTile

bool ImageLoader::storeImage(string &tileName,
                             QImage* oneImage)
{
    QPixmap *onePixmap = NULL;//pixmap version of oneImage
    string firstName;//the first name, if the sprite has two names
    string secondName;//the second name
    size_t nameIndex = 0;//index of a character in tileName
    bool result = true;//false if tileName was malformed

    //### Move the image to a pixmap format ###
    onePixmap = new QPixmap(QPixmap::fromImage(*oneImage, Qt::ColorOnly));
    delete oneImage;
    oneImage = NULL;

    //### Store sprites with a single name ###
    nameIndex = tileName.find(" / ");
    if (nameIndex == string::npos)
    {
        imageList.push_back(onePixmap);
        nameMap.insert(pair<string, unsigned int>(tileName, imageList.size() - 1));
    }//if find()

    //### Store sprites with a dual name ###
    else
    {
        firstName.append(tileName, 0, nameIndex);
        nameIndex += 3;//skip the " / "

        if (tileName.size() > nameIndex)
            secondName.append(tileName, nameIndex, tileName.size() - nameIndex);
        else
        {
            cout << "Error reading dual-named sprite: " << tileName << endl;
            result = false;
        }//else size()

        //### Verify that the names don't have 0 size ###
        if (result)
        {
            if ((firstName.size() == 0) || (secondName.size() == 0))
            {
                cout << "Missing name in dual-named sprite: " << tileName << endl;
                result = false;
            }//if size() || size()
        }//if result

        //### Add the sprites to the list ###
        if (result)
        {
            imageList.push_back(onePixmap);
            nameMap.insert(pair<string, unsigned int>(firstName, imageList.size() - 1));
            nameMap.insert(pair<string, unsigned int>(secondName, imageList.size() - 1));
        }//if result
    }//else find

    return result;
}//storeSprite

bool ImageLoader::loadGraphicsLine(string &oneLine,
                                   QImage *oneImage,
                                   int yPos)
{
    map<char, QRgb>::iterator mapIter;//points to an element in fileColors
    string temp;//temporary copy of oneLine, used to shorten it
    char currentChar= ' ';//current character (a pixel) read from file
    unsigned int xPos = 0;//index of the pixel to draw
    bool result = true;//false if oneLine was malformed

    //### Verify that the line is two spaces followed by the sprite ###
    if (oneLine.size() != DEFAULT_SPRITE_SIZE + 2)
    {
        cout << "ImageLoader::loadGraphicsLine(): expected a line 18 chars long" << endl;
        result = false;
    }//if size()

    //### Remove the leading spaces ###
    if (result)
    {
        temp.clear();
        temp.append(oneLine, 2, oneLine.size() - 2);
        oneLine = temp;
    }//if result

    //### Draw the line ###
    while ((result) && (xPos < oneLine.size()))
    {
        currentChar = oneLine.at(xPos);

        //### Find this color in the map ###
        mapIter = fileColors.find(currentChar);
        if (mapIter == fileColors.end())
        {
            cout << "ImageLoader::loadGraphicsLine(): unknown color " << currentChar << endl;
            result = false;
        }//if mapIter

        //### Draw the pixel ###
        if (result)
            oneImage->setPixel(xPos, yPos, mapIter->second);

        xPos++;
    }//while result && xPos

    return result;
}//loadGraphics

bool ImageLoader::loadColors(ifstream &infile)
{
    string oneLine;//a single line from the file
    bool result = true;//false on file access errors
    bool finishedColors = false;//true if we've finished reading color information

    fileColors.clear();

    while ((result) && (!finishedColors))
    {
        //### Read a line from the file ###
        getline(infile, oneLine);
        if (!infile.good())
        {
            cout << "ImageLoader::loadColors(): unexpected end of file" << endl;
            result = false;
        }//if !good()

        //### Verify that we read at least one character ###
        if (result)
        {
            if (oneLine.size() == 0)
            {
                cout << "ImageLoader::loadColors(): file line had no data" << endl;
                result = false;
            }//if size()
        }//if result

        //### Add the color to the map ###
        if (result)
        {
            if (oneLine.at(0) == '#')
            {
                finishedColors = true;

                if (!loadTile(infile, oneLine))
                    result = false;
            }//if oneLine
            else
            {
                if (!insertColor(oneLine))
                    result = false;
            }//else oneLine
        }//if result
  }//while result && !finishedColors

  return result;
}//loadColors

bool ImageLoader::insertColor(std::string &oneLine)
{
    string temp;//temporary copy of oneLine, used to shorten it
    stringstream converter;//converts strings into ints
    char colorName = ' ';//used to refer to this color in the file
    unsigned int redCol = 0;//the components of this color
    unsigned int greenCol = 0;
    unsigned int blueCol = 0;
    bool result = true;//false if the line was malformed

    //cout << "ImageLoader::insertColor(): added " << oneLine << endl;

    //### Verify that there are at least 6 chars ###
    if (oneLine.size() < 6)
    {
        cout << "ImageLoader::insertColor(): line with less than 6 chars" << endl;
        result = false;
    }//if size()

    //### Extract the color name ###
    if (result)
    {
        colorName = oneLine.at(0);
        if (fileColors.find(colorName) != fileColors.end())
        {
            cout << "ImageLoader::insertColor(): Color " << colorName
                 << " is present multiple times" << endl;
            result = false;
        }//if find()
    }//if result

    //### Strip everything but space-separated color values ###
    if (result)
    {
        temp.clear();
        temp.append(oneLine, 5, oneLine.size() - 6);

        oneLine.clear();
        for (unsigned int i = 0; i < temp.size(); i++)
        {
            if (temp.at(i) != ',')
                oneLine.push_back(temp.at(i));
        }//for i
    }//if result

    //### Extract the red color ###
    if (result)
    {
        converter << oneLine;

        converter >> redCol;
        if (!converter.good())
        {
            cout << "ImageLoader::insertColor(): invalid red color given" << endl;
            result = false;
        }//if !good()
    }//if result

    //### Extract the green color ###
    if (result)
    {
        converter >> greenCol;
        if (!converter.good())
        {
            cout << "ImageLoader::insertColor(): invalid green color given" << endl;
            result = false;
        }//if !good()
    }//if result

    //### Extract the blue color ###
    if (result)
    {
        converter >> blueCol;
        if (converter.fail())
        {
            cout << "ImageLoader::insertColor(): invalid blue color given" << endl;
            result = false;
        }//if fail()
    }//if result

    //### Insert the colors into the map ###
    if (result)
        fileColors.insert(pair<char, QRgb>(colorName, qRgb(redCol, greenCol, blueCol)));

    return result;
}//insertColor

bool ImageLoader::loadCustom(QString filename)
{
    MainWindow *mainWindow = whiteBoard->getMainWindow();
    QImage tileset;//all of the sprites from the custom tileset
    QImage oneTile;//a single tile from tileset
    QPainter tilePainter;//draws tiles from tileset onto oneTile
    int tileWidth = 0;//the size of each tile in pixels
    int tileHeight = 0;
    int tileX = 0;//the coordinates of the current tile to copy from tileset
    int tileY = 0;
    bool result = true;//false on file access error

    //### Load the tiles ###
    if (!tileset.load(filename, NULL))
    {
        whiteBoard->showMessage("Couldn't load " + filename);
        result = false;
    }//if !load()

    //### Verify that we have non-zero dimensions ###
    if (result)
    {
        if ((tileset.width() == 0) || (tileset.height() == 0))
        {
            whiteBoard->showMessage(filename + " has no pixels!");
            result = false;
        }//if width() || height()
    }//if result

    //### Calculate the tile dimensions ###
    if (result)
    {
        tileWidth = tileset.width() / CUSTOM_HORIZ_TILES;
        tileHeight = tileset.height() / CUSTOM_VERT_TILES;

        if ((tileWidth == 0) || (tileHeight == 0))
        {
            whiteBoard->showMessage(filename + " is too small to be a tileset!");
            result = false;
        }//if tileWidth || tileHeight
    }//if result

    //### Verify that the tile width is valid ###
    if (result)
    {
        if (tileWidth * CUSTOM_HORIZ_TILES != tileset.width())
        {
            whiteBoard->showMessage(filename + " has tiles that aren't all the same width!");
            result = false;
        }//if tileWidth * CUSTOM_HORIZ_TILES
    }//if result

    //### Verify that the tile height is valid ###
    if (result)
    {
        if (tileHeight * CUSTOM_VERT_TILES != tileset.height())
        {
            whiteBoard->showMessage(filename + " has tiles that aren't all the same height!");
            result = false;
        }//if tileHeight * CUSTOM_HORIZ_TILES
    }//if result

    //### Create a drawing buffer and initialize drawing ###
    if (result)
    {
        oneTile = QImage(tileWidth, tileHeight, QImage::Format_RGB32);

        if (!tilePainter.begin(&oneTile))
        {
            whiteBoard->showMessage("ImageLoader::loadCustom(): couldn't draw tiles!");
            result = false;
        }//if !begin()
    }//if result

    //### Draw the tiles into imageList ###
    if (result)
    {
        for (unsigned int i = 0; i < imageList.size(); i++)
        {
            tilePainter.drawImage(0, 0, tileset, tileX * tileWidth, tileY * tileHeight,
                                  tileWidth, tileHeight, Qt::AutoColor);

            delete imageList.at(i);
            imageList.at(i) = new QPixmap(QPixmap::fromImage(oneTile));

            tileX++;
            if (tileX >= CUSTOM_HORIZ_TILES)
            {
                tileX = 0;
                tileY++;
            }//if tileX
        }//for i

        tilePainter.end();
    }//if result

    //### Update the display ###
    if (result)
    {
        NGSettings::setSpriteSize(tileWidth, tileHeight);
        mainWindow->resetGraphics();
    }

    return result;
}//loadCustom

QPixmap* ImageLoader::getImage(unsigned int index)
{
    return imageList.at(index);
}//getSprite

unsigned int ImageLoader::numImages(void)
{
    return imageList.size();
}//numImages

int ImageLoader::findImage(const string &spriteName)
{
    map <string, unsigned int>::iterator mapIter;//points to an element in nameMap
    int result = 0;//index of the graphic, or NG_UNKNOWN if not found

    mapIter = nameMap.find(spriteName);
    if (mapIter == nameMap.end())
        result = NGSettings::TELNET_UNKNOWN;
    else
        result = (*mapIter).second;

  return result;
}//findImage
