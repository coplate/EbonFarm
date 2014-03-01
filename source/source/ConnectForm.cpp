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

#include "ConnectForm.hpp"
#include "ConfigWriter.hpp"
#include "WhiteBoard.hpp"
#include "TelnetProtocol.hpp"
#include "MainWindow.hpp"

using namespace std;

ConnectForm::ConnectForm(QWidget* parent,
                         Qt::WindowFlags flags,
                         WhiteBoard* newWhiteBoard) : QDialog(parent, flags)
{
    string hostName = ConfigWriter::loadString("game_config.txt", "Host Name");

    whiteBoard = newWhiteBoard;

    //### Configure GUI ###
    gui.setupUi(this);
    gui.hostNameEdit->setText(QString::fromStdString(hostName));
    toggleConnectButton();

    //### Connect Widgets ###
    connect(gui.hostNameEdit, SIGNAL(returnPressed()),
            this, SLOT(connectToServer()));
    connect(gui.hostNameEdit, SIGNAL(textEdited(const QString &)),
            this, SLOT(toggleConnectButton()));
    connect(gui.cancelButton, SIGNAL(clicked()),
            this, SLOT(cancelButtonClicked()));
    connect(gui.connectButton, SIGNAL(clicked()),
            this, SLOT(connectToServer()));

    show();
}//constructor

ConnectForm::~ConnectForm(void)
{
}//destructor

void ConnectForm::cancelButtonClicked(void)
{
    hide();
}//cancelButtonClicked

void ConnectForm::connectToServer(void)
{
    MainWindow *mainWindow = whiteBoard->getMainWindow();
    TelnetProtocol *telnetPro = whiteBoard->getTelnetPro();
    string hostName;//the host to connect to

    toggleConnectButton();

    if (gui.connectButton->isEnabled())
    {
        hostName = gui.hostNameEdit->text().toStdString();
        ConfigWriter::writeString("game_config.txt", "Host Name", hostName);

        if (telnetPro->connectToServer(hostName))
        {
            hide();
            mainWindow->reportConnection();
        }//if connectToServer()
        else
            whiteBoard->showMessage(tr("Couldn't connect to server."));
    }//if isEnabled()
}//connectToServer

void ConnectForm::toggleConnectButton(void)
{
    if (gui.hostNameEdit->text().size() > 0)
        gui.connectButton->setEnabled(true);
    else
        gui.connectButton->setEnabled(false);
}//toggleConnectButton
