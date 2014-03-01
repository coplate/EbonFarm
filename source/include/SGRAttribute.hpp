/* DESCRIPTION

  Attributes that modify the way a character should be displayed on the
  screen. These are: color, bold, invisible, underlined, and inverse.
*/

#ifndef NG_SGR_ATTRIBUTE
#define NG_SGR_ATTRIBUTE

#include <iostream>

class TelnetWindow;

//One value for each known color. Need to modify CharSaver::colorNames
//if this is ever altered.
enum NGS_Attribute
{
    NGSA_BLACK = 0,
    NGSA_RED = 1,
    NGSA_GREEN = 2,
    NGSA_YELLOW = 3,
    NGSA_BLUE = 4,
    NGSA_MAGENTA = 5,
    NGSA_CYAN = 6,
    NGSA_WHITE = 7
};//NGS_Attribute

//Select Graphic Rendition parameters
enum NGC_Attributes
{
    NGC_NORMAL =              0,  //default
    NGC_BOLD =                1,
    NGC_UNDERLINED =          4,
    NGC_BLINK =               5,  //appears as Bold
    NGC_INVERSE =             7,
    NGC_INVISIBLE =           8,  //hidden
    NGC_NORMAL2 =            22,  //neither bold nor faint
    NGC_NOT_UNDERLINED =     24,
    NGC_STEADY =             25,  //not blinking
    NGC_POSITIVE =           27,  //not inverse
    NGC_VISIBLE =            28,  //not hidden

    NGC_FOREGROUND_BLACK =   30,
    NGC_FOREGROUND_RED =     31,
    NGC_FOREGROUND_GREEN =   32,
    NGC_FOREGROUND_YELLOW =  33,
    NGC_FOREGROUND_BLUE =    34,
    NGC_FOREGROUND_MAGENTA = 35,
    NGC_FOREGROUND_CYAN =    36,
    NGC_FOREGROUND_WHITE =   37,
    NGC_FOREGROUND_DEFAULT = 39,  //original

    NGC_BACKGROUND_BLACK =   40,
    NGC_BACKGROUND_RED =     41,
    NGC_BACKGROUND_GREEN =   42,
    NGC_BACKGROUND_YELLOW =  43,
    NGC_BACKGROUND_BLUE =    44,
    NGC_BACKGROUND_MAGENTA = 45,
    NGC_BACKGROUND_CYAN =    46,
    NGC_BACKGROUND_WHITE =   47,
    NGC_BACKGROUND_DEFAULT = 49   //original
};//NGC_Attributes

class SGRAttribute
{
    public:
        //constructor
        SGRAttribute(void);

        //destructor
        ~SGRAttribute(void);

        //set all values to match those of the source
        void setValues(SGRAttribute *source);

        //sets all attributes to their default values
        void setDefault(void);

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

        //accessors
        bool getBold(void);
        bool getInverse(void);
        bool getInvisible(void);
        bool getUnderlined(void);
        NGS_Attribute getForeground(void);
        NGS_Attribute getBackground(void);

        //Accepts an xterm attribute, and relays it to the telnet window.
        //Returns false if the parameter wasn't recognized
        static bool acceptAttribute(int parameter,
                                    TelnetWindow *theWindow,
                                    bool debugMessages);

    private:
        //character properties
        bool bold;//blink appears as bold
        bool underlined;
        bool inverse;//inverse vs positive
        bool invisible;//visible vs invisible

        //the character color
        NGS_Attribute foregroundColor;
        NGS_Attribute backgroundColor;

};//SGRAttribute

#endif
