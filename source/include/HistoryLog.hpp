#ifndef HISTORYLOG_HPP_INCLUDED
#define HISTORYLOG_HPP_INCLUDED

#include <string>
#include <QGraphicsScene>
#include "SGRAttribute.hpp"

class WhiteBoard;
class DisplayRow;

class HistoryLog
{
    public:
        //constructor
        HistoryLog(WhiteBoard *newWhiteBoard);

        //destructor
        ~HistoryLog(void);

        //Display the window contents in the graphics scene
        void setGraphicsScene(QGraphicsScene *theScene);

        //Remove the window contents from the graphics scene
        void removeFromScene(QGraphicsScene *theScene);

        //Should be called whenever the telnet window changes
        //Adds newLine to the user's history, if appropriate. New lines are not added to the history
        //when showGraphics is false.
        void updateLogic(const std::string &newLine,
                         bool showGraphics);

        //Display or hide the history log
        void show(void);
        void hide(void);

        //Sets the user history to blank spaces
        void clearHistory(void);

    private:
        //Whenever the top line changes, add it to history. Shows the user's most
        //recent events.
        std::vector <DisplayRow*> history;

        //A copy of the contents of history. Used to restore the history log if it has been
        //hidden
        std::vector <std::string*> historyBuf;

        //pointer to the global whiteboard
        WhiteBoard *whiteBoard;

        //the Select Graphic Rendition attributes for the user's history
        SGRAttribute logAttributes;

        //A copy of first line to be added to the user's history. We buffer it in oldHistoryLine
        //so that it isn't displayed at the same time as the (identical) firstLine.
        std::string oldHistoryLine;

        //If the NetSprites were added to a scene, the scene will delete them when it is destroyed.
        //Otherwise, we must delete them.
        bool addedToScene;

        //true if the log is hidden, false if it is being displayed
        bool hidden;

};//HistoryLog

#endif // HISTORYLOG_HPP_INCLUDED
