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

#include "NethackFX.hpp"
#include "TelnetProtocol.hpp"
#include "WhiteBoard.hpp"
#include "MainWindow.hpp"
#include "NetSprite.hpp"
#include "HistoryLog.hpp"
#include "ConfigWriter.hpp"

using namespace std;

NethackFX::NethackFX(WhiteBoard *newWhiteBoard)
{
    whiteBoard = newWhiteBoard;

    theWindow = NULL;
    unknownX = 0;
    unknownY = 0;
    myState = NGF_START;
    charHandler = new CharSaver;
    ruleHandler = new RuleLoader;
    spriteHandler = new ImageLoader(whiteBoard);
    userHistory = new HistoryLog(whiteBoard);
    graveBottom = ConfigWriter::loadString("game_config.txt", "Gravestone Bottom");

    userFX = false;
    lineRuleFX = false;
    showGraphics = false;
    mappingChar = false;
    secondLineFX = true;
}//constructor

NethackFX::~NethackFX(void)
{
    delete charHandler;
    charHandler = NULL;

    delete ruleHandler;
    ruleHandler = NULL;

    delete userHistory;
    userHistory = NULL;

    delete spriteHandler;
    spriteHandler = NULL;
}//destructor

bool NethackFX::initialize(TelnetWindow *newWindow)
{
    string pathAndName;//the path and filename of the current data file to load
    uint8_t width = 0;//window dimensions in characters
    uint8_t height = 0;
    bool result = true;//false on errors

    theWindow = newWindow;
    width = theWindow->getWidth();
    height = theWindow->getHeight();

    //### Verify that the dimensions aren't zero ###
    if ((width == 0) || (height == 0))
    {
        cout << "NethackFX: Window dimensions must be greater than zero" << endl;
        result = false;
    }//if newWidth || newHeight

    //### Load sprites from files ###
    if (result)
    {
        if (!spriteHandler->loadImages())
            result = false;
    }//if result

    //### Load known char-to-sprite mappings from file ###
    if (result)
    {
        pathAndName = NGSettings::DATA_PATH;
        pathAndName.append("knownchars.txt");
        if (!charHandler->load(pathAndName, knownChars))
            result = false;
    }//if result

    //### Load rules for enabling and disabling graphics mode ###
    if (result)
    {
        pathAndName = NGSettings::DATA_PATH;
        pathAndName.append("gfxrules.txt");

        if (!ruleHandler->load(pathAndName))
            result = false;
    }//if result

    return result;
}//initialize

void NethackFX::setGraphicsScene(QGraphicsScene *newScene)
{
    theScene = newScene;

    userHistory->setGraphicsScene(theScene);
}//setGraphicsScene

void NethackFX::resetHistoryLog(QGraphicsScene *newScene)
{
    if (theScene != NULL)
        userHistory->removeFromScene(theScene);

    delete userHistory;
    userHistory = new HistoryLog(whiteBoard);

    setGraphicsScene(newScene);
}//resetHistoryLog

void NethackFX::updateLogic(TelnetWindow *theWindow)
{
    MainWindow *mainWindow = whiteBoard->getMainWindow();
    string oldFirstLine;//checks if the contents of the first line changed

    if (theWindow->getDisplayChanged())
    {
        oldFirstLine.append(firstLine);

        //### Read the top line(s) of the telnet window ###
        firstLine.clear();
        for (int yPos = 0; yPos < FIRST_FX_ROW; yPos++)
        {
            for (unsigned int xPos = 0; xPos < theWindow->getWidth(); xPos++)
                firstLine.push_back(theWindow->getByte(xPos, yPos));
        }//for yPos

        //### Check the graphics rules for the first line of text ###
        if (oldFirstLine != firstLine)
        {
            if (ruleHandler->checkActions(firstLine, lineRuleFX))
            {
                //We found a rule to enable graphics, this should only happen at the welcome
                //message. Switch userFX to true. We've previously been blocking FX for the
                //login screen, now we'll enable them for the game.
                if (lineRuleFX)
                    mainWindow->setGraphicsMode(true);//sets userFX and toggles the checkbox
            }//if ruleHandler

            //If no rule is found, we default to true
            else
                lineRuleFX = true;

            setGraphicsMode();
        }//if oldFirstLine

        handleExtendedMore(theWindow);
        handleGravestone(theWindow);

        //### Add the first line to the user's history ###
        userHistory->updateLogic(firstLine, showGraphics);
        if (showGraphics)
            userHistory->show();
        else
            userHistory->hide();
    }//if getDisplayChanged
}//updateLogic

