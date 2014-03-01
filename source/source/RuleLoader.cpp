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

#include "RuleLoader.hpp"

using namespace std;

RuleLoader::RuleLoader(void)
{
    currentRule = NULL;
    resetFSM();
}//constructor

RuleLoader::~RuleLoader(void)
{
    for (unsigned int i = 0; i < theRules.size(); i++)
        delete theRules.at(i);
    theRules.clear();
}//destructor

void RuleLoader::resetFSM(void)
{
    lineNumber = 0;
    myState = NGRS_NEW_ENTRY;

    delete currentRule;
    currentRule = new FXRule;
}//resetFSM

bool RuleLoader::load(const string &fileName)
{
    ifstream infile(fileName.c_str());//the file to read from
    string oneLine;//a single line from the file
    bool finished = false;//true if we've finished reading from the file
    bool result = true;//false on file access errors

    resetFSM();

    //### Verify that the file is open ###
    if (!infile.good())
    {
        cout << "RuleLoader: couldn't open " << fileName << " for reading" << endl;
        result = false;
    }//if !good()

    while ((result) && (!finished))
    {
        //### Read a line from the file ###
        getline(infile, oneLine);
        if (!infile.good())
        {
            if (infile.eof())
                finished = true;
            else
            {
                cout << "RuleLoader: error reading from " << fileName << endl;
                result = false;
            }//else eof()
        }//if !good()

        if ((result) && (!finished))
        {
            lineNumber++;
            if (!processLine(oneLine))
                result = false;
        }//if result && !finished
    }//while result && !finished

    if (infile.is_open())
        infile.close();

    return result;
}//load

bool RuleLoader::processLine(string &oneLine)
{
    string parameter;
    string theCommand;
    bool result = true;//false if oneLine is malformed

    switch (myState)
    {
        case NGRS_START:
            if (oneLine.size() == 0)
            {
                if (currentRule == NULL)
                {
                    currentRule = new FXRule;
                    myState = NGRS_NEW_ENTRY;
                }//if currentRule
                else
                {
                    cout << "RuleLoader::processLine(): currentRule != NULL" << endl;
                    result = false;
                }//else currentRule
            }//if size()
            else
            {
                cout << "RuleLoader: expected blank line" << endl;
                result = false;
            }//else size()
            break;

        case NGRS_NEW_ENTRY:
            if (getCommand(oneLine, theCommand, parameter))
            {
                if (currentRule->storeCondition(theCommand, parameter))
                    myState = NGRS_GOT_CONDITION;
                else
                {
                    cout << "RuleLoader: error in condition" << endl;
                    result = false;
                }//else storeCondition
            }//if getCommand
            else
                result = false;
            break;

        case NGRS_GOT_CONDITION:
            if (getCommand(oneLine, theCommand, parameter))
            {
                if (currentRule->storeAction(parameter))
                {
                    myState = NGRS_START;
                    theRules.push_back(currentRule);
                    currentRule = NULL;
                }//if storeAction()
                else
                {
                    cout << "RuleLoader: unknown action" << endl;
                    result = false;
                }//else storeAction()
            }//if getCommand
            else
                result = false;
            break;

        default:
            cout << "RuleLoader::processLine(): encountered an unknown state" << endl;
            result = false;
            break;
    }//switch

    if (!result)
    {
        cout << "lineNumber: " << lineNumber << endl;
        delete currentRule;
        currentRule = NULL;
    }//if !result

    return result;
}//processLine

bool RuleLoader::getCommand(string &oneLine,
                            string &theCommand,
                            string &parameter)
{
    unsigned int linePos = 0;//index of a character in oneLine
    bool finished = false;//true if we extracted the command type
    bool result = true;//false if this line is malformed

    theCommand.clear();
    parameter.clear();

    //### Verify that the line contains data ###
    if (oneLine.size() == 0)
    {
        cout << "RuleLoader: expected a command" << endl;
        result = false;
    }//if size

    //### Extract the command name ###
    if (result)
    {
        while ((!finished) && (linePos < oneLine.size()))
        {
            if (oneLine.at(linePos) == ' ')
                finished = true;
            else
                theCommand.push_back(oneLine.at(linePos));

            linePos++;
        }//while !finished

        if (!finished)
        {
            cout << "RuleLoader: expected a command, then a space, then a parameter." << endl;
            result = false;
        }//if !finished
    }//if result

    //### Verify that theCommand contains something ###
    if (result)
    {
        if (theCommand.size() == 0)
        {
            cout << "RuleLoader: there should be a command before the first space" << endl;
            result = false;
        }//if theCommand
    }//if result

    //### Extract the parameter ###
    if (result)
    {
        while (linePos < oneLine.size())
        {
            parameter.push_back(oneLine.at(linePos));
            linePos++;
        }//while linePos

        if (parameter.size() == 0)
        {
            cout << "RuleLoader: no parameter found" << endl;
            result = false;
        }//if size()
    }//if result

    return result;
}//getCommand

bool RuleLoader::checkActions(const std::string &firstLine,
                              bool &useGraphics)
{
    unsigned int rulePos = 0;//index of a rule within theRules
    bool result = false;//true if we found a matching rule

    //### Find a matching rule ###
    while ((rulePos < theRules.size()) && (!result))
    {
        result = theRules.at(rulePos)->checkRule(firstLine, useGraphics);
        rulePos++;
    }//while rulePos && !result

    return result;
}//checkActions
