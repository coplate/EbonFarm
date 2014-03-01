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

#include "MainWindow.hpp"
#ifdef NG_OPEN_GL
    #include <QtOpenGL>
    #include <QGLWidget>
#endif
#include <QCloseEvent>
#include <QShortcutEvent>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QApplication>
#include "WhiteBoard.hpp"
#include "NGSettings.hpp"
#include "NethackFX.hpp"
#include "TelnetProtocol.hpp"
#include "ArrowHandler.hpp"
#include "NetSprite.hpp"
#include "ConnectForm.hpp"
#include "ZoomForm.hpp"
#include "MessageForm.hpp"
#include "NetCursor.hpp"
#include "HistoryLog.hpp"
#include "TipForm.hpp"
#include <farmdockwidget.h>
#include "LatencyWidget.hpp"
#include "GraphicsSettings.hpp"

using namespace std;

MainWindow::MainWindow(QWidget* parent,
                       Qt::WindowFlags flags,
                       WhiteBoard *newWhiteBoard) : QMainWindow(parent, flags)
{
    QWidget *cWidget = NULL;//pointer to the central widget
    QLayout *centralLayout = NULL;//pointer to the central widget's layout

    //### Initialize member variables ###
    whiteBoard = newWhiteBoard;
    gameSettings = new NGSettings;
    arrowHandler = new ArrowHandler(whiteBoard);
    menuMapper = new QSignalMapper(this);
    connectForm = new ConnectForm(this, NULL, whiteBoard);
    zoomForm = new ZoomForm(this, NULL, this);
    tipForm = new TipForm(this, NULL);
    netCursor = new NetCursor(NULL, whiteBoard);
    latencyWidget = new LatencyWidget(this, NULL, whiteBoard);
    graphicsSettings = new GraphicsSettings(this, NULL, whiteBoard);
    farmingWidget = new FarmDockWidget(this);
    farmingWidget->initialize(whiteBoard);

    isConnected = false;

    //### Configure GUI ###
    gui.setupUi(this);
    addMenuBindings();
    connect(menuMapper, SIGNAL(mapped(const QString &)),
            this, SLOT(menuCommand(const QString &)));

    //### Extract the central widget's layout ###
    cWidget = centralWidget();
    if (cWidget == NULL)
        whiteBoard->fatalError("MainWindow::MainWindow(): centralWidget() is NULL");
    centralLayout = cWidget->layout();

    //### Add the latency widget to the top of the layout ###
    centralLayout->addWidget(latencyWidget);
    centralLayout->removeWidget(gui.graphicsView);
    centralLayout->addWidget(gui.graphicsView);
    //gui.graphicsView->setFocus();
    this->setFocusPolicy(Qt::StrongFocus);

    addDockWidget(Qt::BottomDockWidgetArea, farmingWidget);

    connect(this, SIGNAL(tilesFinished()), farmingWidget, SLOT(alert_tiles_finished()));
    //resize(sizeHint());
}//constructor

MainWindow::~MainWindow()
{
    logicTimer.stop();

    delete tipForm;
    tipForm = NULL;

    delete arrowHandler;
    arrowHandler = NULL;

    delete gameSettings;
    gameSettings = NULL;

    delete menuMapper;
    menuMapper = NULL;

    delete connectForm;
    connectForm = NULL;

    delete zoomForm;
    zoomForm = NULL;

    //deleted automatically
    netCursor = NULL;
    latencyWidget = NULL;
    graphicsSettings = NULL;
}//destructor

