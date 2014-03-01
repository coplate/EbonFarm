/*  DESCRIPTION
    Represents a single nethack character from the telnet window. Characters are either shown
    by a graphic (thePixmap) or text (displayText). displayText acts as a buffer for the character,
    since drawing text to the scene is very slow.
*/


#ifndef NETSPRITE_HPP_INCLUDED
#define NETSPRITE_HPP_INCLUDED

#include <QGraphicsTextItem>
#include <QImage>
#include <stdint.h>

class SGRAttribute;
class WhiteBoard;
class TelnetWindow;

class NetSprite: public QGraphicsRectItem
{
    public:
        //constructor
        //newXPos and newYPos are the sprite's position in the telnet window,
        //NOT the pixel coordinates
        NetSprite(QGraphicsItem *parent,
                  WhiteBoard *newWhiteBoard,
                  int newXPos,
                  int newYPos);

        //destructor
        ~NetSprite(void);

        //Sets the background character color
        static void initialize(void);

        //Draws the sprite to the graphicsview
        void paint(QPainter *painter,
                   const QStyleOptionGraphicsItem *option,
                   QWidget *widget);

        //Sets the telnet character to the newValue and SGR attributes
        void setChar(uint8_t newValue,
                     SGRAttribute *writeAttributes);

        //Set graphicsOn to true if we should display the graphic for this telnet character.
        //The character will still be displayed if there is no image for it.
        void setGraphicsMode(bool graphicsOn);

        //returns true if the telnet character has been written to since the last
        //call to getIsChanged()
        bool getIsChanged(void);

        //Accessors
        uint8_t getChar(void);
        SGRAttribute* getAttributes(void);

        //Event handlers for mouse presses and releases
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        //Returns the bounding rectangle for this item
        QRectF boundingRect(void);

        //Inform the sprite that the graphic for this character has changed.
        //Schedules a redraw.
        void changeDisplayGraphic(void);

        //Center text characters in the pixmap
        const static int CHAR_X_OFFSET = -1;
        const static int CHAR_Y_OFFSET = -4;

    private:
        //Pointer to the global whiteboard, don't delete
        WhiteBoard *whiteBoard;

        //Pointer to the telnet window containing nethack sprites
        TelnetWindow *telnetWindow;

        //Defines how the telnet character should be displayed
        SGRAttribute* theAttributes;

        //A pointer to the sprite's graphic, don't delete.
        QPixmap *thePixmap;

        //QString version of the telnet character
        QPixmap displayText;

        //The telnet character
        uint8_t telnetChar;

        //The dimensions of the sprite in pixels
        unsigned int spriteWidth;
        unsigned int spriteHeight;

        //The coordinates of this character in the graphics scene, in character coordinates
        int xPos;
        int yPos;

        //True if the telnet character or its attributes changed since
        //getIsChanged() was called
        bool isChanged;

        //True if we have a graphic for this character, false otherwise
        bool haveGraphic;

        //True if we should display the graphic, if we have one
        bool useGraphic;

        //True if the pixmap and char have been added to a scene and we shouldn't
        //delete them
        bool addedToScene;

        //############### FUNCTIONS ###############

        //Draws the telnet character onto displayText
        void changeDisplayChar(void);

        //Indicate that the sprite contents have changed and should be redrawn
        void requestRedraw(void);

};//class NetSprite

#endif // NETSPRITE_HPP_INCLUDED
