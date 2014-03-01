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

#include "TelnetProtocol.hpp"
#include <QApplication>
#include "WhiteBoard.hpp"
#include "NetSprite.hpp"
#include "MainWindow.hpp"
#include "LatencyWidget.hpp"

using namespace std;

TelnetProtocol::TelnetProtocol(WhiteBoard *newWhiteBoard,
                               bool newDebug)
{
    whiteBoard = newWhiteBoard;
    debugMessages = newDebug;

    theWindow = new TelnetWindow(whiteBoard);
    tcpSocket = new QTcpSocket(NULL);
    myState = NGTS_ERROR;
    prevState = NGTS_ERROR;
    currentByte = 0;
    willTerminalSpeed = false;
    willTerminalType = false;
    willXDisplayLocation = false;
    willNewEnviron = false;
    willEcho = false;
    willToggleFlowControl = false;
    subState = 0;
    showError = true;
    connecting = false;

    escHandler = new XtermEscape(theWindow, whiteBoard, debugMessages);
    netCursor = NULL;

    //### Connect the socket event handlers ###
    connect(tcpSocket, SIGNAL(readyRead()),
            this, SLOT(runFSM()));
    connect(tcpSocket, SIGNAL(disconnected()),
            this, SLOT(socketDisconnected()));
}//constructor

TelnetProtocol::~TelnetProtocol(void)
{
    if (debugMessages)
        cout << "TELNET PROTOCOL DESTRUCTOR CALLED" << endl;

    disconnect(tcpSocket, SIGNAL(readyRead()),
               this, SLOT(runFSM()));
    disconnect(tcpSocket, SIGNAL(disconnected()),
               this, SLOT(socketDisconnected()));

    delete tcpSocket;
    tcpSocket = NULL;

    delete escHandler;
    escHandler = NULL;

    delete theWindow;
    theWindow = NULL;

    if (debugMessages)
        cout << "TELNET PROTOCOL DESTRUCTOR FINISHED" << endl;
}//destructor

bool TelnetProtocol::initialize(NetCursor *newNetCursor)
{
    NethackFX *netFX = whiteBoard->getNetFX();
    bool result = true;//false on errors

    netCursor = newNetCursor;

    //### Connect the send timer ###
    sendTimer.setSingleShot(false);
    sendTimer.setInterval(SEND_INTERVAL);
    connect(&sendTimer, SIGNAL(timeout()),
            this, SLOT(sendData()));
    sendTimer.start();

    //### Create the telnet window ###
    if (!theWindow->initialize(WINDOW_WIDTH, WINDOW_HEIGHT))
        result = false;

    //### Initialize Nethack graphics ###
    if (result)
    {
        if (!netFX->initialize(theWindow))
            result = false;
    }//if result

    return result;
}//initialize

bool TelnetProtocol::connectToServer(const string &serverAddr)
{
    QString errMsg;//show a message if we couldn't connect
    bool result = true;//false if we couldn't connect

    theWindow->enableEraseAll(false);

    //### If we're already connected to a server, disconnect ###
    connecting = true;
    if (tcpSocket->state() == QAbstractSocket::UnconnectedState)
        tcpSocket->disconnectFromHost();

    //### Reset the FSM ###
    escHandler->resetFSM();

    //### Connect to the new server ###
    tcpSocket->connectToHost(QString::fromStdString(serverAddr), 23, QIODevice::ReadWrite);
    if (tcpSocket->waitForConnected())
        myState = NGTS_START;
    else
    {
        errMsg = tr("Couldn't connect to ");
        errMsg.append(QString::fromStdString(serverAddr));
        whiteBoard->showMessage(errMsg);

        myState = NGTS_DISCONNECTED;
        result = false;
    }//else wait

    connecting = false;

    return result;
}//connectToServer

void TelnetProtocol::socketDisconnected(void)
{
    MainWindow *mainWindow = whiteBoard->getMainWindow();
    LatencyWidget *latencyWidget = mainWindow->getLatencyWidget();

    myState = NGTS_DISCONNECTED;
    theWindow->eraseAll();
    theWindow->setCursorX(1);
    theWindow->setCursorY(1);

    if (!connecting)
        whiteBoard->showMessage("We've disconnected from the server!");

    latencyWidget->reportDisconnect();
    mainWindow->setGraphicsMode(false);
}//socketDisconnected

