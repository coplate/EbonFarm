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

#include "GraphicsSettings.hpp"
#include "WhiteBoard.hpp"
#include "ConfigWriter.hpp"
#include "NethackFX.hpp"
#include "MainWindow.hpp"
#include <QDir>

using namespace std;

GraphicsSettings::GraphicsSettings(QWidget* parent,
                                   Qt::WindowFlags flags,
                                   WhiteBoard *newWhiteBoard) : QDialog(parent, flags)
{
    QString tilesetDir;//the directory containing built-in tilesets

    whiteBoard = newWhiteBoard;

    tileChooser = new QFileDialog;
    openGLAvailable = false;
    tilesetChanged = false;
    graphicsModeChanged = false;

    gui.setupUi(this);

    //### Connect buttons ###
    connect(gui.applyButton, SIGNAL(clicked()),
            this, SLOT(applyClicked()));
    connect(gui.okButton, SIGNAL(clicked()),
            this, SLOT(okClicked()));
    connect(gui.cancelButton, SIGNAL(clicked()),
            this, SLOT(cancelClicked()));
    connect(gui.defaultTilesetButton, SIGNAL(toggled(bool)),
            this, SLOT(defaultToggled(bool)));
    connect(gui.customTilesetButton, SIGNAL(toggled(bool)),
            this, SLOT(customToggled(bool)));
    connect(gui.selectTilesetButton, SIGNAL(clicked()),
            this, SLOT(selectTilesetClicked()));
    connect(gui.nativeQtButton, SIGNAL(toggled(bool)),
            this, SLOT(nativeQtToggled(bool)));
    connect(gui.openGLButton, SIGNAL(toggled(bool)),
            this, SLOT(openGLToggled(bool)));
    connect(gui.customFontButton, SIGNAL(toggled(bool)),
            this, SLOT(customFontToggled(bool)));

    //### Connect spin boxes ###
    connect(gui.historyLinesBox, SIGNAL(valueChanged(int)),
            this, SLOT(settingChanged()));
    connect(gui.customFontSizeBox, SIGNAL(valueChanged(int)),
            this, SLOT(settingChanged()));
    connect(gui.customFontBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(settingChanged()));

    //### Configure the tile chooser ###
    connect(tileChooser, SIGNAL(fileSelected(const QString&)),
            this, SLOT(tilesetChosen(const QString&)));
    tileChooser->setFileMode(QFileDialog::ExistingFile);

    tilesetDir = QDir::current().absolutePath() + "/data/tilesets";
    tileChooser->setDirectory(tilesetDir);

    tileChooser->setNameFilter("*.bmp *.png *.jpg *.jpeg *.gif *.tiff");

    //### Configure QSpinBox limits ###
    gui.customFontSizeBox->setMaximum(72);
    gui.customFontSizeBox->setMinimum(7);

    //### Load graphics configuration ###
    load();
}//constructor

GraphicsSettings::~GraphicsSettings(void)
{
    delete tileChooser;
    tileChooser = NULL;
}//destructor

void GraphicsSettings::load(void)
{
    QFont customFont;//a custom font chosen by the user
    string tileset = ConfigWriter::loadString("game_config.txt", "Tileset");
    string customFontName = ConfigWriter::loadString("game_config.txt", "Custom Font Name");
    int customFontSize = ConfigWriter::loadInt("game_config.txt", "Custom Font Size");
    int historyLines = ConfigWriter::loadInt("game_config.txt", "History Size");
    bool useCustomFont = ConfigWriter::loadBool("game_config.txt", "Use Custom Font");
    bool useTileset = ConfigWriter::loadBool("game_config.txt", "Use Tileset");
    bool useOpenGL = ConfigWriter::loadBool("game_config.txt", "Use OpenGL");
    bool useServerTiles = ConfigWriter::loadBool("game_config.txt", "Use Server Tiles");

    //### Disable the OpenGL button if not available ###
    if (openGLAvailable)
        gui.openGLButton->setEnabled(true);
    else
        gui.openGLButton->setEnabled(false);

    //### Check the proper graphics button ###
    if ((useOpenGL) && (openGLAvailable))
        gui.openGLButton->setChecked(true);
    else
        gui.nativeQtButton->setChecked(true);

    //### Check the custom tileset button ###
    if (useTileset)
    {
        gui.customTilesetButton->setChecked(true);
        gui.tilesetBox->setEnabled(true);
        gui.selectTilesetButton->setEnabled(true);
    }//if useTileset
    else
    {
        gui.customTilesetButton->setChecked(false);
        gui.tilesetBox->setEnabled(false);
        gui.selectTilesetButton->setEnabled(false);
    }//else useTileset

    //### Set the custom tileset text ###
    gui.tilesetBox->setText(QString::fromStdString(tileset));

    //### Check the Use Server Tiles button ###
    gui.serverTilesBox->setChecked(useServerTiles);

    //### Enable or disable the custom font controls ###
    if (useCustomFont)
    {
        gui.customFontButton->setChecked(true);
        customFontToggled(true);
    }
    else
    {
        gui.defaultFontButton->setChecked(true);
        customFontToggled(false);
    }

    //### Populate the custom font widgets ###
    customFont.setFamily(QString(customFontName.c_str()));
    gui.customFontBox->setCurrentFont(customFont);
    gui.customFontSizeBox->setValue(customFontSize);

    //### Set the number of history lines ###
    gui.historyLinesBox->setValue(historyLines);

    tilesetChanged = false;
    graphicsModeChanged = false;
    gui.applyButton->setEnabled(false);
}//load