bool MainWindow::start(void)
{
    NethackFX *netFX = whiteBoard->getNetFX();
    TelnetProtocol *telnetPro = whiteBoard->getTelnetPro();
    string tileset = ConfigWriter::loadString("game_config.txt", "Tileset");
    double zoomAmount = 0;//Amount to scale the graphics view
    unsigned int spriteWidth = 0;//the dimensions of each sprite in pixels
    unsigned int spriteHeight = 0;
    int startingWidth = 0;//initial dimensions of the main window
    int startingHeight = 0;
    bool useTileset = ConfigWriter::loadBool("game_config.txt", "Use Tileset");
    bool result = true;//false on errors

    //### Set the zoom value ###
    zoomAmount = static_cast<double>(zoomForm->getZoomAmount()) / static_cast<double>(100);
    gui.graphicsView->resetMatrix();
    gui.graphicsView->scale(zoomAmount, zoomAmount);

    //### Initialize the telnet protocol and display handlers ###
    if (telnetPro->initialize(netCursor))
        configureScene();
    else
        result = false;

    //### Load a custom tileset ###
    if (result)
    {
        if (useTileset)
            netFX->loadCustomTiles(QString::fromStdString(tileset));
    }//if result

    //### Resize the window ###
    if (result)
    {
        spriteWidth = NGSettings::getSpriteWidth();
        spriteHeight = NGSettings::getSpriteHeight();

        startingWidth = spriteWidth * TelnetProtocol::WINDOW_WIDTH;
        startingWidth = static_cast<int>(startingWidth * zoomAmount);
        startingWidth += BORDER_WIDTH;
        // startingWidth += farmingWidget->width();


        startingHeight = spriteHeight * (TelnetProtocol::WINDOW_HEIGHT + NGSettings::getHistoryLines());
        startingHeight = static_cast<int>(startingHeight * zoomAmount);
        startingHeight += BORDER_HEIGHT;
        startingHeight += latencyWidget->height();

        startingHeight += farmingWidget->height();

        resize(startingWidth, startingHeight);
    }//if result

    //### Start the logic timer ###
    if (result)
    {
        logicTimer.setInterval(LOGIC_UPDATE_TIME);
        logicTimer.start();

        connect(&logicTimer, SIGNAL(timeout()),
                this, SLOT(updateLogic()));
    }//if result

    //### Start the cursor blink timer ###
    if (result)
    {
        cursorTimer.setInterval(CURSOR_TIME);
        cursorTimer.start();
        connect(&cursorTimer, SIGNAL(timeout()),
                this, SLOT(blinkCursor()));
    }//if result

    //### Initialize the latency widget ###
    if (result)
    {
        if (!latencyWidget->initialize())
            result = false;
    }//if result
    //### Display the window ###
    if (result)
        show();

    return result;

    /* MDC Pudding Bot = Create a Bot Class - Send EscHandler and Telnet protocol to it
     * connect( eschandler, SIGNAL(Tiles-Finished), Bot, BotStateMachine )
     */
}//start

void MainWindow::configureScene(void)
{
    #ifdef NG_OPEN_GL
        QGLWidget* openGLWidget = NULL;//renders the scene with OpenGL
    #endif
    TelnetProtocol *telnetPro = whiteBoard->getTelnetPro();
    TelnetWindow *telnetWindow = telnetPro->getTelnetWindow();
    NethackFX *netFX = whiteBoard->getNetFX();
    bool haveOpenGL = false;//true if OpenGL is available

    //### Enable OpenGL ###
    #ifdef NG_OPEN_GL
        if (QGLFormat::hasOpenGL())
            haveOpenGL = true;

        graphicsSettings->setOpenGL(haveOpenGL);

        if (graphicsSettings->openGLEnabled())
        {
            openGLWidget = new QGLWidget();

            if (openGLWidget->isValid())
                gui.graphicsView->setViewport(openGLWidget);
            else
            {
                whiteBoard->showMessage("Couldn't create QGLWidget, using software rendering.");

                delete openGLWidget;
                openGLWidget = NULL;
            }//else isValid()
        }//if useOpenGL()
    #else
        graphicsSettings->setOpenGL(false);
    #endif

    //### Set update mode ###
    gui.graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    setSceneSize();

    //### Set the scene background to gray ###
    theScene.setBackgroundBrush(Qt::darkGray);

    //### Add the telnet cursor ###
    netCursor->setZValue(3);
    netCursor->setCursorPos(0, 0);
    theScene.addItem(netCursor);

    //### Prevent keystrokes from being consumed by the graphics view ###
    gui.graphicsView->setFocusPolicy(Qt::NoFocus);
    gui.graphicsView->installEventFilter(this);

    //### Display the scene ###
    NGSettings::setSpriteSize(ImageLoader::DEFAULT_SPRITE_SIZE, ImageLoader::DEFAULT_SPRITE_SIZE);
    telnetWindow->resetWindow(&theScene);
    netFX->setGraphicsScene(&theScene);
    gui.graphicsView->setScene(&theScene);
    gui.graphicsView->show();
}//configureScene