void TelnetProtocol::runFSM(void)
{
    NethackFX *netFX = whiteBoard->getNetFX();
    MainWindow *mainWindow = whiteBoard->getMainWindow();
    LatencyWidget *latencyWidget = mainWindow->getLatencyWidget();
    QByteArray serverData;//data sent by the server
    int byteIndex = 0;//index of the current byte to parse

    //### Receive data from server ###
    serverData = tcpSocket->readAll();

    //### Verify that the socket is valid ###
    if (!tcpSocket->isValid())
    {
        cout << "TelnetProtocol::runFSM(): socket not valid." << endl;
        tcpSocket->disconnectFromHost();
    }//if !isValid()

    //### Calculate server latency ###
    if (serverData.size() > 0)
        latencyWidget->reportReply();

    while (byteIndex < serverData.size())
    {
        //### Extract a byte from the server's message ###
        currentByte = serverData.at(byteIndex);
        byteIndex++;

        //### Output the byte ###
        if (debugMessages)
        {
            if ((currentByte <= 32) || (currentByte >= 240))
                cout << " <" << static_cast<int>(currentByte) << "> ";
            else
                cout << currentByte;
        }//if debugMessages

        //### Reset subState if the FSM changed states ###
        if (prevState != myState)
        {
            prevState = myState;
            subState = 0;
        }//if prevState

        //### Run the FSM ###
        switch (myState)
        {
            case NGTS_DISCONNECTED:
                cout << "TelnetProtocol::runFSM(): called after disconnected." << endl;
                break;

            case NGTS_START: runStart();
                break;

            case NGTS_ERROR:
                showErrorDialog(serverData);
                myState = NGTS_START;
                runStart();
                break;

            case NGTS_IAC: runIAC();
                break;

            case NGTS_ESC: runESC();
                break;

            case NGTS_IAC_DO: runIACDo();
                break;

            case NGTS_IAC_SB: runIACSB();
                break;

            case NGTS_IAC_WILL: runIACWill();
                break;

            case NGTS_IAC_DONT: runIACDont();
                break;

            case NGTS_IAC_SB_TERMSPEED: runIACSBTermSpeed();
                break;

            case NGTS_IAC_SB_XDISPLOC: runIACSBXdisploc();
                break;

            case NGTS_IAC_SB_NEWENVIRON: runIACSBNewEnviron();
                break;

            case NGTS_IAC_SB_TERMTYPE: runIACSBTermType();
                break;

            case NGTS_IAC_SB_TOGGLEFLOW: runIACSBToggleFlow();
                break;

            default:
                cout << "TelnetProtocol::runFSM(): encountered an unknown state!" << endl;
                throw 1;
                break;
        }//switch myState
    }//while byteIndex

    netFX->updateLogic(theWindow);
    netCursor->setCursorPos(theWindow->getCursorX(), theWindow->getCursorY());
}//runFSM

void TelnetProtocol::runStart(void)
{
    switch (currentByte)
    {
        case NGTP_IAC: myState = NGTS_IAC;
            break;

        //carriage return
        case NGTC_CR:
            theWindow->setCursorX(1);
            myState = NGTS_START;
            break;

        //line feed
        case NGTC_LF:
            theWindow->moveCursorY(1);
            myState = NGTS_START;
            break;

        //no operation
        case NGTC_NOP:
            myState = NGTS_START;
            break;

        //backspace
        case NGTC_BS:
            theWindow->moveCursorX(-1);
            myState = NGTS_START;
            break;

        //escape
        case NGTC_ESC:
            myState = NGTS_ESC;
            break;

        //shift in
        case NGTC_SHIFT_IN:
            myState = NGTS_START;
            break;

        //shift out
        case NGTC_SHIFT_OUT:
            myState = NGTS_START;
            break;

        //beep
        case NGTC_BELL:
            QApplication::beep();
            myState = NGTS_START;
            break;

        default:
            theWindow->writeByte(currentByte);
            myState = NGTS_START;
            break;
    }//switch currentByte
}//runStart

