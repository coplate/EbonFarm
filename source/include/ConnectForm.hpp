/* DESCRIPTION
    The connect form that allows the user to specify a nethack server.
*/

#ifndef CONNECTFORM_HPP_INCLUDED
#define CONNECTFORM_HPP_INCLUDED

#include <QDialog>
#include "ui_ConnectForm.h"

class WhiteBoard;

class ConnectForm : public QDialog
{
    Q_OBJECT

    public:
        //constructor
        ConnectForm(QWidget* parent,
                    Qt::WindowFlags flags,
                    WhiteBoard *newWhiteBoard);

        //destructor
        ~ConnectForm(void);

    private slots:
        void cancelButtonClicked(void);
        void connectToServer(void);

        //Enables the connect button if a host name has been entered,
        //disables it otherwise
        void toggleConnectButton(void);

    signals:

    private:
        //Pointer to the global white board
        WhiteBoard *whiteBoard;

        //All of the widgets belonging to the connect form
        Ui::ConnectForm gui;

};//ConnectForm

#endif // CONNECTFORM_HPP_INCLUDED
