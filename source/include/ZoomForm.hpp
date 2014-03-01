#ifndef ZOOMFORM_HPP_INCLUDED
#define ZOOMFORM_HPP_INCLUDED

#include <QDialog>
#include "ui_ZoomForm.h"

class MainWindow;

class ZoomForm : public QDialog
{
    Q_OBJECT

    public:
        //constructor
        ZoomForm(QWidget* parent,
                 Qt::WindowFlags flags,
                 MainWindow *mainWindow);

        //destructor
        ~ZoomForm(void);

        //Returns the zoom box's value
        int getZoomAmount(void);

    private slots:
        void cancelButtonClicked(void);

    signals:

    private:
        //All of the widgets belonging to the zoom form
        Ui::ZoomDialog gui;
};//ZoomForm

#endif // ZOOMFORM_HPP_INCLUDED