void TelnetProtocol::runESC(void)
{
    NGX_State escState = escHandler->run(currentByte);//the current state of the xterm FSM

    if (escState == NGXS_START)
        myState = NGTS_START;
    else if (escState == NGXS_ERROR)
        myState = NGTS_ERROR;
}//runESC

void TelnetProtocol::runIAC(void)
{
    switch (currentByte)
    {
        case NGTP_SB: myState = NGTS_IAC_SB;
            break;

        case NGTP_DO: myState = NGTS_IAC_DO;
            break;

        case NGTP_WILL: myState = NGTS_IAC_WILL;
            break;

        case NGTP_DONT: myState = NGTS_IAC_DONT;
            break;

        case NGTP_DATA_MARK: myState = NGTS_START;
            break;

        default:
            cout << "TelnetProtocol::runIAC(): got an unknown byte: "
                 << static_cast<int>(currentByte) << endl;
            myState = NGTS_ERROR;
            break;
    }//switch currentByte
}//runIAC

void TelnetProtocol::runIACDo(void)
{
    QByteArray reply;//our reply to the server

    switch (currentByte)
    {
        case NGTP_TERMINAL_TYPE:
            if (debugMessages)
            {
                cout << "IAC DO TERMINAL_TYPE" << endl;
                cout << "Sending IAC WILL TERMINAL_TYPE" << endl;
            }//if debugMessages

            reply.push_back(NGTP_IAC);
            reply.push_back(NGTP_WILL);
            reply.push_back(NGTP_TERMINAL_TYPE);
            repeatSend(reply);

            willTerminalType = true;
            myState = NGTS_START;
            break;

        case NGTP_TERMINAL_SPEED:
            if (debugMessages)
            {
                cout << "IAC DO TERMINAL_SPEED" << endl;
                cout << "Sending IAC WILL TERMINAL_SPEED" << endl;
            }//if debugMessages

            reply.push_back(NGTP_IAC);
            reply.push_back(NGTP_WILL);
            reply.push_back(NGTP_TERMINAL_SPEED);
            repeatSend(reply);

            willTerminalSpeed = true;
            myState = NGTS_START;
            break;

        case NGTP_X_DISPLAY_LOCATION:
            if (debugMessages)
            {
                cout << "IAC DO X_DISPLAY_LOCATION" << endl;
                cout << "Sending IAC WILL X_DISPLAY_LOCATION" << endl;
            }//if debugMessages

            reply.push_back(NGTP_IAC);
            reply.push_back(NGTP_WILL);
            reply.push_back(NGTP_X_DISPLAY_LOCATION);
            repeatSend(reply);

            willXDisplayLocation = true;
            myState = NGTS_START;
            break;

        case NGTP_NEW_ENVIRON:
            if (debugMessages)
            {
                cout << "IAC DO NEW_ENVIRON" << endl;
                cout << "Sending IAC WILL NEW_ENVIRON" << endl;
            }//if debugMessages

            reply.push_back(NGTP_IAC);
            reply.push_back(NGTP_WILL);
            reply.push_back(NGTP_NEW_ENVIRON);
            repeatSend(reply);

            willNewEnviron = true;
            myState = NGTS_START;
            break;

        case NGTP_ECHO:
            if (debugMessages)
            {
                cout << "IAC DO ECHO" << endl;
                cout << "Sending IAC WILL ECHO" << endl;
            }//if debugMessages

            reply.push_back(NGTP_IAC);
            reply.push_back(NGTP_WILL);
            reply.push_back(NGTP_ECHO);
            repeatSend(reply);

            willEcho = true;
            myState = NGTS_START;
            break;

        case NGTP_NAWS:
            if (debugMessages)
            {
                cout << "IAC DO NAWS" << endl;
                cout << "Sending IAC WILL NAWS" << endl;
            }//if debugMessages

            reply.push_back(NGTP_IAC);
            reply.push_back(NGTP_WILL);
            reply.push_back(NGTP_NAWS);

            if (debugMessages)
                cout << "Sending IAC SB NAWS 0 " << static_cast<unsigned int>(theWindow->getWidth())
                     << " 0 " << static_cast<unsigned int>(theWindow->getHeight())
                     <<  " IAC SE" << endl;

            reply.push_back(NGTP_IAC);
            reply.push_back(NGTP_SB);
            reply.push_back(NGTP_NAWS);
            reply.push_back(static_cast<char>(0));
            reply.push_back(theWindow->getWidth());
            reply.push_back(static_cast<char>(0));
            reply.push_back(theWindow->getHeight());
            reply.push_back(NGTP_IAC);
            reply.push_back(NGTP_SE);
            repeatSend(reply);

            myState = NGTS_START;
            break;

        case NGTP_TOGGLE_FLOW_CONTROL:
            if (debugMessages)
            {
                cout << "IAC DO TOGGLE_FLOW_CONTROL" << endl;
                cout << "Sending IAC WILL TOGGLE_FLOW_CONTROL" << endl;
            }//if debugMessages

            reply.push_back(NGTP_IAC);
            reply.push_back(NGTP_WILL);
            reply.push_back(NGTP_TOGGLE_FLOW_CONTROL);
            repeatSend(reply);

            willToggleFlowControl = true;
            myState = NGTS_START;
            break;

        default:
            cout << "TelnetProtocol::runIACDo(): got an unknown byte: "
                 << static_cast<int>(currentByte) << endl;
            myState = NGTS_ERROR;
            break;
    }//switch currentByte
}//runIACDo

