/* DESCRIPTION
    A non-blocking message box. MessageForm is preferred to QMessageBox because
*/

#ifndef MESSAGEFORM_HPP_INCLUDED
#define MESSAGEFORM_HPP_INCLUDED

#include <QDialog>
#include <QString>
#include "ui_MessageDialog.h"

class MessageForm: public QDialog
{
    Q_OBJECT

    public:
        //constructor
        MessageForm(QWidget* parent,
                    Qt::WindowFlags flags);

        //destructor
        ~MessageForm(void);

        //Shows theMessage in a pop-up window. Only one message
        //can be displayed at a time.
        void showMessage(const QString &theMessage);

    private slots:
        void okButtonClicked(void);

    signals:

    private:
        //All of the widgets belonging to the connect form
        Ui::MessageDialog gui;

};//MessageForm

#endif // MESSAGEFORM_HPP_INCLUDED
