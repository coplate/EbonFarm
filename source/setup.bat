echo off

echo Generating project...

qmake -project

echo QT += network >> ebonhack.pro
echo CONFIG += release >> ebonhack.pro
echo QMAKE_LIBS += -lm -lws2_32 >> ebonhack.pro
echo QT += opengl >> ebonhack.pro
echo QMAKE_CXXFLAGS += -DNG_OPEN_GL >> ebonhack.pro

echo Generating Makefile...

qmake ebonhack.pro