void TelnetProtocol::runIACSB(void)
{
    switch (currentByte)
    {
        case NGTP_TERMINAL_SPEED: myState = NGTS_IAC_SB_TERMSPEED;
            break;

        case NGTP_X_DISPLAY_LOCATION: myState = NGTS_IAC_SB_XDISPLOC;
            break;

        case NGTP_NEW_ENVIRON: myState = NGTS_IAC_SB_NEWENVIRON;
            break;

        case NGTP_TERMINAL_TYPE: myState = NGTS_IAC_SB_TERMTYPE;
            break;

        case NGTP_TOGGLE_FLOW_CONTROL: myState = NGTS_IAC_SB_TOGGLEFLOW;
            break;

        default:
            cout << "TelnetProtocol::runIACSB(): got an unknown byte: "
                 << static_cast<int>(currentByte) << endl;
            myState = NGTS_ERROR;
            break;
    }//switch currentByte
}//runIACSB

void TelnetProtocol::runIACWill(void)
{
    QByteArray reply;//a reply to send to the server

    switch (currentByte)
    {
        case NGTP_SUPPRESS_GO_AHEAD:
            if (debugMessages)
            {
                cout << "IAC WILL SUPPRESS-GO-AHEAD" << endl;
                cout << "Sending IAC DO SUPPRESS-GO-AHEAD" << endl;
            }//if debugMessages

            reply.push_back(NGTP_IAC);
            reply.push_back(NGTP_DO);
            reply.push_back(NGTP_SUPPRESS_GO_AHEAD);

            repeatSend(reply);
            myState = NGTS_START;
            break;

        case NGTP_STATUS:
            if (debugMessages)
            {
                cout << "IAC WILL STATUS" << endl;
                cout << "Sending IAC DO STATUS" << endl;
            }//if debugMessages

            reply.push_back(NGTP_IAC);
            reply.push_back(NGTP_DO);
            reply.push_back(NGTP_STATUS);

            repeatSend(reply);
            myState = NGTS_START;
            break;

        case NGTP_ECHO:
            if (debugMessages)
                cout << "IAC WILL ECHO" << endl;
            myState = NGTS_START;
            break;

        default:
            cout << "TelnetProtocol::runIACWill(): got an unknown byte: "
                 << static_cast<int>(currentByte) << endl;
            myState = NGTS_ERROR;
            break;
    }//switch currentByte
}//runIACWill

void TelnetProtocol::runIACDont(void)
{
    QByteArray reply;//a reply to send to the server

    switch (currentByte)
    {
        case NGTP_ECHO:
            if (debugMessages)
            {
                cout << "IAC DONT ECHO" << endl;
                cout << "Sending IAC WONT ECHO" << endl;
            }//if debugMessages

            reply.push_back(NGTP_IAC);
            reply.push_back(NGTP_WONT);
            reply.push_back(NGTP_ECHO);

            repeatSend(reply);
            myState = NGTS_START;
            break;

        default:
            cout << "TelnetProtocol::runIACDont(): got an unknown byte: "
                 << static_cast<int>(currentByte) << endl;
            myState = NGTS_ERROR;
            break;
    }//switch currentByte
}//runIACDont

