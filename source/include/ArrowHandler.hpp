/* DESCRIPTION
  Arrow keys are a special case of keyboard input, hitting up and left should send
  a single diagonal command to the server. Arrow key strokes are not sent
  immediately, we wait for NG_ARROW_TIME milliseconds to see if the player hits an
  arrow key with a perpendicular direction to the first.
*/

#ifndef ARROWHANDLER_HPP_INCLUDED
#define ARROWHANDLER_HPP_INCLUDED

#include <QKeyEvent>
#include <QTimer>

class WhiteBoard;

class ArrowHandler : public QObject
{
    Q_OBJECT

    public:
        //constructor
        ArrowHandler(WhiteBoard *newWhiteBoard);

        //destructor
        ~ArrowHandler(void);

        //Returns true if keypress is a directional arrow key, and sends it
        //to the server. Returns false otherwise, and does nothing with the keypress.
        //sendCommand() should be called whenever any key is pressed, even if it's
        //not an arrow key.
        bool sendCommand(int keypress);

        //Max number of milliseconds between pressing two arrow keys and having
        //them combined into a single diagonal movement.
        const static unsigned int ARROW_TIME = 120;

    private slots:
        //sends prevArrow to the server, sets bufferedArrow to false
        void sendBufferedArrow(void);

    signals:

    private:
        //Pointer to the global white board
        WhiteBoard *whiteBoard;

        //Check for 2-key combos. If an arrow key is pressed, wait before sending the command to the server.
        //If no other key is pressed after ARROW_TIME milliseconds, send the command. Otherwise, we have a
        //diagonal direction.
        QTimer arrowTimer;

        //The previous arrow key, used to get diagonal movement
        int prevArrow;

        //true if there's an arrow key that needs to be considered in prevArrow
        bool bufferedArrow;

        //*************** FUNCTIONS ***************

        //Set the value of prevArrow, sets bufferedArrow to false, updates arrowTimer
        void newBufferedArrow(int newArrow);

};//ArrowHandler

#endif
