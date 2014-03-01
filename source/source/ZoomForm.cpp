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

#include "ZoomForm.hpp"
#include "WhiteBoard.hpp"
#include "MainWindow.hpp"
#include "ConfigWriter.hpp"
#include "NGSettings.hpp"

using namespace std;

ZoomForm::ZoomForm(QWidget* parent,
                   Qt::WindowFlags flags,
                   MainWindow *mainWindow) : QDialog(parent, flags)
{
    int storedZoom = ConfigWriter::loadInt("game_config.txt", "Zoom Factor");

    gui.setupUi(this);
    connect(gui.okButton, SIGNAL(clicked()),
            mainWindow, SLOT(zoomFormNewValue()));
    connect(gui.cancelButton, SIGNAL(clicked()),
            this, SLOT(cancelButtonClicked()));

    gui.zoomBox->setValue(storedZoom);
}//constructor

ZoomForm::~ZoomForm(void)
{
}//destructor

void ZoomForm::cancelButtonClicked(void)
{
    hide();
}//cancelButtonClicked

int ZoomForm::getZoomAmount(void)
{
    return gui.zoomBox->value();
}//getZoomAmount