void NethackFX::handleGravestone(TelnetWindow *theWindow)
{
    MainWindow *mainWindow = whiteBoard->getMainWindow();
    string graveRow;//the bottom line of the gravestone from the telnet window

    for (unsigned int xPos = 0; xPos < theWindow->getWidth(); xPos++)
        graveRow.push_back(theWindow->getByte(xPos, GRAVE_BOTTOM_ROW));

    if (graveRow.find(graveBottom) != string::npos)
        mainWindow->setGraphicsMode(false);
}//handleGravestone

void NethackFX::handleExtendedMore(TelnetWindow *theWindow)
{
    string secondLine;//the second line from the telnet window
    bool showSecondFX = true;//true if showGraphics and secondLineFX are true
    bool oldSecondLine = secondLineFX;//see if secondLineFX changed

    //### Extract the second line ###
    for (unsigned int xPos = 0; xPos < theWindow->getWidth(); xPos++)
        secondLine.push_back(theWindow->getByte(xPos, FIRST_FX_ROW));

    //### Detect --More-- ###
    if (secondLine.find("--More--") == string::npos)
        secondLineFX = true;
    else
        secondLineFX = false;

    //### Toggle graphics if necessary ###
    if (oldSecondLine != secondLineFX)
    {
        if ((showGraphics) && (secondLineFX))
            showSecondFX = true;
        else
            showSecondFX = false;

        theWindow->setGraphicsMode(showSecondFX, FIRST_FX_ROW, 1);
    }//if oldSecondLine
}//handleExtendedMore

void NethackFX::updateQuery(void)
{
    TelnetProtocol *telnetPro = whiteBoard->getTelnetPro();

    queryServer();
    for (int i = 0; i < query.size(); i++)
        telnetPro->sendKeystroke(query.at(i));
    query.clear();
}//updateQuery

void NethackFX::setGraphicsMode(void)
{
    TelnetProtocol *telnetPro = whiteBoard->getTelnetPro();
    TelnetWindow *telnetWindow = telnetPro->getTelnetWindow();
    int fxHeight = 0;//the height of the FX region in telnet characters, minus one
    bool oldShowGraphics = showGraphics;//see if showGraphics changed
    bool showSecondFX = true;//true if showGraphics and secondLineFX are true

    //###The first FX line is toggled by handleExtendedMore ###
    fxHeight = FIRST_STATS_ROW - FIRST_FX_ROW - 1;

    //### Enable or disable graphics mode ###
    if (userFX)
    {
        if (lineRuleFX)
            showGraphics = true;
        else
            showGraphics = false;
    }//if userFX
    else
        showGraphics = false;

    //### Switch graphics modes if required ###
    if (showGraphics != oldShowGraphics)
    {
        if ((showGraphics) && (secondLineFX))
            showSecondFX = true;
        else
            showSecondFX = false;

        telnetWindow->setGraphicsMode(showSecondFX, FIRST_FX_ROW, 1);
        telnetWindow->setGraphicsMode(showGraphics, FIRST_FX_ROW + 1, fxHeight);

        if (showGraphics)
            userHistory->show();
        else
            userHistory->hide();
    }//if showGraphics
}//setGraphicsMode

void NethackFX::setUserFX(bool newUserFX)
{
    userFX = newUserFX;
    setGraphicsMode();
}//setFXMode

QPixmap* NethackFX::getImage(unsigned int index)
{
    QPixmap *result = NULL;//the image to return, or NULL if there is no image at index

    if (index < spriteHandler->numImages())
        result = spriteHandler->getImage(index);

    return result;
}//getImage