void TelnetProtocol::runIACSBTermSpeed(void)
{
    QByteArray reply;//a reply to send to the server

    switch (subState)
    {
        //SEND
        case 0:
            if (currentByte == NGTN_SEND)
                subState++;
            else
            {
                cout << "TelnetProtocol::runIACSBTermSpeed(): state 0 got unknown byte "
                     << currentByte << endl;
                myState = NGTS_ERROR;
            }//else currentByte
            break;

        //IAC
        case 1:
            if (currentByte == NGTP_IAC)
                subState++;
            else
            {
                cout << "TelnetProtocol::runIACSBTermSpeed(): state 1 got unknown byte "
                     << currentByte << endl;
                myState = NGTS_ERROR;
            }//else currentByte
            break;

        //SE
        case 2:
            if (currentByte == NGTP_SE)
            {
                if (debugMessages)
                {
                    cout << "IAC SB TERMINAL-SPEED SEND IAC SE" << endl;
                    cout << "Sending IAC SB TERMINAL-SPEED IS 38400,38400 IAC SE" << endl;
                }//if debugMessages

                reply.push_back(NGTP_IAC);
                reply.push_back(NGTP_SB);
                reply.push_back(NGTP_TERMINAL_SPEED);
                reply.push_back(NGTN_IS);
                reply.push_back('3');
                reply.push_back('8');
                reply.push_back('4');
                reply.push_back('0');
                reply.push_back('0');
                reply.push_back(',');
                reply.push_back('3');
                reply.push_back('8');
                reply.push_back('4');
                reply.push_back('0');
                reply.push_back('0');
                reply.push_back(NGTP_IAC);
                reply.push_back(NGTP_SE);

                repeatSend(reply);
                myState = NGTS_START;
            }//if currentByte
            else
            {
                cout << "TelnetProtocol::runIACSBTermSpeed(): state 2 got unknown byte "
                     << currentByte << endl;
                myState = NGTS_ERROR;
            }//else currentByte
            break;

        default:
            cout << "TelnetProtocol::runIACSBTermSpeed(): hit an unknown state!" << endl;
            myState = NGTS_ERROR;
            break;
    }//switch currentByte
}//runIACSBTermSpeed

void TelnetProtocol::runIACSBXdisploc(void)
{
    QByteArray reply;//a reply to send to the server

    switch (subState)
    {
        //SEND
        case 0:
            if (currentByte == NGTN_SEND)
                subState++;
            else
            {
                cout << "TelnetProtocol::runIACSBTermSpeed(): state 0 got unknown byte "
                    << currentByte << endl;
                myState = NGTS_ERROR;
            }//else currentByte
            break;

        //IAC
        case 1:
            if (currentByte == NGTP_IAC)
                subState++;
            else
            {
                cout << "TelnetProtocol::runIACSBTermSpeed(): state 1 got unknown byte "
                     << currentByte << endl;
                myState = NGTS_ERROR;
            }//else currentByte
            break;

        //SE
        case 2:
            if (currentByte == NGTP_SE)
            {
                if (debugMessages)
                {
                    cout << "IAC SB X-DISPLAY-LOCATION SEND IAC SE" << endl;
                    cout << "Sending IAC SB X-DISPLAY-LOCATION IS ebonhack:0.0 IAC SE" << endl;
                }//if debugMessages

                reply.push_back(NGTP_IAC);
                reply.push_back(NGTP_SB);
                reply.push_back(NGTP_X_DISPLAY_LOCATION);
                reply.push_back(NGTN_IS);
                reply.push_back('e');
                reply.push_back('b');
                reply.push_back('o');
                reply.push_back('n');
                reply.push_back('h');
                reply.push_back('a');
                reply.push_back('c');
                reply.push_back('k');
                reply.push_back(':');
                reply.push_back('0');
                reply.push_back('.');
                reply.push_back('0');
                reply.push_back(NGTP_IAC);
                reply.push_back(NGTP_SE);

                repeatSend(reply);
                myState = NGTS_START;
            }//if currentByte
            else
            {
                cout << "TelnetProtocol::runIACSBTermSpeed(): state 2 got unknown byte "
                     << currentByte << endl;
                myState = NGTS_ERROR;
            }//else currentByte
            break;

        default:
            cout << "TelnetProtocol::runIACSBXdisploc(): hit an unknown state!" << endl;
            myState = NGTS_ERROR;
            break;
    }//switch subState
}//runIACSBXdisploc

