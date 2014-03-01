#ifndef GRAPHICSSETTINGS_HPP_INCLUDED
#define GRAPHICSSETTINGS_HPP_INCLUDED

#include <QDialog>
#include <QFileDialog>
#include "ui_GraphicsSettings.h"

class WhiteBoard;

class GraphicsSettings : public QDialog
{
    Q_OBJECT

    public:
        //constructor
        GraphicsSettings(QWidget* parent,
                         Qt::WindowFlags flags,
                         WhiteBoard *newWhiteBoard);

        //destructor
        ~GraphicsSettings(void);

        //Report that OpenGL is or isn't available on this system
        void setOpenGL(bool available);

        //Returns true if we should use OpenGL for graphics rendering, false otherwise
        bool openGLEnabled(void);

        //Returns true if we should use tile data sent by the server, false if we should ignore it
        bool serverTilesEnabled(void);

    private slots:
        //Button event handlers
        void applyClicked();
        void okClicked();
        void cancelClicked();
        void customToggled(bool);
        void defaultToggled(bool);
        void nativeQtToggled(bool);
        void openGLToggled(bool);
        void selectTilesetClicked();
        void customFontToggled(bool);

        //Event handler when a new tileset is chosen
        void tilesetChosen(const QString&);

        //Event handler called when some aspect of the configuration changes. It's used in cases where
        //a widget produced a change that we aren't interested in responding to, but want to track that
        //the change occurred.
        void settingChanged();

    private:
        //Pointer to the global white board, don't delete
        WhiteBoard *whiteBoard;

        //Allows the user to select a custom tileset
        QFileDialog *tileChooser;

        //The widgets belonging to the graphics settings form
        Ui::GraphicsSettingsDialog gui;

        //True if EbonHack is able to use OpenGL, false for Qt native graphics
        //only.
        bool openGLAvailable;

        //True if the user selected a new tileset that needs to be loaded
        bool tilesetChanged;

        //True if the user switch graphics modes
        bool graphicsModeChanged;

        //Loads graphics settings from file
        void load(void);

        //Apply any outstanding changes to graphics settings. Returns false on error and
        //displays a message to the user
        bool applyChanges(void);

};//GraphicsSettings

#endif
