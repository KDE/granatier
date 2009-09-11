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
#include "gameview.h"
#include "gamescene.h"
#include "settings.h"
#include "arenaselector.h"
#include "ui_generalsettings.h"

#include <KActionCollection>
#include <KStandardGameAction>
#include <KToggleAction>
#include <KMessageBox>
#include <KConfigDialog>
#include <KGameThemeSelector>
#include <KInputDialog>
#include <KLocale>

class GeneralSettings : public QWidget
{
public:
    GeneralSettings(QWidget *parent) : QWidget(parent)
    {
        ui.setupUi(this);
    }
private:
    Ui::GeneralSettings ui;
};

MainWindow::MainWindow() {
	// Initialize the game
	m_game = NULL;
	m_view = NULL;
	// Set the window menus
	KStandardGameAction::gameNew(this, SLOT(newGame(bool)), actionCollection());
	KStandardGameAction::highscores(this, SLOT(showHighscores()), actionCollection());
	KStandardAction::preferences(this, SLOT(showSettings()), actionCollection());
	KStandardGameAction::quit(this, SLOT(close()), actionCollection());
    	KAction* soundAction = new KToggleAction(i18n("&Play sounds"), this);
	soundAction->setChecked(Settings::sounds());
	actionCollection()->addAction("sounds", soundAction);
	connect(soundAction, SIGNAL(triggered(bool)), this, SLOT(setSoundsEnabled(bool)));
    	KAction* levelAction = new KAction(i18n("&Change level"), this);
	actionCollection()->addAction("level", levelAction);
	connect(levelAction, SIGNAL(triggered(bool)), this, SLOT(changeLevel()));
    // init game
    initGame();
	// KScoreDialog
	m_kScoreDialog = new KScoreDialog(KScoreDialog::Name | KScoreDialog::Score | KScoreDialog::Level, this);
	// Setup the window
	setupGUI();
}

MainWindow::~MainWindow() {
	delete m_game;
	delete m_view;
	delete m_kScoreDialog;
}

void MainWindow::initGame() {
    //the focus has to be set at the beginning and also at the end to cover all possible cases
    //TODO: check why setting the focus only at the end doesn't work
    this->setFocusProxy(m_view);
    this->setFocus();
	// If a Game instance already exists
	if (m_game) {
		// Delete the Game instance
		delete m_game;
	}
	// Create a new Game instance
	m_game = new Game();
	connect(m_game, SIGNAL(gameOver(bool)), this, SLOT(newGame(bool)));		// TODO Remove the useless bool parameter from gameOver()
	// If a GameView instance already exists
	if (m_view) {
		// Delete the GameView instance
		delete m_view;
	}
	// Create a new GameView instance
	m_view = new GameView(m_game);
	m_view->setBackgroundBrush(Qt::black);
	setCentralWidget(m_view);
    m_game->setGameScene(dynamic_cast <GameScene*> (m_view->scene()));
	//m_view->setFocus();
    this->setFocusProxy(m_view);
    this->setFocus();
}

void MainWindow::newGame(const bool gameOver) {
	bool gameRunning;		// True if the game is running (game timer is active), false otherwise
   
	gameRunning = m_game->getTimer()->isActive();
	// If the game is running
	if (gameRunning) {
		// Pause the game
		m_game->pause();
	}
	// If the game was not over
	if (!gameOver){	
		// Confirm before starting a new game
		if (KMessageBox::warningYesNo(this, i18n("Are you sure you want to quit the current game?"), i18n("New game")) == KMessageBox::Yes) {
			// Start a new game
			initGame();
		}
		else {
			// If the game was running
			if (gameRunning) {
				// Resume the game
				m_game->start();
			}
		}
	}
	else {
		// Display the score information
		KMessageBox::information(this, i18n("The winner is %1.").arg(m_game->getWinner()), i18n("Game Over"));
		// Add the score to the highscores table
// 		m_kScoreDialog->setConfigGroup(KGameDifficulty::localizedLevelString());
// 		KScoreDialog::FieldInfo scoreInfo;
// 		scoreInfo[KScoreDialog::Level].setNum(m_game->getLevel());
// 		scoreInfo[KScoreDialog::Score].setNum(m_game->getScore());
// 		// If the new score is a highscore then display the highscore dialog
// 		if (m_kScoreDialog->addScore(scoreInfo)) {
// 			// If the payer did not cheat, manage Highscores
// 			if (!m_game->isCheater()) {
// 				m_kScoreDialog->exec();
// 			} else {		// else, warn the player not to cheat again :)
// 				KMessageBox::information(this, i18n("You cheated, no Highscore for you ;)"), i18n("Cheater!"));	
// 			}
// 		}
		
		// Start a new game
		initGame();
	}
}

void MainWindow::changeLevel() {
	int newLevel = KInputDialog::getInteger(i18n("Change level"), i18nc("The number of the game level", "Level"), m_game->getLevel(), 1, 1000000, 1, 10, 0, this);
	if (newLevel > 0) {
		m_game->setLevel(newLevel);
	}
}

void MainWindow::showHighscores() {
 	m_kScoreDialog->exec();
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
    m_nOldNumberOfPlayers = Settings::self()->players();
    m_nOldPointsToWin = Settings::self()->pointsToWin();
    settingsDialog->addPage(new GeneralSettings(settingsDialog), i18nc("General settings", "General"), "games-config-options");
    // Theme
    m_strOldTheme = Settings::self()->theme();
    settingsDialog->addPage(new KGameThemeSelector(settingsDialog, Settings::self(), KGameThemeSelector::NewStuffDisableDownload), i18n("Theme"), "games-config-theme");
    // Arena
    m_strOldArena = Settings::self()->arena();
    settingsDialog->addPage(new ArenaSelector(settingsDialog, Settings::self(), ArenaSelector::NewStuffDisableDownload), i18n("Arena"), "games-config-board");
        
    connect(settingsDialog, SIGNAL(settingsChanged(const QString&)), this, SLOT(loadSettings()));
    settingsDialog->show();
}

void MainWindow::loadSettings()
{
    if(m_strOldTheme != Settings::self()->theme())
    {
        ((GameScene*)m_view->scene())->loadTheme();
        m_strOldTheme = Settings::self()->theme();
    }
    if(m_strOldArena != Settings::self()->arena())
    {
        initGame();
        m_strOldArena = Settings::self()->arena();
    }
    if(m_nOldNumberOfPlayers != Settings::self()->players())
    {
        initGame();
        m_nOldNumberOfPlayers = Settings::self()->players();
    }
    if(m_nOldPointsToWin != Settings::self()->pointsToWin())
    {
        initGame();
        m_nOldPointsToWin = Settings::self()->pointsToWin();
    }
}

void MainWindow::close() {
	bool gameRunning;		// True if the game is running (game timer is active), false otherwise
   
	gameRunning = m_game->getTimer()->isActive();
	// If the game is running
	if (gameRunning) {
		// Pause the game
		m_game->pause();
	}
	// Confirm before closing
	if(KMessageBox::warningYesNo(this, i18n("Are you sure you want to quit Granatier?"), i18nc("To quit Granatier", "Quit")) == KMessageBox::Yes) {
		KXmlGuiWindow::close();
	}
	else {
		// If the game was running
		if (gameRunning) {
			// Resume the game
			m_game->start();
		}
	}
}

