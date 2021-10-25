/*
    SPDX-FileCopyrightText: 2008 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
    SPDX-FileCopyrightText: 2007-2008 Pierre-Benoit Bessse <besse@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <KXmlGuiWindow>

class Game;
class GameView;
class GameScene;
class PlayerSettings;
class KgThemeProvider;
class KConfigDialog;

/**
 * @brief This class enables to create the main window for Granatier.
 */
class MainWindow : public KXmlGuiWindow {

Q_OBJECT

private :

    /** The GameView instance that manages the game drawing and the collisions */
    GameView* m_view;

    GameScene* m_scene;

    /** The Game instance that manages the main loop and events */
    Game* m_game;

    /** The PlayerSettings instance for player name, shortcuts ...  */
    PlayerSettings* m_playerSettings;

    /** The KgThemeProvider instance */
    KgThemeProvider* m_themeProvider;
    QString m_currentThemeIdentifier;

    /**  */
    QStringList m_tempRandomArenaModeArenaList;

    KConfigDialog* m_settingsDialog;

public:

    /**
      * Creates a new MainWindow instance.
      */
    MainWindow();

    /**
      * Deletes the MainWindow instance.
      */
    ~MainWindow() override;

private Q_SLOTS:

    /**
      * Initializes the MainWindow for a new game.
      * Creates a new Game instance and a new GameView instance that displays the game.
      */
    void initGame();

    /**
      * Starts a new game.
      */
    void newGame();

    /**
      * Sets the sounds enabled / disabled.
      * @param p_enabled if true the sounds will be enabled, otherwise they will be disabled
      */
    void setSoundsEnabled(bool p_enabled);

    /**
      * Shows the settings dialog.
      */
    void showSettings();

    /**
      * Applies the new settings.
      */
    void applyNewSettings();

    /**
      * Cancel at settings dialog clicked.
      */
    void settingsDialogCanceled();

    /**
      * Closes the MainWindow.
      */
    void close();
};

#endif