void MainWindow::setSceneSize(void)
{
    QRectF sceneSize;//the dimensions of the scene in pixels
    unsigned int windowWidth = TelnetProtocol::WINDOW_WIDTH;
    unsigned int windowHeight = TelnetProtocol::WINDOW_HEIGHT;
    unsigned int spriteWidth = NGSettings::getSpriteWidth();
    unsigned int spriteHeight = NGSettings::getSpriteHeight();
    int logHeight = 0;//height of the history log in pixels
    int sceneWidth = 0;//scene width in pixels
    int sceneHeight = 0;//scene height in pixels
    int sceneX = 0;//coordinates of the top right corner of the scene rectangle
    int sceneY = 0;

    //### Calculate the scene size ###
    sceneWidth = windowWidth * spriteWidth;
    sceneHeight = windowHeight * spriteHeight;

    //### Make room for the history log ###
    logHeight = NGSettings::getHistoryLines() * spriteHeight;
    sceneHeight += logHeight;

    //### Set the scene size and position ###
    sceneSize.setRect(sceneX, sceneY, sceneWidth, sceneHeight);
    theScene.setSceneRect(sceneSize);
}//setSceneSize

void MainWindow::alertTilesFinished(void)
{
    emit tilesFinished();
}

void MainWindow::resetGraphics(void)
{
    TelnetProtocol *telnetPro = whiteBoard->getTelnetPro();
    TelnetWindow *telnetWindow = telnetPro->getTelnetWindow();
    NethackFX *netFX = whiteBoard->getNetFX();
    bool graphicsEnabled = gui.actionGraphics_Enabled->isChecked();

    gameSettings->reloadHistoryLines();

    gameSettings->updateFontFromConfig();
    telnetWindow->resetWindow(&theScene);
    netFX->resetHistoryLog(&theScene);

    setSceneSize();

    netFX->setUserFX(!graphicsEnabled);
    netFX->setUserFX(graphicsEnabled);
}//resetGraphics

void MainWindow::updateLogic(void)
{
    NethackFX *netFX = whiteBoard->getNetFX();

    netFX->updateQuery();
}//updateLogic

void MainWindow::pauseCursor(void)
{
    if (!netCursor->isVisible())
        netCursor->show();

    netCursor->setColor(Qt::red);
    cursorTimer.stop();
}//pauseCursor

void MainWindow::restoreCursor(void)
{
    netCursor->setColor(Qt::white);
    cursorTimer.start();
}//restoreCursor

void MainWindow::blinkCursor(void)
{
    if (netCursor->isVisible())
        netCursor->hide();
    else
        netCursor->show();
}//blinkCursor

void MainWindow::menuCommand(const QString &param)
{
    TelnetProtocol *telnetPro = whiteBoard->getTelnetPro();
    string keystrokes = param.toStdString();

    //### Send buffered arrows before the new command ###
    arrowHandler->sendCommand(Qt::Key_unknown);

    for (unsigned int i = 0; i < keystrokes.size(); i++)
        telnetPro->sendKeystroke(keystrokes.at(i));
}//menuCommand

void MainWindow::keyReleaseEvent(QKeyEvent *keyEvent)
{
    //### Send buffered arrows when a key is released ###
    arrowHandler->sendCommand(Qt::Key_unknown);

    //### Pass the event to the parent class ###
    keyEvent->ignore();
    QMainWindow::keyPressEvent(keyEvent);
}//keyReleaseEvent

