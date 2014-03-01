#ifndef NETCURSOR_HPP_INCLUDED
#define NETCURSOR_HPP_INCLUDED

#include <QGraphicsRectItem>
#include <QTimer>
#include <QFont>
#include <stdint.h>

class WhiteBoard;
class TelnetWindow;

class NetCursor : public QGraphicsRectItem
{
    public:
        //constructor
        //newXPos and newYPos are the sprite's position in the telnet window,
        //NOT the pixel coordinates
        NetCursor(QGraphicsItem *parent,
                  WhiteBoard *newWhiteBoard);

        //destructor
        ~NetCursor(void);

        //Draws the cursor
        void paint(QPainter *painter,
                   const QStyleOptionGraphicsItem *option,
                   QWidget *widget);

        //Returns the bounding rectaingle, in item coordinates
        QRectF boundingRect(void);

        //Change the cursor color
        void setColor(Qt::GlobalColor newColor);

        //Moves the cursor to the specified telnet location
        void setCursorPos(uint8_t newX,
                          uint8_t newY);

        //Height of the cursor in pixels
        static const int CURSOR_HEIGHT = 2;

    private:
        //Pointer to the global whiteboard, don't delete
        WhiteBoard *whiteBoard;

        //Pointer to the telnet window containing nethack sprites
        TelnetWindow *telnetWindow;

        //The color to draw the cursor
        Qt::GlobalColor cursorColor;

        //Detect when the cursor moves
        uint8_t oldX;
        uint8_t oldY;

};//NetCursor

#endif // NETCURSOR_HPP_INCLUDED