QPixmap* NethackFX::findImage(uint8_t telnetChar,
                              SGRAttribute *theAttributes)
{
    QPixmap *result = NULL;//the image to return
    NGS_Attribute charColor = NGSA_BLACK;//the color of this character
    map <string, int>::iterator knownIter;//points to an element in knownChars
    string theKey;//contains a character and color combination
    int imageIndex = 0;//index of this sprite in the sprite list

    //### Construct the key from the character and its color ###
    theKey.push_back(telnetChar);
    charColor = theAttributes->getForeground();
    theKey.push_back(static_cast<char>(charColor));

    //### Search for the graphic in knownChars ###
    knownIter = knownChars.find(theKey);
    if (knownIter != knownChars.end())
    {
        imageIndex = knownIter->second;

        if (imageIndex != NGSettings::TELNET_UNKNOWN)
            result = spriteHandler->getImage(imageIndex);
    }//if knownIter

    return result;
}//findImage

void NethackFX::sendWhatIs(int charX,
                           int charY,
                           uint8_t telnetChar,
                           SGRAttribute *theAttributes,
                           bool mapUnknownChar)
{
    string theKey;//contains a character and color combination
    map <string, int>::iterator knownIter;//points to an element in knownChars
    NGS_Attribute charColor = NGSA_BLACK;//the color of this character
    bool allOK = true;//false if we shouldn't send a query

    //### Construct a search key for this character ###
    theKey.push_back(telnetChar);
    charColor = theAttributes->getForeground();
    theKey.push_back(static_cast<char>(charColor));

    //### Verify that the user clicked in the game area ###
    if ((charY < FIRST_FX_ROW) || (charY >= FIRST_STATS_ROW))
        allOK = false;

    //### Verify that the user hasn't started a query manually ###
    if (allOK)
    {
        if (firstLine.find("Pick an object.") != string::npos)
        {
            QApplication::beep();
            allOK = false;
        }//if find()
    }//if allOK

    //### Verify that this character has no mapping ###
    if ((allOK) && (mapUnknownChar))
    {
        knownIter = knownChars.find(theKey);
        if (knownIter != knownChars.end())
        {
            if (knownIter->second == NGSettings::TELNET_UNKNOWN)
                whiteBoard->showMessage(QString("This character can't be mapped to any one image."));
            else
                whiteBoard->showMessage(QString("This character is already mapped to an image!"));

            allOK = false;
        }//if knownIter
    }//if allOK && mapUnknownChar

    //### Start the query ###
    if (allOK)
    {
        unknownX = charX;
        unknownY = charY;
        myState = NGF_SEND_QUERY;
        mappingChar = mapUnknownChar;
    }//if allOK
}//sendWhatIs

void NethackFX::queryServer(void)
{
    string theName;//the name for this tile, received from the server

    switch (myState)
    {
        case NGF_START:
            break;

        case NGF_SEND_QUERY:
            cout << "Sending ;" << endl;
            query.push_back(';');//what is there

            fsmTimer.start();
            myState = NGF_SEND_DIRECTIONS;
            break;

        case NGF_SEND_DIRECTIONS:
            if (firstLine.find("Pick an object.") != string::npos)
            {
                if (theWindow->getCursorY() != 1)
                {
                    sendDirections();
                    fsmTimer.start();
                    myState = NGF_SEND_WHATIS;
                }//if getCursorY()
            }//if find() || find()

            else if (fsmTimer.elapsed() >= REPLY_GRACETIME)
            {
                fsmTimer.start();
                myState = NGF_RESET;
                cout << "NethackFX::queryServer(): no reply, or couldn't understand it!" << endl;
            }//else if timeElapsed()
            break;

        case NGF_SEND_WHATIS:
            if (fsmTimer.elapsed() >= QUERY_TIME)
            {
                query.push_back('.');//what is this?

                fsmTimer.start();
                if (mappingChar)
                    myState = NGF_WAIT_DESCRIPTION;
                else
                    myState = NGF_RESET;
            }//if timeElapsed()
            break;

        case NGF_WAIT_DESCRIPTION:
            theName = extractName();
            if (theName.size() != 0)
            {
                cout << "Got a reply: " << theName << endl;
                addDescription(theName);

                fsmTimer.start();
                myState = NGF_RESET;
            }//if size()

            else if (fsmTimer.elapsed() > REPLY_GRACETIME)
            {
                fsmTimer.start();
                myState = NGF_RESET;
                cout << "NethackFX::queryServer(): no name, or couldn't understand it!" << endl;
                addDescription("NG_UNKNOWN");
            }//else if timeElapsed()
            break;

        case NGF_RESET:
            if (fsmTimer.elapsed() >= QUERY_TIME)
                myState = NGF_START;
            break;

        default:
            cout << "NethackFX::queryServer(): encountered an unknown state" << endl;
            throw 1;
            break;
    }//switch myState
}//queryServer

