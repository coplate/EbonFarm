/*Copyright 2009-2011 David McCallum

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

#include <iostream>
#include <string>
#include <QApplication>
#include "WhiteBoard.hpp"

int main(int argc,
         char *argv[])
{
    QApplication qtApp(argc, argv);
    WhiteBoard *whiteBoard = NULL;//contains the program
    std::string parameter;//a command-line parameter
    bool debugMode = false;//true if we should output debugging information
    int result = 0;//return value for this program

    //### Verify that a proper number of parameters was given ###
    if (argc > 2)
    {
         std::cout << "Usage: ebonhack [--debug]" << std::endl;
         result = 1;
    }//else if argc

    //### Check for the --debug parameter ###
    if (result == 0)
    {
        if (argc == 2)
        {
            parameter = argv[1];
            if (parameter == "--debug")
                debugMode = true;
            else
            {
                std::cout << "Usage: ebonhack [--debug]" << std::endl;
                result = 1;
            }//else argv
        }//if argc
    }//if result

    //### Run the program ###
    if (result == 0)
    {
        whiteBoard = new WhiteBoard(&qtApp, debugMode);
        result = whiteBoard->run();
    }//if result

    //### Free Memory ###
    delete whiteBoard;
    whiteBoard = NULL;

    std::cout << "Program halted." << std::endl;

    return result;
}//main
