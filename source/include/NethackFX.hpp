/* DESCRIPTION
    Analyzes the contents of the telnet window, and decides which parts
    should be represented by graphics, and which parts should be
    represented by text. Tries to map unknown telnet characters to
    graphics loaded from file.
*/

#ifndef NG_NETHACK_FX
#define NG_NETHACK_FX

#include <map>
#include <fstream>
#include <sstream>
#include <QTime>
#include <vector>

#include "TelnetWindow.hpp"
#include "CharSaver.hpp"
#include "RuleLoader.hpp"
#include "ImageLoader.hpp"

class ImageLoader;
class WhiteBoard;
class HistoryLog;

//States in the FSM. These are intended for communicating with the server
enum NGF_State
{
    //We aren't sending any queries
    NGF_START,

    //The following states are for sending different parts of the query, and waiting for server reply
    NGF_SEND_QUERY, NGF_SEND_DIRECTIONS,
    NGF_DELAY_SENDDIR, NGF_SEND_WHATIS, NGF_WAIT_DESCRIPTION,

    //Wait a small amount of time and go to NGF_START
    NGF_RESET
};//NGF_State

class NethackFX
{
    public:
        //constructor
        NethackFX(WhiteBoard *newWhiteBoard);

        //destructor
        ~NethackFX(void);

        //Initialize the graphics
        bool initialize(TelnetWindow *newWindow);

        //save knownChars to file
        void save(void);

        //Should be called when the telnet window's contents change.
        void updateLogic(TelnetWindow *theWindow);

        //Handle 'what is' queries to the server. Needs to be called periodically.
        void updateQuery(void);

        //Send a what is command to the server at the specified location in the telnet window
        //If mapUnknownChar is true, we'll try to assign a graphic to an unknown character
        void sendWhatIs(int charX,
                        int charY,
                        uint8_t telnetChar,
                        SGRAttribute *theAttributes,
                        bool mapUnknownChar);

        //Loads custom graphics from the specified file. filename should be the full path. Returns
        //false on failure and displays an error message
        bool loadCustomTiles(QString filename);

        //Loads the default tileset, returns false on failure and displays an error message
        bool loadDefaultTiles(void);

        //Given a telnet character and attributes, returns a pointer to the corresponding nethack image.
        //Returns NULL if no image mapping exists.
        QPixmap* findImage(uint8_t telnetChar,
                           SGRAttribute *theAttributes);

        //Return the graphic at the given index in the sprite list. Returns NULL if index is out of bounds.
        QPixmap* getImage(unsigned int index);

        //Display the window contents in the graphics scene
        void setGraphicsScene(QGraphicsScene *newScene);

        //Deletes and re-creates the history log, use when a new tileset is chosen
        void resetHistoryLog(QGraphicsScene *newScene);

        //Mutator
        void setUserFX(bool active);

        //The first row of nethack map, should be drawn in sprites
        static const int FIRST_FX_ROW = 1;

        //The row in the telnet window corresponding to the bottom of the gravestone
        static const int GRAVE_BOTTOM_ROW = 15;

        //The first row of player statistics, should be drawn in text
        static const int FIRST_STATS_ROW = 22;

        //Time in milleseconds to wait between consecutive data to the server
        static const int QUERY_TIME = 500;

        //Time in milliseconds to wait for a reply from the server
        static const int REPLY_GRACETIME = 5000;

  private:
        //Map character+color to an index containing the corresponding graphic
        //Shows users their recent history
        std::map <std::string, int> knownChars;

        //Pointer to the global whiteboard
        WhiteBoard *whiteBoard;

        //Reads rules for turning graphics mode on and off from file
        RuleLoader *ruleHandler;

        //Reads and writes known matches from char to graphic
        CharSaver *charHandler;

        //Displays the most recent nethack events at the top of the screen.
        //Each line in the history is a copy of the first line in the telnet window.
        HistoryLog *userHistory;

        //Pointer to the graphics scene containing the history log, don't delete
        QGraphicsScene *theScene;

        //Loads sprites from file
        ImageLoader *spriteHandler;

        //A 2D array of telnet characters, just a pointer don't delete
        TelnetWindow *theWindow;

        //Used to switch states in the FSM based on time elapsed
        QTime fsmTimer;

        //data to be sent to the server
        QByteArray query;

        //The top line of text from the telnet window. It may be temporarily
        //incomplete or malformed, say if we only got a part of the server's message.
        std::string firstLine;

        //We'll look for this text in GRAVE_BOTTOM_ROW, and disable graphics if we find it
        std::string graveBottom;

        //Current state in the FSM
        NGF_State myState;

        //coordinates of a telnet character we don't know how to convert to graphics
        uint8_t unknownX;
        uint8_t unknownY;

        //True if we should display graphics, false if everything should be text.
        //This variable is set based on the current rule in firstLine. If no rule is found,
        //lineRuleFX defaults to true.
        bool lineRuleFX;

        //True if we should display graphics, false if everything should be text.
        //This variable is set by the user, and takes precedence over lineRuleFX.
        bool userFX;

        //True if we're currently displaying graphics, false otherwise.
        bool showGraphics;

        //True if we're sending a query to map an unknown character to a graphic,
        //false if we're sending a simple 'what is' query for the user
        bool mappingChar;

        //False if there's a --More-- on the second line
        bool secondLineFX;

        //*************** Functions ***************

        //send cursor directions to the server as part of a what is? command
        void sendDirections(void);

        //Send a what is query to the server, if needed
        void queryServer(void);

        //Extract a name from firstLine. The result is any text inside the first pair
        //of brackets ()
        std::string extractName(void);

        //Sets showGraphics based on userFX and lineRuleFX, and refreshes the display if needed.
        void setGraphicsMode(void);

        //Add a description to knownChars, false if none found
        bool addDescription(const std::string &theName);

        //Check for names of the form "An elven wizard called Squinkles"
        //Returns the word immediately before theSuffix, or empty string if not found.
        std::string checkSuffix(const std::string &theName,
                                const std::string &theSuffix);

        //Check for names of the form "tame kitten" or "peaceful dwarf"
        std::string checkPrefix(const std::string &theName,
                                const std::string &thePrefix);

        //Search for theName in the mapping of string to spriteIndex. If found, return true
        //and store this mapping in tempChars or knownChars.
        bool addMapping(std::string theKey,
                        std::string theName);

        //Disable graphics on the second line if it contains a --More--
        void handleExtendedMore(TelnetWindow *theWindow);

        //Disable graphics when the character dies
        void handleGravestone(TelnetWindow *theWindow);

};//NethackFX

#endif
