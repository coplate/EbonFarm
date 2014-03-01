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

#include "CharSaver.hpp"

using namespace std;

CharSaver::CharSaver(void)
{
    colorNames.push_back("black");
    colorNames.push_back("red");
    colorNames.push_back("green");
    colorNames.push_back("yellow");
    colorNames.push_back("blue");
    colorNames.push_back("magenta");
    colorNames.push_back("cyan");
    colorNames.push_back("white");

    loadErrors = false;
}//constructor

CharSaver::~CharSaver(void)
{
}//destructor

bool CharSaver::load(const string &pathAndName,
                     map <string, int> &knownChars)
{
    ifstream infile(pathAndName.c_str());//the file to read from
    string oneLine;//a single line from the file
    bool finished = false;//true if we finished reading the file
    bool result = true;//false on file access error

    //### Verify that the file is open ###
    if (!infile.is_open())
    {
        cout << "CharSaver::load(): couldn't open file " << pathAndName << endl;
        result = false;
    }//if !is_open()

    //### Read the entries ###
    while ((result) && (!finished))
    {
        //### Read a line from the file ###
        getline(infile, oneLine);
        if (!infile.good())
        {
            if ((infile.eof()) && (oneLine == "END"))
                finished = true;
            else
            {
                cout << "Error reading from file " << pathAndName << endl;
                result = false;
            }//else eof() && oneLine
        }//if !good()

        //### Detect END if it's not the last line ###
        if ((result) && (!finished))
        {
            if (oneLine == "END")
                finished = true;
        }//if result && !finished

        //### Extract the information from this line ###
        if ((result) && (!finished))
        {
            if (!processLine(oneLine, knownChars))
                result = false;
        }//if result && !finished
    }//while result && !finished

    //### Close the file when finished ###
    if (infile.is_open())
        infile.close();

    //### Remember if we couldn't load ###
    if (!result)
        loadErrors = true;

    return result;
}//load

bool CharSaver::processLine(string &oneLine,
                            map <string, int> &knownChars)
{
    stringstream converter;//converts a line of text into different primitives
    string theColor;//the color of the telnet character
    string theKey;//key to add to knownChars
    string temp;//temporary copy of oneLine, used to shorten it
    unsigned int spriteIndex = 0;//the index of the associated graphic
    uint8_t theChar = 0;//the telnet character to map to a graphic
    uint8_t colorValue = 0;//integer value for this color, defined by SGRAttribute
    bool result = true;//false if the line was malformed

    //### Verify that the line contains data ###
    if (oneLine.size() == 0)
    {
        cout << "CharSaver::processLine(): found a line with no data" << endl;
        result = false;
    }//if size()

    //### Read the telnet character ###
    if (result)
    {
        theChar = oneLine.at(0);

        temp.append(oneLine, 1, oneLine.size() - 1);
        converter << temp;
    }//if result

    //### Read the color ###
    if (result)
    {
        converter >> theColor;
        if (!converter.good())
        {
            cout << "CharSaver::processLine(): error reading color from file" << endl;
            result = false;
        }//if !good()
    }//if result

    //### Convert this color to an integer ###
    if (result)
    {
        if (!findColor(theColor, colorValue))
        {
            cout << "CharSaver::processLine(): unknown color " << theColor << endl;
            result = false;
        }//if !findColor
    }//if result

    //### Read the sprite index ###
    if (result)
    {
        converter >> spriteIndex;
        if (converter.fail())
        {
            cout << "CharSaver::processLine(): error reading index from file" << endl;
            result = false;
        }//if fail()
    }//if result

    //### Add the character to the map ###
    if (result)
    {
        theKey.push_back(theChar);
        theKey.push_back(static_cast<char>(colorValue));

        knownChars.insert(pair <string, int>(theKey, spriteIndex));
    }//if result

    return result;
}//processLine

bool CharSaver::findColor(const string &theColor,
                          uint8_t &colorValue)
{
    unsigned int colorPos = 0;//index in colorNames
    bool result = false;//true if we found this color

    while ((colorPos < colorNames.size()) && (!result))
    {
        if (colorNames.at(colorPos) == theColor)
        {
            colorValue = colorPos;
            result = true;
        }//if colorNames

        colorPos++;
    }//while colorPos

    return result;
}//findColor

bool CharSaver::save(const string &pathAndName,
                     map <string, int> &knownChars)
{
    ofstream outfile;//the file to write to
    map<string, int>::iterator mapIter;//points to an element in knownChars
    string theKey;//a key from knownChars
    int colorIndex = 0;//index of the current color in colorNames
    char oneChar = ' ';//the current known character
    bool result = true;//false on file access error

    if (loadErrors)
        cout << "CharSaver::save(): not saving because there were errors loading." << endl;
    else
    {
        //### Open the file for writing ###
        outfile.open(pathAndName.c_str());
        if (!outfile.is_open())
        {
            cout << "Couldn't open " << pathAndName << " for writing" << endl;
            result = false;
        }//if !is_open()

        //### Save the mappings ###
        mapIter = knownChars.begin();
        while ((result) && (mapIter != knownChars.end()))
        {
            theKey = (*mapIter).first;

            //### Verify that the key is 2 bytes long ###
            if (theKey.size() != 2)
            {
                cout << "CharSaver::save(): got a key that isn't 2 bytes long" << endl;
                result = false;
            }//if size()

            //### Extract the character and color ###
            if (result)
            {
                oneChar = theKey.at(0);

                colorIndex = static_cast<int>(theKey.at(1));
                if ((colorIndex < 0) || (colorIndex >= static_cast<int>(colorNames.size())))
                {
                    cout << "CharSaver::save(): got an invalid color index " << colorIndex << endl;
                    result = false;
                }//if colorIndex || colorIndex
            }//if result

            //### Write them to file ###
            if (result)
            {
                outfile << oneChar << " " << colorNames.at(colorIndex)
                        << " " << (*mapIter).second << endl;
                if (!outfile.good())
                {
                    cout << "Error writing to " << pathAndName << endl;
                    result = false;
                }//if !good()
            }//if result

            mapIter++;
        }//while result && mapIter

        if (outfile.is_open())
        {
            outfile << "END";
            outfile.close();
        }//if is_open()
    }//else loadErrors

    return result;
}//save
