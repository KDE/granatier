/*
 * Copyright 2008 Mathias Kraus <k.hias@gmx.de>
 * Copyright 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
 * Copyright 2007-2008 Pierre-Benoit Bessse <besse@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of 
 * the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mainwindow.h"
#include "game.h"
#include "gameview.h"
#include "gamescene.h"
#include "settings.h"
#include "arenaselector.h"
#include "playerselector.h"
#include "playersettings.h"
#include "ui_generalsettings.h"

#include <QGraphicsView>
#include <QTimer>
#include <KActionCollection>
#include <KStandardGameAction>
#include <KToggleAction>
#include <KMessageBox>
#include <KConfigDialog>
#include <KGameThemeSelector>
#include <KLocale>

#include <QGraphicsSvgItem>

class GeneralSettings : public QWidget
{
public:
    GeneralSettings(QWidget *parent) : QWidget(parent)
    {
        ui.setupUi(this);
        ui.groupBox->setVisible(false);
    }
private:
    Ui::GeneralSettings ui;
};

MainWindow::MainWindow()
{
    // Initialize the game
    m_game = NULL;
    m_view = NULL;
    m_playerSettings = new PlayerSettings();
    // Set the window menus
    KStandardGameAction::gameNew(this, SLOT(newGame(bool)), actionCollection());
    //KStandardGameAction::highscores(this, SLOT(showHighscores()), actionCollection());
    KStandardAction::preferences(this, SLOT(showSettings()), actionCollection());
    KStandardGameAction::quit(this, SLOT(close()), actionCollection());
    KAction* soundAction = new KToggleAction(i18n("&Play sounds"), this);
    soundAction->setChecked(Settings::sounds());
    actionCollection()->addAction( QLatin1String( "sounds" ), soundAction);
    connect(soundAction, SIGNAL(triggered(bool)), this, SLOT(setSoundsEnabled(bool)));
    // init game
    initGame();
    // Setup the window
    setupGUI(Keys | Save | Create);
}

MainWindow::~MainWindow()
{
    delete m_game;
    delete m_view;
    delete m_playerSettings;
}

void MainWindow::initGame()
{
    //the focus has to be set at the beginning and also at the end to cover all possible cases
    //TODO: check why setting the focus only at the end doesn't work
    this->setFocusProxy(m_view);
    this->setFocus();
    // If a Game instance already exists
    if (m_game)
    {
        // Delete the Game instance
        delete m_game;
    }
    // Create a new Game instance
    m_game = new Game(m_playerSettings);
    connect(m_game, SIGNAL(gameOver(bool)), this, SLOT(newGame(bool)));     // TODO Remove the useless bool parameter from gameOver()
    // If a GameView instance already exists
    if (m_view)
    {
        // Delete the GameView instance
        delete m_view;
    }
    // Create a new GameView instance
    m_view = new GameView(m_game);
    m_view->setBackgroundBrush(Qt::black);
    setCentralWidget(m_view);
    m_game->setGameScene(dynamic_cast <GameScene*> (m_view->scene()));
    
    this->setFocusProxy(m_view);
    this->setFocus();
}

void MainWindow::newGame(const bool gameOver)
{
    bool gameRunning;       // True if the game is running (game timer is active), false otherwise
  
    gameRunning = m_game->getTimer()->isActive();
    // If the game is running
    if (gameRunning)
    {
        // Pause the game
        m_game->pause();
    }
    // If the game was not over
    if (!gameOver)
    {
        // Confirm before starting a new game
        if (KMessageBox::warningYesNo(this, i18n("Are you sure you want to quit the current game?"), i18n("New game")) == KMessageBox::Yes)
        {
            // Start a new game
            initGame();
        }
        else
        {
            // If the game was running
            if (gameRunning)
            {
                // Resume the game
                m_game->start();
            }
        }
    }
    else
    {
        // Start a new game
        initGame();
    }
}

void MainWindow::setSoundsEnabled(bool p_enabled) {
	m_game->setSoundsEnabled(p_enabled);
}

void MainWindow::showSettings()
{
    if (KConfigDialog::showDialog("settings"))
    {
        return;
    }
    KConfigDialog* settingsDialog = new KConfigDialog(this, "settings", Settings::self());
    // General Settings
    settingsDialog->addPage(new GeneralSettings(settingsDialog), i18nc("General settings", "General"), "games-config-options");
    // Theme
    settingsDialog->addPage(new KGameThemeSelector(settingsDialog, Settings::self(), KGameThemeSelector::NewStuffDisableDownload), i18n("Theme"), "games-config-theme");
    // Arena
    settingsDialog->addPage(new ArenaSelector(settingsDialog, Settings::self(), ArenaSelector::NewStuffDisableDownload), i18n("Arena"), "games-config-board");
    // Player
    settingsDialog->addPage(new PlayerSelector(settingsDialog, m_playerSettings), i18n("Player"), "games-config-custom");
    
    connect(settingsDialog, SIGNAL(settingsChanged(const QString&)), this, SLOT(applyNewSettings()));
    connect(settingsDialog, SIGNAL(cancelClicked()), this, SLOT(settingsDialogCanceled()));
    settingsDialog->show();
}

void MainWindow::applyNewSettings()
{
    Settings::self()->setDummy(0);
    m_playerSettings->savePlayerSettings();
    initGame();
}

void MainWindow::settingsDialogCanceled()
{
    m_playerSettings->discardUnsavedSettings();
}

void MainWindow::close()
{
    bool gameRunning;       // True if the game is running (game timer is active), false otherwise
  
    gameRunning = m_game->getTimer()->isActive();
    // If the game is running
    if (gameRunning)
    {
        // Pause the game
        m_game->pause();
    }
    // Confirm before closing
    if(KMessageBox::warningYesNo(this, i18n("Are you sure you want to quit Granatier?"), i18nc("To quit Granatier", "Quit")) == KMessageBox::Yes)
    {
        KXmlGuiWindow::close();
    }
    else
    {
        // If the game was running
        if (gameRunning)
        {
            // Resume the game
            m_game->start();
        }
    }
}