void GraphicsSettings::settingChanged()
{
    gui.applyButton->setEnabled(true);
}

void GraphicsSettings::setOpenGL(bool available)
{
    bool useOpenGL = ConfigWriter::loadBool("game_config.txt", "Use OpenGL");

    openGLAvailable = available;

    //### Disable OpenGL in the config file if not available ###
    if ((!available) && (useOpenGL))
    {
        useOpenGL = false;
        ConfigWriter::writeBool("game_config.txt", "Use OpenGL", false);
    }//if !available && useOpenGL

    //### Reload settings ###
    load();
}//setOpenGL

bool GraphicsSettings::applyChanges(void)
{
    bool result = true;//false on errors
    MainWindow *mainWindow = whiteBoard->getMainWindow();
    NethackFX *netFX = whiteBoard->getNetFX();
    QString tileset = gui.tilesetBox->text();
    QFont currentFont = gui.customFontBox->currentFont();
    bool useTileset = gui.customTilesetButton->isChecked();
    bool useOpenGL = gui.openGLButton->isChecked();
    bool useServerTiles = gui.serverTilesBox->isChecked();

    //### Make sure we have an entry to write to file ###
    if (tileset.size() == 0)
    {
        tileset = "NONE";
        gui.tilesetBox->setText(tileset);
    }//if size()

    //### Load the new tileset ###
    if (tilesetChanged)
    {
        if (useTileset)
        {
            if (!netFX->loadCustomTiles(tileset))
                result = false;
        }//if useTileset
        else
        {
            if (!netFX->loadDefaultTiles())
                result = false;
        }//else useTileset
    }//if tilesetChanged

    //### Save settings and close the dialog ###
    if (result)
    {
        if (graphicsModeChanged)
        {
            whiteBoard->showMessage("EbonHack must be restarted for the changes to take effect.");
            graphicsModeChanged = false;
        }//if graphicsModeChanged

        tilesetChanged = false;

        ConfigWriter::writeBool("game_config.txt", "Use Tileset", useTileset);
        ConfigWriter::writeString("game_config.txt", "Tileset", tileset.toStdString());
        ConfigWriter::writeBool("game_config.txt", "Use OpenGL", useOpenGL);
        ConfigWriter::writeBool("game_config.txt", "Use Server Tiles", useServerTiles);
        ConfigWriter::writeBool("game_config.txt", "Use Custom Font", gui.customFontButton->isChecked());
        ConfigWriter::writeString("game_config.txt", "Custom Font Name", currentFont.family().toStdString());
        ConfigWriter::writeInt("game_config.txt", "Custom Font Size", gui.customFontSizeBox->value());
        ConfigWriter::writeInt("game_config.txt", "History Size", gui.historyLinesBox->value());

        mainWindow->resetGraphics();
    }//if result

    return result;
}

void GraphicsSettings::applyClicked(void)
{
    if (applyChanges())
        gui.applyButton->setEnabled(false);
}

void GraphicsSettings::okClicked(void)
{
    if (gui.applyButton->isEnabled())
    {
        if (applyChanges())
        {
            gui.applyButton->setEnabled(false);
            hide();
        }//if allOK
    }
    else
        hide();
}//okClicked

void GraphicsSettings::cancelClicked(void)
{
    load();
    hide();
}//cancelClicked

void GraphicsSettings::defaultToggled(bool checked)
{
    if (checked)
    {
        gui.tilesetBox->setEnabled(false);
        gui.selectTilesetButton->setEnabled(false);

        tilesetChanged = true;
        gui.applyButton->setEnabled(true);
    }//if checked
}//defaultToggled

void GraphicsSettings::customToggled(bool checked)
{
    if (checked)
    {
        gui.tilesetBox->setEnabled(true);
        gui.selectTilesetButton->setEnabled(true);

        tilesetChanged = true;
        gui.applyButton->setEnabled(true);
    }//if checked
}//customToggled

void GraphicsSettings::nativeQtToggled(bool)
{
    graphicsModeChanged = true;
    gui.applyButton->setEnabled(true);
}//nativeQtToggled

void GraphicsSettings::openGLToggled(bool)
{
    graphicsModeChanged = true;
    gui.applyButton->setEnabled(true);
}//openGLToggled

void GraphicsSettings::customFontToggled(bool checked)
{
    if (checked)
    {
        gui.customFontBox->setEnabled(true);
        gui.customFontSizeBox->setEnabled(true);
    }
    else
    {
        gui.customFontBox->setEnabled(false);
        gui.customFontSizeBox->setEnabled(false);
    }

    gui.applyButton->setEnabled(true);
}//customFontToggled

void GraphicsSettings::selectTilesetClicked(void)
{
    tileChooser->show();
}//selectTilesetClicked

void GraphicsSettings::tilesetChosen(const QString &filename)
{
    tilesetChanged = true;
    gui.tilesetBox->setText(filename);
    gui.applyButton->setEnabled(true);
}//tilesetChosen

bool GraphicsSettings::openGLEnabled(void)
{
    return gui.openGLButton->isChecked();
}//openGLEnabled

bool GraphicsSettings::serverTilesEnabled(void)
{
    return gui.serverTilesBox->isChecked();
}//serverTilesEnabled
