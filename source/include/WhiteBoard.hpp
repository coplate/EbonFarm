#ifndef WHITEBOARD_HPP_INCLUDED
#define WHITEBOARD_HPP_INCLUDED

#include <iostream>
#include <QRegExp>

class QApplication;
class MainWindow;
class QString;
class NethackFX;
class TelnetProtocol;
class MessageForm;
class QWidget;

class WhiteBoard
{
    public:
        //constructor
        WhiteBoard(QApplication *newQtApp,
                   bool debugMode);

        //destructor
        ~WhiteBoard(void);

        //Start the QT application engine
        int run(void);

        //Output the error message and crash gracelessly, intended to catch internal bugs,
        //NOT error conditions
        static void fatalError(const QString &errMsg);

        //Displays theMessage to the user in a pop up window.
        void showMessage(const QString &theMessage);

        //Returns the position of the first match, or -1 if there was no match.
        static int indexIn(const std::string &regExp,
                           const std::string &theString);

        //Accessors
        QApplication* getQTApp(void);
        MainWindow* getMainWindow(void);
        NethackFX* getNetFX(void);
        TelnetProtocol* getTelnetPro(void);
        MessageForm* getMessageForm(void);

    private:
        //The Qt engine that runs the program
        QApplication *qtApp;

        //The main window for the application
        MainWindow *mainWindow;

        //Understands the telnet protocol, handles socket networking
        TelnetProtocol *telnetPro;

        //adds graphics to the nethack interface
        NethackFX *netFX;

        //A non-blocking dialog with an OK button, used to show messages to
        //the user.
        MessageForm *messageForm;

        //Detects matches for the indexIn function
        static QRegExp regExpRule;

};//WhiteBoard

#endif // WHITEBOARD_HPP_INCLUDED
