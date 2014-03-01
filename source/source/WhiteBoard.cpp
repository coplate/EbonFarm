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

#include "WhiteBoard.hpp"
#include <QApplication>
#include <QString>
#include <QObject>
#include "MainWindow.hpp"
#include "NethackFX.hpp"
#include "TelnetProtocol.hpp"
#include "NetSprite.hpp"
#include "MessageForm.hpp"

using namespace std;

QRegExp WhiteBoard::regExpRule;

WhiteBoard::WhiteBoard(QApplication *newQtApp,
                       bool debugMode)
{
    qtApp = newQtApp;

    //Null the objects in case the constructor from one relies on another,
    //makes it easier to debug.
    telnetPro = NULL;
    mainWindow = NULL;
    messageForm = NULL;
    netFX = NULL;

    ConfigWriter::setPath(NGSettings::DATA_PATH);

    telnetPro = new TelnetProtocol(this, debugMode);
    mainWindow = new MainWindow(NULL, NULL, this);
    messageForm = new MessageForm(NULL, Qt::WindowStaysOnTopHint);
    netFX = new NethackFX(this);
}//constructor

WhiteBoard::~WhiteBoard(void)
{
    delete messageForm;
    messageForm = NULL;

	delete telnetPro;
    telnetPro = NULL;

    delete mainWindow;
    mainWindow = NULL;

    netFX->save();
    delete netFX;//must come after mainWindow is deleted
    netFX = NULL;
}//destructor

int WhiteBoard::run(void)
{
    int result = 0;//the application's return code

    if (!mainWindow->start())
        result = 1;

    if (result == 0)
        result = qtApp->exec();

    return result;
}//run

NethackFX* WhiteBoard::getNetFX(void)
{
    return netFX;
}//getNetFX

QApplication* WhiteBoard::getQTApp(void)
{
    return qtApp;
}//getQTApp

MainWindow* WhiteBoard::getMainWindow(void)
{
    return mainWindow;
}//getMainWindow

TelnetProtocol* WhiteBoard::getTelnetPro(void)
{
    return telnetPro;
}//getTelnetPro

MessageForm* WhiteBoard::getMessageForm(void)
{
    return messageForm;
}//getMessageForm

void WhiteBoard::showMessage(const QString &theMessage)
{
    messageForm->showMessage(theMessage);
}//showMessage

void WhiteBoard::fatalError(const QString &errMsg)
{
    cout << errMsg.toStdString() << endl;
    cout << "Crashing..." << endl;

    throw 1;
}//fatalError

int WhiteBoard::indexIn(const string &regExp,
                        const string &theString)
{
    regExpRule.setPattern(QString::fromStdString(regExp));

    QString test = QString::fromStdString(theString);
    cout << "Pattern: " << regExpRule.pattern().toStdString() << endl;
    cout << "String: '" << test.toStdString() << "'" << endl;
    cout << "Result: " << regExpRule.indexIn(QString::fromStdString(theString), 0, QRegExp::CaretAtZero) << endl;
    cout << endl;

    return regExpRule.indexIn(QString::fromStdString(theString), 0, QRegExp::CaretAtZero);
}//indexIn

