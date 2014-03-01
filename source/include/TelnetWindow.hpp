/* DESCRIPTION

  A 2D array of telnet characters, representing the information that should
  be displayed on the screen. (virtual terminal) Methods are provided for
  modifying the data according to Telnet and Xterm protocols.
*/

#ifndef NG_TELNET_WINDOW
#define NG_TELNET_WINDOW

#include <vector>
#include <QGraphicsScene>

#include "DisplayRow.hpp"
#include "SGRAttribute.hpp"

class WhiteBoard;

class TelnetWindow
{
    public:
        //constructor
        TelnetWindow(WhiteBoard *newWhiteBoard);

        //destructor
        ~TelnetWindow(void);

        //create a new telnet window with the specified width and height
        bool initialize(uint8_t newWidth,
                        uint8_t newHeight);

        //Resets the telnet window, should be called when a new tileset is chosen.
        void resetWindow(QGraphicsScene *newScene);

        //return the character at the specified location
        uint8_t getByte(uint8_t xPos,
                        uint8_t yPos);

        //return the display attributes for the specified character
        SGRAttribute* getAttributes(uint8_t xPos,
                                    uint8_t yPos);

        //Write a character to cursorX,cursorY in theWindow, and advance cursorX and cursorY
        void writeByte(uint8_t oneByte);

        //Run the 'erase all' command - erase the whole screen
        void eraseAll(void);

        //Run the 'erase to right' command - erase everything to the right of the cursor?
        //Does this include the cursor position?
        void eraseToRight(void);

        //Run the 'erase below' command - erase everything below the cursor?
        //Does this include the cursor position?
        void eraseBelow(void);

        //Run the 'delete characters' command - erase numDelete to the right of the cursor?
        void deleteCharacters(int numDelete);

        //Run the 'delete lines' command
        void deleteLines(int numDelete);

        //accessors
        uint8_t getWidth(void);
        uint8_t getHeight(void);

        //Returns the coordinates of the telnet cursor
        uint8_t getCursorX(void);
        uint8_t getCursorY(void);

        //mutators
        void setCursorX(uint8_t newValue);
        void setCursorY(uint8_t newValue);
        void enableEraseAll(bool enabled);

        //move the cursor by the specified amount
        void moveCursorX(int amount);
        void moveCursorY(int amount);

        //reset the SGR write attributes to default
        void setDefaultAttribute(void);

        //Set display styles
        void setBold(void);
        void setInverse(void);
        void setInvisible(void);
        void setUnderlined(void);

        //clear display styles
        void clearBold(void);
        void clearInverse(void);
        void clearInvisible(void);
        void clearUnderlined(void);

        //set text and background color
        void setForeground(NGS_Attribute color);
        void setBackground(NGS_Attribute color);

        //True if the telnet window contents have changed since the last time getDisplayChanged()
        //was called.
        bool getDisplayChanged(void);

        //returns true if the telnet or its attributes changed since charChanged was called
        bool charChanged(uint8_t x,
                         uint8_t y);

        //Enables or disables graphics for the specified rows from the telnet window
        void setGraphicsMode(bool useGraphics,
                             uint8_t topRow,
                             uint8_t numRows);

        //Tell the sprite at this location that its assigned graphic has changed.
        //Schedules a redraw.
        void notifyFxChange(int x,
                            int y);

    private:
        //A 2-D array of characters to be displayed.
        std::vector <DisplayRow*> theWindow;

        //Pointer to the global whiteboard, don't delete
        WhiteBoard *whiteBoard;

        //Pointer to the graphics scene containing this window, don't delete
        QGraphicsScene *theScene;

        //the current Select Graphic Rendition attribute
        SGRAttribute writeAttribute;

        //the width and height of the telnet window, in characters
        uint8_t windowWidth;
        uint8_t windowHeight;

        //x and y location to write to in the telnet window
        uint8_t writeX;
        uint8_t writeY;

        //True if the telnet window contents have changed. Set to false when getDisplayChanged()
        //is called.
        bool displayChanged;

        //True if eraseAll() should erase the display as normal, false otherwise
        bool allowEraseAll;

        //True if this is the first time eraseAll was called, false otherwise
        bool firstEraseAll;

        //############### FUNCTIONS ###############

        //Deletes all items in aWindow, removing them from theScene first. Sets theScene to NULL.
        void clearWindow(std::vector <DisplayRow*> aWindow);

        //Display the window contents in the graphics scene
        void setGraphicsScene(QGraphicsScene *newScene);

};//TelnetWindow

#endif
