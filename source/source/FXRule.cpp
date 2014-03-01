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

#include "FXRule.hpp"

using namespace std;

FXRule::FXRule(void)
{
    action = NGFX_ENABLE;
}//constructor

FXRule::~FXRule(void)
{
}//destructor

bool FXRule::storeCondition(string &prefix,
                            string &regExp)
{
    bool result = true;//false if we already got this condition, or it's unknown

    //### Verify that the prefix is REGEXP: ###
    if (prefix != "REGEXP:")
    {
        result = false;
        cout << "FXRule::storeCondition(): expecting REGEXP: got "
             << prefix << " " << regExp << endl;
    }//if prefix

    //### Store the regular expression ###
    if (result)
    {
        theRule.setPattern(QString::fromStdString(regExp));
        if (!theRule.isValid())
        {
            result = false;
            cout << "FXRule::storeCondition(): invalid expression" << endl;
        }//if !isValid()
    }//if result

    return result;
}//storeCondition

bool FXRule::storeAction(string &parameter)
{
    bool result = true;//false if we already got this action, or it's unknown

    //### Store the action ###
    if (result)
    {
        if (parameter == "enable")
            action = NGFX_ENABLE;

        else if (parameter == "disable")
            action = NGFX_DISABLE;

        else
            result = false;
    }//if result

    return result;
}//storeAction

bool FXRule::checkRule(const std::string &firstLine,
                       bool &useGraphics)
{
    bool result = false;//true if this line satisfies the expression

    if (theRule.indexIn(QString::fromStdString(firstLine), 0, QRegExp::CaretAtZero) >= 0)
    {
        if (action == NGFX_ENABLE)
            useGraphics = true;
        else if (action == NGFX_DISABLE)
            useGraphics = false;
        else
        {
            cout << "FXRule::checkRule(): invalid action detected" << endl;
            throw 1;
        }//else action

        result = true;
    }//if indexIn()

    return result;
}//checkRule
