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

#include "TipForm.hpp"
#include "ConfigWriter.hpp"
#include "NGSettings.hpp"

using namespace std;

TipForm::TipForm(QWidget* parent,
                 Qt::WindowFlags flags) : QDialog(parent, flags)
{
    bool showTips = ConfigWriter::loadBool("tips.txt", "Show Tips");

    gui.setupUi(this);

    connect(gui.showTipsBox, SIGNAL(stateChanged(int)),
            this, SLOT(tipsBoxChanged(int)));
    connect(gui.okButton, SIGNAL(clicked(bool)),
            this, SLOT(okButtonPressed()));
    connect(gui.nextTipButton, SIGNAL(clicked(bool)),
            this, SLOT(displayTip()));

    if (showTips)
    {
        gui.showTipsBox->setCheckState(Qt::Checked);
        displayTip();
    }//if showTips
}//constructor

TipForm::~TipForm(void)
{
}//destructor

void TipForm::displayTip(void)
{
    QString tipName;//the name of the tip to load
    QString tipText;//the tip to display
    int tipIndex = ConfigWriter::loadInt("tips.txt", "Current Tip");
    int numTips = ConfigWriter::loadInt("tips.txt", "Num Tips");

    tipName = "Tip " + QString::number(tipIndex);
    tipText = QString::fromStdString(ConfigWriter::loadString("tips.txt", tipName.toStdString()));
    gui.tipText->setText(tipText);

    show();

    tipIndex++;
    if (tipIndex > numTips)
        tipIndex = 1;
    ConfigWriter::writeInt("tips.txt", "Current Tip", tipIndex);
}//displayTip

void TipForm::tipsBoxChanged(int state)
{
    if (state == Qt::Checked)
        ConfigWriter::writeBool("tips.txt", "Show Tips", true);
    else if (state == Qt::Unchecked)
        ConfigWriter::writeBool("tips.txt", "Show Tips", false);
}//tipsBoxChanged

void TipForm::okButtonPressed(void)
{
    hide();
}//okButtonPressed
