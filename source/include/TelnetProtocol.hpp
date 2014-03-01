/* DESCRIPTION

  A Finite State Machine that understands the telnet protocol. This machine is
  not complete, there are parts of the telnet protocol that are not handled.
  It should be able to understand everything the nethack server sends it.

  The finite state machine is divided hierarchically to reduce complexity.
  Each state in the highest-level machine may contain a complete sub-FSM,
  depending on the complexity and length of the data. When sub FSMs
  are implemented in the TelnetProtocol class, the subState variable is used
  to refer to our state.

  Only simple sub-FSMs are handled here. Complex FSMs are handled in other classes.
  For example, the XtermEscape class handles Xterm escape sequences.
*/

#ifndef NG_TELNET_PROTOCOL
#define NG_TELNET_PROTOCOL

#include <string>
#include <fstream>
#include <QTcpSocket>

#include "DisplayRow.hpp"
#include "XtermEscape.hpp"
#include "NethackFX.hpp"
#include "NetCursor.hpp"

class WhiteBoard;

//The byte values for each known telnet command
enum NGT_Protocol
{
    NGTP_ECHO =                    1,  //sender echoes back all keystrokes
    NGTP_SUPPRESS_GO_AHEAD =       3,  //suppress go ahead (value 249) for full-duplex transmission
    NGTP_STATUS =                  5,  //discuss the status of options
    NGTP_TERMINAL_TYPE =          24,  //terminal-type option
    NGTP_NAWS =                   31,  //negotiate about window size, client tells server size
    NGTP_TERMINAL_SPEED =         32,  //terminal-speed option
    NGTP_TOGGLE_FLOW_CONTROL =    33,  //flow control using XON and XOFF to start and stop sending
    NGTP_X_DISPLAY_LOCATION =     35,  //X-display location <host>:<dispnum>[.<screennum>]
    NGTP_NEW_ENVIRON =            39,  //new values for environment variables (eg DISPLAY)
    NGTP_SE =                    240,  //end subnegotiation
    NGTP_DATA_MARK =             242,  //resume acting on non-telnet commands after a TCP urgent notification
    NGTP_GO_AHEAD =              249,  //end of input for half-duplex connections
    NGTP_SB =                    250,  //subnegotiation of the indicated option
    NGTP_WILL =                  251,  //will use, will begin performing
    NGTP_WONT =                  252,  //won't use, won't begin performing
    NGTP_DO =                    253,  //do
    NGTP_DONT =                  254,  //demand to stop using specified option
    NGTP_IAC =                   255   //interpret as command
};//NGT_Protocol

//ASCII Control characters understood by telnet
enum NGT_Control
{
    NGTC_NOP =                     0,  //do nothing? may be a request for feedback?
    NGTC_BELL =                    7,  //produce an audible or visible signal
    NGTC_BS =                      8,  //backspace, move cursor one to the left
    NGTC_LF =                     10,  //line feed, move down one line, stay in same column
    NGTC_CR =                     13,  //carriage return, move cursor to start of current line
    NGTC_SHIFT_OUT =              14,  //shift out,
    NGTC_SHIFT_IN =               15,  //shift in, return to previous character meaning?
    NGTC_ESC =                    27,  //escape, start of escape sequence
};//NGT_Control

//Byte values for a subnegotiation of an option
enum NGT_Negotiation
{
    NGTN_IS =                      0,
    NGTN_SEND =                    1,
    NGTN_INFO =                    2
};//NGT_Negotiation

//Each state in the FSM has a descriptive label based on the Telnet command
enum NGT_States
{
    NGTS_DISCONNECTED, NGTS_START, NGTS_ERROR, NGTS_IAC,
    NGTS_IAC_DO, NGTS_IAC_SB, NGTS_IAC_WILL, NGTS_IAC_DONT,
    NGTS_ESC, NGTS_IAC_SB_TERMSPEED, NGTS_IAC_SB_XDISPLOC, NGTS_IAC_SB_NEWENVIRON,
    NGTS_IAC_SB_TERMTYPE, NGTS_IAC_SB_TOGGLEFLOW
};//NGTP_States

class TelnetProtocol : public QObject
{
    Q_OBJECT

