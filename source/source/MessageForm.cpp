/*Copyright 2009-2013 David McCallum

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

#include "MessageForm.hpp"
#include <iostream>

using namespace std;

MessageForm::MessageForm(QWidget* parent,
                         Qt::WindowFlags flags) : QDialog(parent, flags)
{
    //### Configure GUI ###
    gui.setupUi(this);
    connect(gui.okButton, SIGNAL(clicked()),
            this, SLOT(okButtonClicked()));
}//constructor

MessageForm::~MessageForm(void)
{
}//destructor

void MessageForm::okButtonClicked(void)
{
    hide();
}//okButtonClicked

void MessageForm::showMessage(const QString &theMessage)
{
    gui.messageLabel->setText(theMessage);
    adjustSize();
    show();
    raise();
    activateWindow();
}//showMessage
