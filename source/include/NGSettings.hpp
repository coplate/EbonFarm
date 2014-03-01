#ifndef NG_SETTINGS
#define NG_SETTINGS

#include <QString>
#include <QFont>
#include "ConfigWriter.hpp"
#include <stdint.h>

class NGSettings
{
    public:
        //constructor
        NGSettings(void);

        //destructor
        ~NGSettings(void);

        //Updates the font family and size. minSpriteSize is used to auto-size the font, but custom
        //fonts ignore it and load their sizes from config instead.
        static void updateFontFromConfig();

        //Index to use if the sprite for a telnet character is unknown
        static const int TELNET_UNKNOWN = 65535;

        //ASCII value for the enter key
        static const int ENTER_KEY = 13;

        //Location of the data files
        static const std::string DATA_PATH;

        //returns the key to use for directions
        static char getUpLeftKey(void);
        static char getUpKey(void);
        static char getUpRightKey(void);
        static char getRightKey(void);
        static char getDownRightKey(void);
        static char getDownKey(void);
        static char getDownLeftKey(void);
        static char getLeftKey(void);

        //Returns true if we're using the number pad for directional input
        static bool getUseNumpad(void);

        //Returns the font for displaying telnet characters
        static QFont* getTelnetFont(void);

        //Returns the vertical offset for the telnet font. Adjusts the font position so that
        //characters are centered in their tiles.
        static int getFontOffset(void);

        //Reloads the number of history lines from config file.
        void reloadHistoryLines(void);

        //Mutator
        static void setSpriteSize(unsigned int newWidth,
                                  unsigned int newHeight);

        //Accessors
        static unsigned int getSpriteWidth(void);
        static unsigned int getSpriteHeight(void);
        static unsigned int getHistoryLines(void);

    private:
        //The font for displaying telnet characters
        static QFont *telnetFont;

        //The size of telnet sprites, in pixels
        static unsigned int spriteWidth;
        static unsigned int spriteHeight;

        //The number of lines of history to display in the history log
        static unsigned int historyLines;

        //Amount to vertically offset the telnet font, in pixels. Needed because
        //the font does not always center in the tile.
        static int fontOffset;

        //The key to use for each direction
        static char upLeftKey;
        static char upKey;
        static char upRightKey;
        static char rightKey;
        static char downLeftKey;
        static char downKey;
        static char downRightKey;
        static char leftKey;

        //True if we're using the number pad for directional input, false otherwise
        static bool useNumpad;

        //set the direction keys to either the numpad or keyboard values
        void setKeys(void);

        //Adjusts the telnet character font size and offset to match the minSpriteDimension. Only the offset is adjusted
        //for custom fonts.
        static void setFontSize();

};//NGSettings

#endif