bool NethackFX::addDescription(const string &theName)
{
    NGS_Attribute charColor = NGSA_BLACK;//the color of this character
    string theKey;//the combination of character and color
    string partialName;//a part of theName, used to find a partial match
    char theChar = theWindow->getByte(unknownX, unknownY);//the char to add
    bool result = false;//true if we found a match for this name

    cout << "Got a description for char: " << theChar << endl;

    //### Construct the key from the character and its color ###
    theKey.push_back(theChar);
    charColor = theWindow->getAttributes(unknownX, unknownY)->getForeground();
    theKey.push_back(static_cast<char>(charColor));

    //### Find the sprite index based on a complete match ###
    if (addMapping(theKey, theName))
        result = true;

    //### Find a named monster ###
    if (!result)
    {
        partialName = checkSuffix(theName, " called ");
        if (addMapping(theKey, partialName))
            result = true;
    }//if !result

    //### Find a tame monster ###
    if (!result)
    {
        partialName = checkPrefix(theName, "tame ");
        if (addMapping(theKey, partialName))
            result = true;
    }//if !result

    //### Find a peaceful monster ###
    if (!result)
    {
        partialName = checkPrefix(theName, "peaceful ");
        if (addMapping(theKey, partialName))
            result = true;
    }//if !result

    //### Find an invisible monster ###
    if (!result)
    {
        partialName = checkPrefix(theName, "invisible ");
        if (addMapping(theKey, partialName))
            result = true;
    }//if !result

    //### Find an unidentified ring ###
    if (!result)
    {
        partialName = checkSuffix(theName, " ring");
        if (addMapping(theKey, partialName))
            result = true;
    }//if !result

    //### Update the display ###
    if (result)
        theWindow->notifyFxChange(unknownX, unknownY);
    else
    {
        cout << "Unknown sprite: " << theName << endl;
        whiteBoard->showMessage(QString("Couldn't find an image for that character."));
    }//if !result

    return result;
}//addDescription

bool NethackFX::addMapping(string theKey,
                           string theName)
{
    int spriteIndex = 0;//the corresponding graphic for this char
    bool result = true;//false if this name wasn't found

    //### Vefity that the name contains data ###
    if (theName.size() == 0)
        result = false;

    //### Find a sprite with this name ###
    if (result)
    {
        spriteIndex = spriteHandler->findImage(theName);
        if (spriteIndex == NGSettings::TELNET_UNKNOWN)
            result = false;
    }//if result

    //### Add this entry to the appropriate map ###
    if (result)
        knownChars.insert(pair <string, int>(theKey, spriteIndex));

    return result;
}//addMapping

string NethackFX::checkSuffix(const string &theName,
                              const string &theSuffix)
{
    string result;//the partial name to return, empty string if not found
    string temp;//the part of theName before " called "
    size_t nameIndex = 0;//index of a character in the name

    nameIndex = theName.find(theSuffix);
    if (nameIndex != string::npos)
    {
        temp.append(theName, 0, nameIndex);

        nameIndex = 0;
        while (nameIndex < temp.size())
        {
            if (temp.at(nameIndex) == ' ')
                result.clear();
            else
                result.push_back(temp.at(nameIndex));

            nameIndex++;
        }//while nameIndex
    }//if nameIndex

    return result;
}//checkSuffix