void TelnetProtocol::runIACSBNewEnviron(void)
{
    QByteArray reply;//a reply to send to the server

    switch (subState)
    {
        //SEND
        case 0:
            if (currentByte == NGTN_SEND)
                subState++;
            else
            {
                cout << "TelnetProtocol::runIACSBNewEnviron(): state 0 got unknown byte "
                     << currentByte << endl;
                myState = NGTS_ERROR;
            }//else currentByte
            break;

        //IAC
        case 1:
            if (currentByte == NGTP_IAC)
                subState++;
            else
            {
                cout << "TelnetProtocol::runIACSBNewEnviron(): state 1 got unknown byte "
                     << currentByte << endl;
                myState = NGTS_ERROR;
            }//else currentByte
            break;

        //SE
        case 2:
            if (currentByte == NGTP_SE)
            {
                if (debugMessages)
                {
                    cout << "IAC SB NEW_ENVIRON SEND IAC SE" << endl;
                    cout << "Sending IAC SB NEW-ENVIRON IS VAR DISPLAY VALUE jackfish:0.0 IAC SE" << endl;
                }//if debugMessages

                reply.push_back(NGTP_IAC);
                reply.push_back(NGTP_SB);
                reply.push_back(NGTP_NEW_ENVIRON);
                reply.push_back(NGTN_IS);
                reply.push_back(static_cast<char>(0));//VAR
                reply.push_back('D');
                reply.push_back('I');
                reply.push_back('S');
                reply.push_back('P');
                reply.push_back('L');
                reply.push_back('A');
                reply.push_back('Y');
                reply.push_back(1);//VALUE
                reply.push_back('j');
                reply.push_back('a');
                reply.push_back('c');
                reply.push_back('k');
                reply.push_back('f');
                reply.push_back('i');
                reply.push_back('s');
                reply.push_back('h');
                reply.push_back(':');
                reply.push_back('0');
                reply.push_back('.');
                reply.push_back('0');
                reply.push_back(NGTP_IAC);
                reply.push_back(NGTP_SE);

                repeatSend(reply);
                myState = NGTS_START;
            }//if currentByte
            else
            {
                cout << "TelnetProtocol::runIACSBNewEnviron(): state 2 got unknown byte "
                     << currentByte << endl;
                myState = NGTS_ERROR;
            }//else currentByte
            break;

        default:
            cout << "TelnetProtocol::runIACSBNewEnviron(): hit an unknown state!" << endl;
            myState = NGTS_ERROR;
            break;
    }//switch subState
}//runIACSBNewEnviron

void TelnetProtocol::runIACSBTermType(void)
{
    QByteArray reply;//a reply to send to the server

    switch (subState)
    {
        //SEND
        case 0:
            if (currentByte == NGTN_SEND)
                subState++;
            else
            {
                cout << "TelnetProtocol::runIACSBTermType(): state 0 got unknown byte "
                     << currentByte << endl;
                myState = NGTS_ERROR;
            }//else currentByte
            break;

        //IAC
        case 1:
            if (currentByte == NGTP_IAC)
                subState++;
            else
            {
                cout << "TelnetProtocol::runIACSBTermType(): state 1 got unknown byte "
                     << currentByte << endl;
                myState = NGTS_ERROR;
            }//else currentByte
            break;

        //SE
        case 2:
            if (currentByte == NGTP_SE)
            {
                if (debugMessages)
                {
                    cout << "IAC SB TERMINAL_TYPE SEND IAC SE" << endl;
                    cout << "Sending IAC SB TERMINAL-TYPE IS xterm IAC SE" << endl;
                }//if debugMessages

                reply.push_back(NGTP_IAC);
                reply.push_back(NGTP_SB);
                reply.push_back(NGTP_TERMINAL_TYPE);
                reply.push_back(NGTN_IS);
                reply.push_back('x');
                reply.push_back('t');
                reply.push_back('e');
                reply.push_back('r');
                reply.push_back('m');
                reply.push_back(NGTP_IAC);
                reply.push_back(NGTP_SE);

                repeatSend(reply);
                myState = NGTS_START;
            }//if currentByte
            else
            {
                cout << "TelnetProtocol::runIACSBTermType(): state 2 got unknown byte "
                     << currentByte << endl;
                myState = NGTS_ERROR;
            }//else currentByte
            break;

        default:
            cout << "TelnetProtocol::runIACSBTermType(): hit an unknown state!" << endl;
            myState = NGTS_ERROR;
            break;
    }//switch subState
}//runIACSBTermType()

