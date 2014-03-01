#!/bin/bash

QMAKE_LOC=qmake
USE_OPENGL=1
DEBUG_MODE=0

function showUsage
{
    echo Usage:
    echo '   setup.sh [--no_opengl] [--debug]'
    exit
}

#Verify that a valid number of parameters were given
if [ $# -gt "2" ]
then
    echo Too many parameters.
    echo
    showUsage
fi

#Generate the basic project file
$QMAKE_LOC -project

#Parse the command line parameters
for p in $@;
do
    if [ $p == "--no_opengl" ]
    then
        USE_OPENGL=0

    elif [ $p == "--debug" ]
    then
        DEBUG_MODE=1

    else
        echo Invalid parameter: $p
        showUsage
    fi
done

#Enable OpenGL support
if [ $USE_OPENGL -eq 1 ]
then
    echo OpenGL support enabled.
    echo QT += opengl >> ebonhack.pro
    echo QMAKE_CXXFLAGS += -DNG_OPEN_GL >> ebonhack.pro
else
    echo OpenGL support disabled.
fi

#Specify Qt options and libraries
echo QT += network >> ebonhack.pro
echo LIBS += -lm >> ebonhack.pro
echo CONFIG += qt thread >> ebonhack.pro

#Specify directories
echo DESTDIR = ./ >> ebonhack.pro
echo MOC_DIR = ./object >> ebonhack.pro
echo OBJECTS_DIR = ./object >> ebonhack.pro
echo UI_DIR = ./object >> ebonhack.pro

#Specify debug or release mode
if [ $DEBUG_MODE -eq 1 ]
then
    echo Building debug version.
    echo CONFIG += debug >> ebonhack.pro
else
    echo Building release version.
    echo CONFIG += release >> ebonhack.pro
fi

echo Generating Makefile...
$QMAKE_LOC ebonhack.pro

echo
echo Now type 'make' to compile
echo
