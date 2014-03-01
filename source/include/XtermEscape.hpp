/* DESCRIPTION

    An FSM that understands Xterm Escape Sequences.
    All sequences start with the ESC char (ASCII 27), which should be
    accepted by the calling class before run() is called.

    This class only understands a subset of the xterm sequences, which should
    be enough to run nethack. There are known bugs when viewing other people's
    games. Unknown sequences are handled in two ways:

    1) If ignoring the sequence has been found not to corrupt the screen content, we
       ignore it and cout a warning.

    2) If the sequence changes the screen content, or is completely unknown, we pop up
       an error message and tell the user that the display may not reflect the game contents.
*/

#ifndef NG_XTERM_ESC
#define NG_XTERM_ESC

#include <iostream>
#include <deque>

#include "DisplayRow.hpp"
#include "SGRAttribute.hpp"
#include "TelnetWindow.hpp"
#include "NGSettings.hpp"

class WhiteBoard;

//### States in the xterm escape sequence FSM ###
enum NGX_State
{
    NGXS_START, NGXS_ERROR, NGXS_G0_CHAR_SET, NGXS_G1_CHAR_SET,
    NGXS_CSI, NGXS_CSI_DEC, NGXS_CSI_PARAM, NGXS_OSC,
    NGXS_OSC_PARAM
};//NGX_State

class XtermEscape
{
    public:
        //constructor
        XtermEscape(TelnetWindow *newWindow,
                    WhiteBoard *newWhiteBoard,
                    bool newDebug);

        //destructor
        ~XtermEscape(void);

        //Run the FSM, potentially modifying the write location and the telnet window.
        //Returns the current state of the FSM, NGXS_START if we successfully accepted this
        //escape sequence, and NGXS_ERROR if the escape sequence was not understood. If the
        //result is any other state, then XtermEscape is expecting more bytes.
        NGX_State run(uint8_t currentByte);

        //Reset the finite state machine to it's original state, useful if connecting to
        //a different server
        void resetFSM(void);

        //Returns true if we should use the tile specified by getTileNumber
        bool getUseTileNumber(void);

        //Returns the current tile number that the server told us to use
        int getTileNumber(void);

    private:
        //parameters sent by the server
        std::deque <QByteArray*> parameters;

        //pointer to the global white board
        WhiteBoard *whiteBoard;

        //The 2D array of characters to display, just a pointer don't delete
        TelnetWindow *theWindow;

        //Our current state in the FSM
        NGX_State myState;

        //The saved cursor coordinates
        uint8_t savedCursorX;
        uint8_t savedCursorY;

        //Top and bottom of the scrolling region
        int topScrolling;
        int bottomScrolling;

        //The nethack tile the server is suggesting we use for future telnet characters
        int tileNumber;

        //True if we're using the alternate buffer, whatever that is
        bool alternateBuffer;

        //Returns true if currentByte is a value from '0' to '9'
        bool isASCIIDigit(uint8_t currentByte);

        //True if we should output verbose messages for debugging
        bool debugMessages;

        //True if tileNumber represents a tile to match to received telnet characters,
        //false otherwise
        bool useTileNumber;

        //Only show a message about set window unhandled once
        bool showedWindowMsg;

        //############### FUNCTIONS ###############

        //One function for each state in the FSM
        void runStart(uint8_t currentByte);
        void runG0CharSet(uint8_t currentByte);
        void runG1CharSet(uint8_t currentByte);
        void runCSI(uint8_t currentByte);
        void runCSIDEC(uint8_t currentByte);
        void runDECPMSet(int decSet);
        void runDECPMReset(int decSet);
        void runCSIParam(uint8_t currentByte);
        void runCSISetScrolling(void);
        void runSGRCharAttributes(void);
        void runCSIResetMode(void);
        void runOSC(uint8_t currentByte);
        void runOSCParam(uint8_t currentByte);

        //Execute a command
        void setCursorPosition(void);
        void eraseInDisplay(void);
        void linePosition(void);
        void eraseInLine(void);
        void cursorPosition(void);
        void cursorCharAbsolute(void);
        void deleteCharacters(void);
        void cursorForward(void);
        void cursorUp(void);
        void cursorDown(void);
        void cursorBackward(void);
        void deleteLines(void);
        void setNethackTile(void);

        //Run an operating system command
        void handleOSC(void);

        //Set the Nethack tile to use for subsequent characters
        void startGlyph(void);
        void endGlyph(void);

        //Extract the cursor movement distance from parameters
        int getCursorMoveDist(void);

        //Converts parameter to an integer, stored in intVersion. Returns false
        //if parameter didn't contain a base-10 ASCII integer, eg "456".
        bool extractInt(QByteArray *oneParameter,
                        int &intVersion);

        //Returns parameter as a std string. If parameter contains any NULL characters,
        //the result may not contain the entire parameter.
        std::string arrayToStr(QByteArray *parameter);

        //Clear parameters sent by the server
        void resetParameters(void);

};//XtermEscape

#endif