void TelnetProtocol::runIACSBToggleFlow()
{
    switch (subState)
    {
        //flow type
        case 0:
            if (currentByte == 0)//OFF
                subState = 10;
            else if (currentByte == 1)//ON
                subState = 20;
            else if (currentByte == 2)//RESTART_ANY
                subState = 30;
            else if (currentByte == 3)//RESTART_XON
                subState = 40;
            else
            {
                cout << "TelnetProtocol::runIACSBToggleFlow(): state 0 got unknown byte "
                     << currentByte << endl;
                myState = NGTS_ERROR;
            }//else currentByte
            break;

        //ALL, IAC
        case 10:
        case 20:
        case 30:
        case 40:
            if (currentByte == NGTP_IAC)
                subState++;
            else
            {
                cout << "TelnetProtocol::runIACSBToggleFlow(): state "
                     << subState << " got unknown byte " << currentByte << endl;
                myState = NGTS_ERROR;
            }//else currentByte
            break;

        //OFF, SE
        case 11:
            if (currentByte == NGTP_SE)
            {
                if (debugMessages)
                    cout << "IAC SB TOGGLE_FLOW_CONTROL OFF IAC SE" << endl;
                myState = NGTS_START;
            }//if currentByte
            else
            {
                cout << "TelnetProtocol::runIACSBToggleFlow(): state "
                     << subState << " got unknown byte " << currentByte << endl;
                myState = NGTS_ERROR;
            }//else currentByte
            break;

        //ON, SE
        case 21:
            if (currentByte == NGTP_SE)
            {
                if (debugMessages)
                    cout << "IAC SB TOGGLE_FLOW_CONTROL ON IAC SE" << endl;
                myState = NGTS_START;
            }//if currentByte
            else
            {
                cout << "TelnetProtocol::runIACSBToggleFlow(): state "
                     << subState << " got unknown byte " << currentByte << endl;
                myState = NGTS_ERROR;
            }//else currentByte
            break;

        //RESTART_ANY, SE
        case 31:
            if (currentByte == NGTP_SE)
            {
                if (debugMessages)
                    cout << "IAC SB TOGGLE_FLOW_CONTROL RESTART_ANY IAC SE" << endl;
                myState = NGTS_START;
            }//if currentByte
            else
            {
                cout << "TelnetProtocol::runIACSBToggleFlow(): state "
                     << subState << " got unknown byte " << currentByte << endl;
                myState = NGTS_ERROR;
            }//else currentByte
            break;

        //RESTART_XON, SE
        case 41:
            if (currentByte == NGTP_SE)
            {
                if (debugMessages)
                    cout << "IAC SB TOGGLE_FLOW_CONTROL RESTART_XON IAC SE" << endl;
                myState = NGTS_START;
            }//if currentByte
            else
            {
                cout << "TelnetProtocol::runIACSBToggleFlow(): state "
                     << subState << " got unknown byte " << currentByte << endl;
                myState = NGTS_ERROR;
            }//else currentByte
            break;

        default:
            cout << "TelnetProtocol::runIACSBToggleFlow(): hit an unknown state!" << endl;
            myState = NGTS_ERROR;
            break;
    }//switch subState
}//runIACSBToggleFlow

bool TelnetProtocol::getDisplayChanged(void)
{
    return theWindow->getDisplayChanged();
}//getDisplayChanged