void MainWindow::keyPressEvent(QKeyEvent *keyEvent)
{
    string keyText;//unicode text from the keypress
    QFlags <Qt::KeyboardModifier> modifiers(keyEvent->modifiers());//status of ALT, CTRL and SHIFT keys
    TelnetProtocol *telnetPro = whiteBoard->getTelnetPro();
    bool handledEvent = false;//true if we used this keyEvent, otherwise pass it along

    //### Translate arrow keys to nethack commands ###
    if (arrowHandler->sendCommand(keyEvent->key()))
        handledEvent = true;

    //### Send ASCII commands ###
    if (!handledEvent)
    {
        keyText = keyEvent->text().toStdString();
        if (keyText.size() > 0)
        {
            for (unsigned int i = 0; i < keyText.size(); i++)
                telnetPro->sendKeystroke(keyText.at(i));
            handledEvent = true;
        }//if size()
    }//if !handledEvent

    //### Pass the event to the parent class if we didn't handle it ###
    if (handledEvent)
        keyEvent->accept();
    else
    {
        keyEvent->ignore();
        QMainWindow::keyPressEvent(keyEvent);
    }//else handledEvent
}//keyPressEvent

bool MainWindow::eventFilter(QObject *obj,
                             QEvent *event)
{
    QKeyEvent *keyEvent = NULL;//event data, if this is a keystroke event
    bool result = false;//return true to consume the keystroke

    if (event->type() == QEvent::KeyPress)
    {
         keyEvent = dynamic_cast<QKeyEvent*>(event);
         this->keyPressEvent(keyEvent);
         result = true;
    }//if type()

    else if (event->type() == QEvent::KeyRelease)
    {
        keyEvent = dynamic_cast<QKeyEvent*>(event);
        this->keyReleaseEvent(keyEvent);
        result = true;
    }//else if type()

    //### Standard event processing ###
    else
        result = QObject::eventFilter(obj, event);

    return result;
}//eventFilter

void MainWindow::reportConnection(void)
{
    isConnected = true;
}//reportConnection

void MainWindow::closeEvent(QCloseEvent *event)
{
    MessageForm *messageForm = whiteBoard->getMessageForm();

    messageForm->close();
    event->accept();
}//close

void MainWindow::dumpScreen(void)
{
    TelnetProtocol *telnetPro = whiteBoard->getTelnetPro();
    uint8_t oneByte;//a single character from the telnet window

    for (unsigned int yPos = 0; yPos < telnetPro->getHeight(); yPos++)
    {
        for (unsigned int xPos = 0; xPos < telnetPro->getWidth(); xPos++)
        {
            oneByte = telnetPro->getByte(xPos, yPos);

            if ((oneByte >= 32) && (oneByte <= 126))
                cout << oneByte;
            else
                cout << " ";
        }//for xPos
        cout << endl;
    }//for yPos
}//dumpScreen

void MainWindow::showConnectForm(void)
{
    connectForm->show();
}//showConnectForm

void MainWindow::showZoomForm(void)
{
    zoomForm->show();
}//showZoomForm

void MainWindow::zoomFormNewValue(void)
{
    double newZoom = 1;//the zoom amount to apply

    newZoom = static_cast<double>(zoomForm->getZoomAmount()) / static_cast<double>(100);
    gui.graphicsView->resetMatrix();
    gui.graphicsView->scale(newZoom, newZoom);

    ConfigWriter::writeInt("game_config.txt", "Zoom Factor", zoomForm->getZoomAmount());
    zoomForm->hide();
}//zoomFormNewValue

void MainWindow::toggleGraphics(bool enableGraphics)
{
    NethackFX *netFX = whiteBoard->getNetFX();

    netFX->setUserFX(enableGraphics);
}//toggleGraphics

void MainWindow::setGraphicsMode(bool newValue)
{
    gui.actionGraphics_Enabled->setChecked(newValue);
}//setGraphicsMode