    public:
        //constructor
        TelnetProtocol(WhiteBoard *newWhiteBoard,
                       bool newDebug);


        //destructor
        ~TelnetProtocol(void);

        //configure and initialize the TCP network engine, returns false on errors
        //and couts a message
        bool initialize(NetCursor *newNetCursor);

        //Connect to the nethack server, returns false if we couldn't connect
        //and couts a message.
        //This is a blocking connect.
        bool connectToServer(const std::string &serverAddr);

        //Retrieve a byte from the telnet window
        uint8_t getByte(uint8_t xPos,
                        uint8_t yPos);

        //return the display attributes for the specified character
        SGRAttribute* getAttributes(uint8_t xPos,
                                    uint8_t yPos);

        //True if the telnet window contents have changed since the last time getDisplayChanged()
        //was called.
        bool getDisplayChanged(void);

        //Send the keyboard command to the server
        void sendKeystroke(uint8_t keyType);
        void sendCommand(const QString &command);
        //accessors
        uint8_t getWidth(void);
        uint8_t getHeight(void);
        TelnetWindow *getTelnetWindow(void);
        XtermEscape *getEscHandler(void);

        //Show a message that the client got telnet data outside of the window bounds.
        //Probably due to viewing a game with a large window.
        void showBoundsDialog(void);

        //Dimensions of the telnet window, in characters
        static const int WINDOW_WIDTH = 80;
        static const int WINDOW_HEIGHT = 24;

        //We'll try to send data to the server at this interval, in milliseconds
        static const int SEND_INTERVAL = 50;

    private slots:
        //Run the Finite State Machine for accepting Telnet commands and data
        //Event handler for received data
        void runFSM(void);

        //Event handler for socket disconnects
        void socketDisconnected(void);

        //Sends anything in queuedData to the server. Should only be called by the
        //sendTimer
        void sendData(void);

    private:
        //Pointer to the global whiteboard, don't delete
        WhiteBoard *whiteBoard;

        //Handles low-level networking
        QTcpSocket *tcpSocket;

        //Understands xterm escape sequences, responds appropriately
        XtermEscape *escHandler;

        //The 2D array of characters to display
        TelnetWindow *theWindow;

        //A blinking underscore representing the telnet cursor.
        //Just a pointer, don't delete
        NetCursor *netCursor;

        //Data to be sent to the server
        QByteArray sendQueue;

        //Our current state in the FSM
        NGT_States myState;

        //Our previous state in the FSM. If it changes, subState is set to zero.
        NGT_States prevState;

        //Periodically try to send data
        QTimer sendTimer;

        //Messages with more than 3 bytes of data are accepted using a separate sub-FSM for
        //each. subState is our current state in this sub-FSM.
        int subState;

        //the current byte we've received from the server
        uint8_t currentByte;

        //True if we're connecting to a new server, we shouldn't show a message about disconnecting
        //from the existing server, if any.
        volatile bool connecting;

        //Set to true if we've received a request for this option, and will respond
        bool willTerminalSpeed;
        bool willTerminalType;
        bool willXDisplayLocation;
        bool willNewEnviron;
        bool willEcho;
        bool willToggleFlowControl;

        //True if we should output verbose debugging messages
        bool debugMessages;

        //True if we should show an error dialog when the FSM runs into errors.
        //Ensures the dialog only pops up once.
        bool showError;

        //*************** FUNCTIONS ***************

        //Adds theData to the sendQueue and tries to send it immediately.
        //If all data couldn't be sent on the first try, periodically tries again until
        //the data is sent.
        void repeatSend(const QByteArray &theData);

        //One function for each state in the FSM
        void runStart(void);
        void runIAC(void);
        void runESC(void);
        void runIACDo(void);
        void runIACSB(void);
        void runIACWill(void);
        void runIACDont(void);
        void runIACSBTermSpeed(void);
        void runIACSBXdisploc(void);
        void runIACSBNewEnviron(void);
        void runIACSBTermType(void);
        void runIACSBToggleFlow(void);

        //Inform the user that we received an unknown message
        void showErrorDialog(const QByteArray &serverData);

};//TelnetProtocol

#endif
