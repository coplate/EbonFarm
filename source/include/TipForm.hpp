/*Copyright 2009 David McCallum

This file is part of EbonHack.

    EbonHack is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    EbonHack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with EbonHack.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TIPFORM_HPP_INCLUDED
#define TIPFORM_HPP_INCLUDED

#include <QDialog>
#include "ui_TipForm.h"

class TipForm : public QDialog
{
    Q_OBJECT

    public:
        //constructor
        TipForm(QWidget* parent,
                Qt::WindowFlags flags);

        //destructor
        ~TipForm(void);

    public slots:
        //load a tip and show the form
        void displayTip(void);

    private slots:
        void tipsBoxChanged(int state);
        void okButtonPressed(void);

    private:
        //All of the widgets belonging to the tip of the day form
        Ui::TipDialog gui;

};//TipForm

#endif // TIPFORM_HPP_INCLUDED