string NethackFX::checkPrefix(const string &theName,
                              const string &thePrefix)
{
    string result;//the partial name to return, empty string if not found
    string temp;//the part of theName after "tame "
    size_t nameIndex = 0;//index of a character in the name
    bool finished = false;//true if we've finished extracting the partial name

    nameIndex = theName.find(thePrefix);
    if (nameIndex != string::npos)
    {
        nameIndex += thePrefix.size();//skip the prefix

        //verify that there's more text after the prefix
        if (theName.size() > nameIndex)
        {
            temp.append(theName, nameIndex, theName.size() - nameIndex);

            //extract the word immediately after the prefix
            nameIndex = 0;
            while (!finished)
            {
                if (nameIndex >= temp.size())
                    finished = true;
                else if (temp.at(nameIndex) == ' ')
                    finished = true;
                else
                    result.push_back(temp.at(nameIndex));

                nameIndex++;
            }//while temp && nameIndex
        }//if size()
    }//if nameIndex

    return result;
}//checkPrefix

string NethackFX::extractName(void)
{
    string result;//the name we're extracting
    unsigned int linePos = 0;//our position in firstLine
    bool foundBracket = false;//true if we found an opening bracket
    bool finished = false;//true if we read in the name

    //### Scan until an opening bracket is found ###
    while ((linePos < firstLine.size()) && (!foundBracket))
    {
        if (firstLine.at(linePos) == '(')
            foundBracket = true;

        linePos++;
    }//while linePos && !foundBracket

    //### Read the name ###
    while ((linePos < firstLine.size()) && (!finished))
    {
        if (firstLine.at(linePos) == ')')
            finished = true;
        else
            result.push_back(firstLine.at(linePos));

        linePos++;
    }//while linePos && !finished

    //### Don't return anything until we get the whole name ###
    if (!finished)
        result.clear();

    return result;
}//extractName

void NethackFX::sendDirections(void)
{
    int moveX = 0;//the horizontal distance to move the cursor
    int moveY = 0;//vertical distance to move the cursor
    uint8_t startX = theWindow->getCursorX();
    uint8_t startY = theWindow->getCursorY();

    moveX = static_cast<int>(unknownX) - static_cast<int>(startX);
    moveY = static_cast<int>(unknownY) - static_cast<int>(startY);

    //### Move the cursor vertically ###
    if (!NGSettings::getUseNumpad())//verify that we have 8-tile movements
    {
        while (moveY > 8)
        {
            query.push_back('J');//down 8
            moveY -= 8;
        }//while moveY

        while (moveY < -8)
        {
            query.push_back('K');//up 8
            moveY += 8;
        }//while moveY
    }//if getDownKey()

    while (moveY > 0)
    {
        query.push_back(NGSettings::getDownKey());
        moveY--;
    }//while moveY

    while (moveY < 0)
    {
        query.push_back(NGSettings::getUpKey());
        moveY++;
    }//while moveY

    //### Move the cursor horizontally ###
    if (!NGSettings::getUseNumpad())//verify that we have 8-tile movements
    {
        while (moveX > 8)
        {
            query.push_back('L');//right 8
            moveX -= 8;
        }//while moveX

        while (moveX < -8)
        {
            query.push_back('H');//left 8
            moveX += 8;
        }//while moveX
    }//if getDownKey()

    while (moveX > 0)
    {
        query.push_back(NGSettings::getRightKey());
        moveX--;
    }//while moveX

    while (moveX < 0)
    {
        query.push_back(NGSettings::getLeftKey());
        moveX++;
    }//while moveX

    cout << "Sending ";
    for (int i = 0; i < query.size(); i++)
        cout << query.at(i);
    cout << endl;
}//sendDirections

void NethackFX::save(void)
{
    string pathAndName = NGSettings::DATA_PATH;

    pathAndName.append("knownchars.txt");

    charHandler->save(pathAndName, knownChars);
}//save

bool NethackFX::loadCustomTiles(QString filename)
{
    return spriteHandler->loadCustom(filename);
}//loadCustomTiles

bool NethackFX::loadDefaultTiles(void)
{
    MainWindow *mainWindow = whiteBoard->getMainWindow();
    bool result = true;//false on file access error

    delete spriteHandler;
    spriteHandler = new ImageLoader(whiteBoard);

    if (spriteHandler->loadImages())
    {
        NGSettings::setSpriteSize(ImageLoader::DEFAULT_SPRITE_SIZE, ImageLoader::DEFAULT_SPRITE_SIZE);
        mainWindow->resetGraphics();
    }
    else
        result = false;

    return result;
}//loadDefaultTiles
