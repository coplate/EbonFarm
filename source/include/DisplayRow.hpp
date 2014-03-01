/* DESCRIPTION

  A horizontal row of telnet characters, including their attributes such as
  bold, invisible, color, etc.

*/

#ifndef NG_DISPLAY_ROW
#define NG_DISPLAY_ROW

#include <iostream>
#include <vector>
#include <stdint.h>
#include <QGraphicsScene>

class NetSprite;
class SGRAttribute;
class WhiteBoard;

class DisplayRow
{
    public:
        //constructor
        DisplayRow(WhiteBoard *newWhiteBoard,
                   uint8_t newSize,
                   int y);

        //destructor
        ~DisplayRow(void);

        //set the character at the specified index
        void setChar(uint8_t index,
                     uint8_t value,
                     SGRAttribute *writeAttributes);

        //retrieve the character at the specified index
        uint8_t getChar(uint8_t index);

        //Retrieve the nethack sprite at the specified index
        NetSprite* getNetSprite(uint8_t index);

        //retrieve the display attributes for the char at the specified index
        SGRAttribute* getAttributes(uint8_t index);

        //Returns the number of elements in the DisplayRow
        unsigned int size(void);

        //Removes all contained sprites from the specified graphics scene
        void removeFromScene(QGraphicsScene *theScene);

    private:
        //Pointer to the global whiteboard, don't delete
        WhiteBoard *whiteBoard;

        //A row of character data, including associated graphic
        std::vector<NetSprite*> theSprites;

};//DisplayRow

#endif
