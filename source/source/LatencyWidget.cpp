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

#include "LatencyWidget.hpp"
#include "WhiteBoard.hpp"
#include "MainWindow.hpp"
#include <QFontMetrics>
#include <QPainter>

using namespace std;

LatencyWidget::LatencyWidget(QWidget* parent,
                             Qt::WindowFlags flags,
                             WhiteBoard *newWhiteBoard) : QLabel(parent, flags)
{
    whiteBoard = newWhiteBoard;

    waitingReply = false;
    averageLatency = 0;
    textWidth = 0;

    setText("Latency: 0 ms");
}//constructor

LatencyWidget::~LatencyWidget(void)
{
}//destructor

bool LatencyWidget::initialize(void)
{
    QFontMetrics fontMetrics(font());
    bool result = true;//false if we couldn't load graphics

    //### Connect the update timer ###
    updateTimer.setInterval(LATENCY_THRESHOLD);
    updateTimer.setSingleShot(true);
    connect(&updateTimer, SIGNAL(timeout()),
            this, SLOT(doUpdate()));

    //### Load the stop and go icons ###
    if (!loadImages())
        result = false;

    //### Calculate the width of the latency text ###
    if (result)
        textWidth = fontMetrics.width("Latency: 8888 ms  ", -1);

    return result;
}//initialize

bool LatencyWidget::loadImages(void)
{
    bool result = true;//false if we couldn't load an image

    //### Load the go image ###
    if (!goImage.load("data/go.png", "PNG", Qt::AutoColor))
    {
        whiteBoard->showMessage("LatencyWidget::loadImages(): couldn't load go.png");
        result = false;
    }//if !load()

    //### Load the stop image ###
    if (result)
    {
        if (!stopImage.load("data/stop.png", "PNG", Qt::AutoColor))
        {
            whiteBoard->showMessage("LatencyWidget::loadImages(): couldn't load stop.png");
            result = false;
        }//if !load()
    }//if result

    //### Scale the images ###
    if (result)
    {
        goImage = goImage.scaled(goImage.width(), height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        stopImage = stopImage.scaled(goImage.width(), height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }//if result

    return result;
}//loadImages

void LatencyWidget::paintEvent(QPaintEvent *event)
{
    QPainter customPainter;//draws the stop or go icon

    //### Resize images if necessary ###
    if (goImage.height() != height())
        loadImages();

    //### Draw the label text ###
    QLabel::paintEvent(event);

    //### Draw the icon ###
    customPainter.begin(this);
    if (waitingReply)
        customPainter.drawPixmap(textWidth, 0, stopImage);
    else
        customPainter.drawPixmap(textWidth, 0, goImage);
}//paintEvent

void LatencyWidget::reportCommand(void)
{
    MainWindow *mainWindow = whiteBoard->getMainWindow();

    if (!waitingReply)
    {
        waitingReply = true;
        commandTime.start();

        if (averageLatency > LATENCY_THRESHOLD)
        {
            update();
            mainWindow->pauseCursor();
        }//if averageLatency
        else
            updateTimer.start();
    }//if !waitingReply
}//reportCommand

void LatencyWidget::reportDisconnect(void)
{
    updateTimer.stop();
    waitingReply = false;
    doUpdate();
}//reportAllSent

void LatencyWidget::doUpdate(void)
{
    MainWindow *mainWindow = whiteBoard->getMainWindow();

    update();

    if (waitingReply)
        mainWindow->pauseCursor();
    else
        mainWindow->restoreCursor();
}//doUpdate

void LatencyWidget::reportReply(void)
{
    MainWindow *mainWindow = whiteBoard->getMainWindow();

    if (waitingReply)
    {
        updateTimer.stop();

        averageLatency *= static_cast<float>(RUNNING_AVG_SIZE - 1) / static_cast<float>(RUNNING_AVG_SIZE);
        if (commandTime.elapsed() != 0)
            averageLatency += static_cast<float>(commandTime.elapsed()) / static_cast<float>(RUNNING_AVG_SIZE);

        if (averageLatency > 9999)
            averageLatency = 9999;

        setText("Latency: " + QString::number(commandTime.elapsed(), 10) + " ms");
        waitingReply = false;
        //mainWindow->alertTilesFinished();
        update();

        mainWindow->restoreCursor();
    }//if waitingReply
}//reportReply