void MainWindow::showManual(void)
{
    QString manualUrl;//the absolute path to the EbonHack manual
    QString errMsg;//show an error message if we couldn't open the manual

    manualUrl = QDir::currentPath();
    manualUrl += QString("/manual.html");
    manualUrl = QDir::toNativeSeparators(manualUrl);
    manualUrl = QString("file:///") + manualUrl;

    if (!QDesktopServices::openUrl(QUrl(manualUrl, QUrl::TolerantMode)))
        whiteBoard->showMessage(manualUrl);
}//showManual

void MainWindow::showTipOfTheDay(void)
{
    tipForm->displayTip();
}//showTipOfTheDay

void MainWindow::showGraphicsSettings(void)
{
    graphicsSettings->show();
}//showGraphicsSettings

void MainWindow::quitClicked(void)
{
    whiteBoard->getQTApp()->closeAllWindows();
}//quitClicked

void MainWindow::addMenuBindings(void)
{
    //### EbonHack Menu ###
    connect(gui.actionConnect, SIGNAL(triggered(bool)),
            this, SLOT(showConnectForm()));
    connect(gui.actionEbonHack_Manual, SIGNAL(triggered(bool)),
            this, SLOT(showManual()));
    connect(gui.actionGraphics_Enabled, SIGNAL(toggled(bool)),
            this, SLOT(toggleGraphics(bool)));
    connect(gui.actionGraphics_Settings, SIGNAL(triggered(bool)),
            this, SLOT(showGraphicsSettings()));
    connect(gui.actionTip_of_the_Day, SIGNAL(triggered(bool)),
            this, SLOT(showTipOfTheDay()));
    connect(gui.actionZoom, SIGNAL(triggered(bool)),
            this, SLOT(showZoomForm()));
    connect(gui.actionQuit, SIGNAL(triggered(bool)),
            this, SLOT(quitClicked()));

    //### Game Menu ###
    addBinding(gui.actionCompile_Options, "#version\r");
    addBinding(gui.actionExplore_Mode, "X");
    addBinding(gui.actionHistory, "V");
    addBinding(gui.actionOptions, "O");
    addBinding(gui.actionRedraw_Screen, getCTRL('R'));
    addBinding(gui.actionSuicide, "#quit\r");
    addBinding(gui.actionVersion, "v");
    addBinding(gui.actionSave_And_Exit, "S");

    //### Gear Menu ###
    addBinding(gui.actionApparel_Off, "A");
    addBinding(gui.actionExchange_Weapons, "x");
    addBinding(gui.actionLoad_Quiver, "Q");
    addBinding(gui.actionPut_On_Accessory, "P");
    addBinding(gui.actionRemove_Accessory, "R");
    addBinding(gui.actionTake_Off_Armor, "T");
    addBinding(gui.actionTwo_Weapon_Combat, "#twoweapon\r");
    addBinding(gui.actionWear_Armor, "W");
    addBinding(gui.actionWield_Weapon, "w");

    //### Action Menu ###
    addBinding(gui.actionAgain, getCTRL('A'));
    addBinding(gui.actionApply, "a");
    addBinding(gui.actionChat, "#chat\r");
    addBinding(gui.actionClose_Door, "c");
    addBinding(gui.actionEat, "e");
    addBinding(gui.actionFight, "F");
    addBinding(gui.actionFire, "f");
    addBinding(gui.actionForce, "#force\r");
    addBinding(gui.actionKick, getCTRL('D'));
    addBinding(gui.actionLoot, "#loot\r");
    addBinding(gui.actionOpen_Door, "o");
    addBinding(gui.actionPay, "p");
    addBinding(gui.actionRest, ".");
    addBinding(gui.actionRide, "#ride\r");
    addBinding(gui.actionSearch, "s");
    addBinding(gui.actionSit, "#sit\r");
    addBinding(gui.actionThrow, "t");
    addBinding(gui.actionUntrap, "#untrap\r");
    addBinding(gui.actionWipe_Face, "#wipe\r");

    //### Move Menu ###
    addBinding(gui.actionUp, "<");
    addBinding(gui.actionDown, ">");
    addBinding(gui.actionJump, "#jump\r");
    addBinding(gui.actionGo, "g");
    addBinding(gui.actionGo_Ignore_Forks, "G");
    addBinding(gui.actionMove, "m");
    addBinding(gui.actionMove_Far, "M");

    //### Magic Menu ###
    addBinding(gui.actionDip, "#dip\r");
    addBinding(gui.actionEngrave, "E");
    addBinding(gui.actionInvoke, "#invoke\r");
    addBinding(gui.actionList_Reorder_Spells, "+");
    addBinding(gui.actionMonster_Ability, "#monster\r");
    addBinding(gui.actionOffer, "#offer\r");
    addBinding(gui.actionPray, "#pray\r");
    addBinding(gui.actionQuaff_Potion, "q");
    addBinding(gui.actionRead_Scroll_Book, "r");
    addBinding(gui.actionRub, "#rub\r");
    addBinding(gui.actionTeleport, getCTRL('T'));
    addBinding(gui.actionTurn_Undead, "#turn\r");
    addBinding(gui.actionZap_Spell, "Z");
    addBinding(gui.actionZap_Wand, "z");

    //### Inventory ###
    addBinding(gui.actionCount_Gold, "$");
    addBinding(gui.actionDrop, "d");
    addBinding(gui.actionDrop_Many, "D");
    addBinding(gui.actionEquipped_Amulet, "\"");
    addBinding(gui.actionEquipped_Armor, "[");
    addBinding(gui.actionEquipped_Rings, "=");
    addBinding(gui.actionEquipped_Tools, "(");
    addBinding(gui.actionEquipped_Weapons, ")");
    addBinding(gui.actionGet, ",");
    addBinding(gui.actionInventory, "i");
    addBinding(gui.actionInventory_of_Type, "I");
    addBinding(gui.actionList_All_Equipment, "*");
    addBinding(gui.actionToggle_Autopickup, "@");

    //### Info ###
    addBinding(gui.actionAdjust_Letters, "#adjust\r");
    addBinding(gui.actionBase_Attributes, getCTRL('X'));
    addBinding(gui.actionConduct, "#conduct\r");
    addBinding(gui.actionDiscoveries, "\\");
    addBinding(gui.actionEnhance_Skills, "#enhance\r");
    addBinding(gui.actionName_Creature, "C");
    addBinding(gui.actionName_Object, "#name\r");
    addBinding(gui.actionRepeat_Message, getCTRL('P'));

    //### Help ###
    addBinding(gui.actionCommand_Info, "&");
    addBinding(gui.actionExtended_Commands, "#?\r");
    addBinding(gui.actionGet_Trap_Type, "^");
    addBinding(gui.actionHelp, "?");
    addBinding(gui.actionWhat_is, "/");
    addBinding(gui.actionWhat_is_Here, ":");
    addBinding(gui.actionWhat_is_There, ";");
}//addMenuBindings

void MainWindow::addBinding(QAction* menuItem,
                            const string &keyStrokes)
{
    connect(menuItem, SIGNAL(triggered()), menuMapper, SLOT(map()));
    menuMapper->setMapping(menuItem, QString::fromStdString(keyStrokes));
}//addBinding

LatencyWidget* MainWindow::getLatencyWidget(void)
{
    return latencyWidget;
}//getLatencyWidget

FarmDockWidget* MainWindow::getFarmingWidget(void)
{
    return farmingWidget;

}//getfarmingwidget

GraphicsSettings* MainWindow::getGraphicsSettings(void)
{
    return graphicsSettings;
}//getGraphicsSettings

string MainWindow::getCTRL(char letter)
{
    uint8_t intVal = static_cast<uint8_t>(letter);
    string result;//the ctrl character to return

    if ((intVal >= 65) && (intVal <= 90))
        result.push_back(intVal - 64);
    else
    {
        cout << "MainWindow::getCTRL(): only capital letters accepted, bailing!" << endl;
        throw 1;
    }//else intVal && intVal

    return result;
}//getCTRL