uint8_t TelnetProtocol::getWidth(void)
{
    return theWindow->getWidth();
}//getWidth

uint8_t TelnetProtocol::getHeight(void)
{
    return theWindow->getHeight();
}//getHeight

SGRAttribute* TelnetProtocol::getAttributes(uint8_t xPos,
                                            uint8_t yPos)
{
    return theWindow->getAttributes(xPos, yPos);
}//getAttributes

TelnetWindow* TelnetProtocol::getTelnetWindow(void)
{
    return theWindow;
}//getTelnetWindow

uint8_t TelnetProtocol::getByte(uint8_t xPos,
                                uint8_t yPos)
{
    return theWindow->getByte(xPos, yPos);
}//getByte

void TelnetProtocol::sendKeystroke(uint8_t keyType)
{
    MainWindow *mainWindow = whiteBoard->getMainWindow();
    LatencyWidget *latencyWidget = mainWindow->getLatencyWidget();
    QByteArray theMessage;//the message to send

    theWindow->enableEraseAll(true);

    theMessage.append(keyType);
    latencyWidget->reportCommand();
    repeatSend(theMessage);
}//sendKeystroke

void TelnetProtocol::sendCommand(const QString &command)
{
    MainWindow *mainWindow = whiteBoard->getMainWindow();
    LatencyWidget *latencyWidget = mainWindow->getLatencyWidget();
    QByteArray theMessage;//the message to send

    theWindow->enableEraseAll(true);

    theMessage.append(command);
    latencyWidget->reportCommand();
    repeatSend(theMessage);
}//sendKeystroke

void TelnetProtocol::repeatSend(const QByteArray &theData)
{
    sendQueue.append(theData);
    sendData();
}//repeatSend

void TelnetProtocol::sendData(void)
{
    int bytesWritten = 0;//the number of bytes sent

    if (sendQueue.size() > 0)
    {
        bytesWritten = tcpSocket->write(sendQueue);

        if (bytesWritten == sendQueue.size())
            sendQueue.clear();

        else if (bytesWritten < 0)
        {
            cout << "TelnetProtocol::sendData(): send error" << endl;
            tcpSocket->disconnectFromHost();
            sendQueue.clear();
        }//else if bytesWritten

        else
        {
            cout << "TelnetProtocol::sendData(): sent " << bytesWritten
                 << " out of " << sendQueue.size() << " bytes " << endl;

            sendQueue.remove(0, bytesWritten);
        }//if bytesWritten

        if (!tcpSocket->isValid())
        {
            cout << "TelnetProtocol::sendData(): socket not valid" << endl;
            tcpSocket->disconnectFromHost();
            sendQueue.clear();
        }//if !isValid()
    }//if size()
}//sendData

void TelnetProtocol::showBoundsDialog(void)
{
    QString errMsg;//the error to display

    if (showError)
    {
        errMsg = QString("WARNING: EbonHack received data outside of the display window. ");
        errMsg += QString("The display may no longer accurately reflect the game contents. ");
        errMsg += QString("\n\n");
        errMsg += QString("This message will only appear once.");

        whiteBoard->showMessage(errMsg);

        showError = false;
    }//if showError
}//showBoundsDialog

XtermEscape* TelnetProtocol::getEscHandler(void)
{
    return escHandler;
}//getEscHandler

void TelnetProtocol::showErrorDialog(const QByteArray &serverData)
{
    QString errMsg;//the error to display
    uint8_t oneByte = 0;//the current byte to output

    if (serverData.size() > 0)
    {
        cout << "Remaining data: " << endl;
        for (int i = 0; i < serverData.size(); i++)
        {
            oneByte = serverData.at(i);
            cout << static_cast<int>(oneByte) << " ";
        }//for i
        cout << endl;
    }//if size()

    if (showError)
    {
        cout << "TelnetProtocol got an unknown message." << endl;

        errMsg = QString("WARNING: EbonHack received an unknown message from the server. ");
        errMsg += QString("The display may no longer accurately reflect the game contents. ");
        errMsg += QString("\n\n");
        errMsg += QString("This message will only appear once.");

        whiteBoard->showMessage(errMsg);
        showError = false;
    }//if showError
}//showErrorDialog
