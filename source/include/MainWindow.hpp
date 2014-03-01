/*  DESCRIPTION

    The main window for the application. Contains the menu bar and the graphicsView
    widget that displays the telnet window. If the graphicsView has keyboard focus,
    arrow keys will scroll through the view, instead of moving the character.

    To prevent this, the graphics view is never allowed to gain focus. A side effect
    is that no other widget in the MainWindow can ever have keyboard focus either, or
    it would claim every keypress for itself.
*/

#ifndef MAINWINDOW_HPP_INCLUDED
#define MAINWINDOW_HPP_INCLUDED

#include <QMainWindow>
#include <QGraphicsScene>
#include <QSignalMapper>
#include <QTimer>
#include <QGraphicsTextItem>
#include "ui_MainWindow.h"

class WhiteBoard;
class KeyboardHandler;
class NethackFX;
class NGSettings;
class ConnectForm;
class ArrowHandler;
class ZoomForm;
class NetCursor;
class TipForm;
class LatencyWidget;
class FarmDockWidget;
class GraphicsSettings;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        //constructor
        MainWindow(QWidget* parent,
                   Qt::WindowFlags flags,
                   WhiteBoard *newWhiteBoard);

        //destructor
        ~MainWindow(void);

        //Initialize the game, returns false on failure and couts a message
        bool start(void);

        //Deletes tiles and history log and re-creates them. Used whenever the tileset
        //changes.
        void resetGraphics(void);

        void alertTilesFinished(void);

        //Initialize Allegro's graphics, timers and input
        bool initAllegro(void);

        //If we connected to the server, report it here
        void reportConnection(void);

        //Enables or disables nethack graphics
        void setGraphicsMode(bool newValue);

        //accessors
        LatencyWidget *getLatencyWidget(void);
        FarmDockWidget *getFarmingWidget(void);
        GraphicsSettings *getGraphicsSettings(void);

        //Pause the cursor and changes it red while waiting for a reply
        void pauseCursor(void);

        //Turns the cursor white, cause it to start blinking in and out
        void restoreCursor(void);

        //The minimum allowed window dimensions in pixels
        static const unsigned int MIN_XRESOLUTION = 496;
        static const unsigned int MIN_YRESOLUTION = 368;

        //Periodically update the game logic, time is in milliseconds
        static const unsigned int LOGIC_UPDATE_TIME = 100;

        //Center the blinking cursor below the character
        static const int CURSOR_X_OFFSET = 1;
        static const int CURSOR_Y_OFFSET = -4;

        //Number of milliseconds to wait between alternating the cursor. This controls
        //the frequency at which it blinks.
        static const unsigned int CURSOR_TIME = 400;

        //Rough estimate of the window border's thickness, including the menu bar
        static const unsigned int BORDER_WIDTH = 60;
        static const unsigned int BORDER_HEIGHT = 90;

    private slots:
        void menuCommand(const QString &param);
        void showConnectForm(void);
        void showZoomForm(void);
        void toggleGraphics(bool enableGraphics);
        void showManual(void);
        void showTipOfTheDay(void);
        void showGraphicsSettings(void);
        void quitClicked(void);

        //update the game logic
        void updateLogic(void);

        //Shut the program down when the main window is closed
        void closeEvent(QCloseEvent *event);

        //Causes the telnet cursor to blink in and out
        void blinkCursor(void);

    public slots:
        void zoomFormNewValue(void);

    signals:
        void tilesFinished();
    protected:
        //Causes keystrokes to ignore the filtered object(s)
        bool eventFilter(QObject *obj,
                         QEvent *event);

    private:
        //Pointer to the global whiteboard, don't delete
        WhiteBoard *whiteBoard;

        //Sends directional arrow commands to the server, handles diagonal
        //movement when two keys are pressed simultaneously
        ArrowHandler *arrowHandler;

        //contains settings that affect multiple classes
        NGSettings *gameSettings;

        //Lets us specify parameters for signals emitted by menu items
        QSignalMapper *menuMapper;

        //Allows the user to enter a host name and connect to the server
        ConnectForm *connectForm;

        //Displays the average latency, assuming that any data sent to the server should
        //immediately be followed by some sort of reply. Shows red until the most recent
        //user command has been responded to.
        LatencyWidget *latencyWidget;
        FarmDockWidget *farmingWidget;

        //Allows the user to choose a custom tileset and set the video mode to OpenGL or Qt
        GraphicsSettings *graphicsSettings;

        //Zooms the graphicsView in or out. We can't put the zoom box right in the main window,
        //since it requires keyboard focus.
        ZoomForm *zoomForm;

        //A blinking underscore representing the telnet cursor
        NetCursor *netCursor;

        //Shows a tip-of-the-day message
        TipForm *tipForm;

        //manages the images to be displayed in the GraphicsView
        QGraphicsScene theScene;

        //Causes the telnet cursor to blink in and out
        QTimer cursorTimer;

        //All of the widgets belonging to the main window
        Ui::MainWindow gui;

        //Runs updateLogic every LOGIC_UPDATE_TIME milliseconds
        QTimer logicTimer;

        //True if we're connected to a nethack server, false otherwise
        bool isConnected;

        //############### FUNCTIONS ###############

        //Obtain input from the keyboard
        void keyPressEvent(QKeyEvent *event);
        void keyReleaseEvent(QKeyEvent *keyEvent);

        //Write the contents of the telnet window to dump.txt
        void dumpScreen(void);

        //Set up the QGraphicsView and QGraphicsScene
        void configureScene(void);

        //Given a capital letter, returns a string containing the ASCII representation
        //of the letter if CTRL is held.
        std::string getCTRL(char letter);

        //Connect each item in the top menu bar to a slot, and specify the keystroke(s)
        //to send to the server when the item is clicked.
        void addMenuBindings(void);

        //The specified keyStrokes will be sent to the server when the menuItem is clicked
        void addBinding(QAction* menuItem,
                        const std::string &keyStrokes);

        //Sets the size of the scene based on the sprite size and number of sprites in the
        //telnet window. Includes the history log.
        void setSceneSize(void);

};//MainWindow

#endif // MAINWINDOW_HPP_INCLUDED


