#ifndef NG_LATENCY_WIDGET
#define NG_LATENCY_WIDGET

#include <QLabel>
#include <QTime>
#include <iostream>
#include <QTimer>

class WhiteBoard;

class LatencyWidget : public QLabel
{
    Q_OBJECT

    public:
        //constructor
        LatencyWidget(QWidget* parent,
                      Qt::WindowFlags flags,
                      WhiteBoard *newWhiteBoard);

        //destructor
        ~LatencyWidget(void);

        //Loads goImage and stopImage, returns false on error
        bool initialize(void);

        //Report that the user has performed a command, but hasn't yet received
        //a reply from the server
        void reportCommand(void);

        //Report that the connection has been lost
        void reportDisconnect(void);

        //Report that we've received data from the server
        void reportReply(void);

        //Use a running average with this many items to calculate latency
        static const int RUNNING_AVG_SIZE = 5;

        //If the average latency is <= this threshold, we won't show the stopImage
        //unless the latency threshold is passed for the current command. Time is in
        //milliseconds.
        static const int LATENCY_THRESHOLD = 100;

    public slots:
        //Event handler, called when this widget needs to be redrawn
        void paintEvent(QPaintEvent *event);

    private slots:
        //Event handler, called by updateTimer. Redraws the go/stop image.
        void doUpdate(void);

    private:
        //Pointer to the global white board, don't delete
        WhiteBoard *whiteBoard;

        //The time of the oldest unanswered command sent to the server
        QTime commandTime;

        //Image to display when ebonhack believes the screen is up to date
        QPixmap goImage;

        //Image to display when ebonhack believes that we're still waiting for data
        QPixmap stopImage;

        //If the average latency is <= LATENCY_TRESHOLD, we won't display the stopImage
        //immediately after commands are issued. Prevents flicker on fast connections.
        QTimer updateTimer;

        //The average latency to the server
        float averageLatency;

        //The width of the latency text in pixels, used to determine where we should draw
        //goImage and stopImage
        int textWidth;

        //True if the user sent a command to the server, and is waiting for a reply
        bool waitingReply;

        //############### FUNCTIONS ###############
        //Loads and scales the stop and go icons, returns false on failure and displays an
        //error message
        bool loadImages(void);

};//LatencyWidget

#endif
