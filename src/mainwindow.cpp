/*
    SPDX-FileCopyrightText: 2008 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
    SPDX-FileCopyrightText: 2007-2008 Pierre-Benoit Bessse <besse@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mainwindow.h"
#include "game.h"
#include "gameview.h"
#include "gamescene.h"
#include "settings.h"
#include "config/arenaselector.h"
#include "config/playersettings.h"
#include "config/playerselector.h"
#include "ui_generalsettings.h"
// KDEGames
#include <kdegames_version.h>
#include <KStandardGameAction>
#include <KgThemeSelector>
// KF
#include <kwidgetsaddons_version.h>
#include <KStandardGuiItem>
#include <KActionCollection>
#include <KMessageBox>
#include <KConfigDialog>
#include <KLocalizedString>
#include <KToggleAction>
// Qt
#include <QGraphicsView>
#include <QTimer>
#include <QPushButton>

class GeneralSettings : public QWidget
{
public:
    explicit GeneralSettings(QWidget *parent) : QWidget(parent)
    {
        ui.setupUi(this);
        ui.groupBox->setVisible(false);
        ui.kcfg_Dummy->setVisible(false); // this is only to notify changes in playerselector
    }
private:
    Ui::GeneralSettings ui;
};

MainWindow::MainWindow()
{
    m_settingsDialog = nullptr;
    // Initialize the game
    m_game = nullptr;
    m_view = nullptr;
    m_scene = nullptr;
    m_playerSettings = new PlayerSettings();
    m_themeProvider = new KgThemeProvider(QByteArray("Theme"), this);
#if KDEGAMES_VERSION >= QT_VERSION_CHECK(7, 4, 0)
    m_themeProvider->discoverThemes(QStringLiteral("themes"), QStringLiteral("granatier"));
#else
    m_themeProvider->discoverThemes(QByteArray("appdata"), QStringLiteral("themes"), QStringLiteral("granatier"));
#endif
    // Set the window menus
    KStandardGameAction::gameNew(this, &MainWindow::newGame, actionCollection());
    //KStandardGameAction::highscores(this, &MainWindow::showHighscores, actionCollection());
    KStandardAction::preferences(this, &MainWindow::showSettings, actionCollection());
    KStandardGameAction::quit(this, &MainWindow::close, actionCollection());
    auto* soundAction = new KToggleAction(i18n("&Play sounds"), this);
    soundAction->setChecked(Settings::sounds());
    actionCollection()->addAction( QStringLiteral( "sounds" ), soundAction);
    connect(soundAction, &KToggleAction::triggered, this, &MainWindow::setSoundsEnabled);
    // init game
    initGame();
    // Setup the window
    setupGUI(Keys | Save | Create);
}

MainWindow::~MainWindow()
{
    delete m_view;
    delete m_scene;
    delete m_game;
    delete m_playerSettings;
    delete m_settingsDialog;
}

void MainWindow::initGame()
{
    //the focus has to be set at the beginning and also at the end to cover all possible cases
    //TODO: check why setting the focus only at the end doesn't work
    this->setFocusProxy(m_view);
    this->setFocus();

    // If a GameView instance already exists
    if (m_view)
    {
        // Delete the GameView instance
        delete m_view;
    }

    // If a GameScene instance already exists
    if (m_scene)
    {
        // Delete the GameScene instance
        delete m_scene;
    }

    // If a Game instance already exists
    if (m_game)
    {
        // Delete the Game instance
        delete m_game;
    }
    // Create a new Game instance
    m_game = new Game(m_playerSettings);
    connect(m_game, &Game::gameOver, this, &MainWindow::newGame);

    m_scene = new GameScene(m_game, m_themeProvider);

    // Create a new GameView instance
    m_view = new GameView(m_scene, m_game);
    m_view->setBackgroundBrush(Qt::black);
    setCentralWidget(m_view);
    m_game->setGameScene(dynamic_cast <GameScene*> (m_view->scene()));

    this->setFocusProxy(m_view);
    this->setFocus();
}

void MainWindow::newGame()
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
    if (!m_game->getGameOver())
    {
        // Confirm before starting a new game
#if KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(5, 100, 0)
        if (KMessageBox::warningTwoActions(this,
#else
        if (KMessageBox::warningYesNo(this,
#endif
                                      i18n("Are you sure you want to quit the current game?"),
                                      i18nc("@title:window", "New Game"),
                                      KGuiItem(i18nc("@action:button", "Quit Game"), QStringLiteral("window-close")),
                                      KStandardGuiItem::cancel())
#if KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(5, 100, 0)
            == KMessageBox::PrimaryAction)
#else
            == KMessageBox::Yes)
#endif
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
    if (m_settingsDialog)//(KConfigDialog::showDialog("settings"))
    {
        delete m_settingsDialog;
    }
    auto* settingsDialog = new KConfigDialog(this, QStringLiteral("settings"), Settings::self());
    settingsDialog->setMinimumSize(900, 600);
    // General Settings
    settingsDialog->addPage(new GeneralSettings(settingsDialog), i18nc("General settings", "General"), QStringLiteral("games-config-options"));
    // Theme
    m_themeProvider->rediscoverThemes();
    m_currentThemeIdentifier = QString::fromLatin1(m_themeProvider->currentTheme()->identifier());
    settingsDialog->addPage(new KgThemeSelector(m_themeProvider, KgThemeSelector::DefaultBehavior, settingsDialog), i18n("Theme"), QStringLiteral("games-config-theme"));
    // Arena
    settingsDialog->addPage(new ArenaSelector(settingsDialog, Settings::self(), &m_tempRandomArenaModeArenaList, ArenaSelector::DefaultBehavior), i18n("Arena"), QStringLiteral("games-config-board"));
    // Player
    settingsDialog->addPage(new PlayerSelector(m_playerSettings, PlayerSelector::DefaultBehavior, settingsDialog), i18n("Player"), QStringLiteral("games-config-custom"));

    m_settingsDialog = settingsDialog;

    connect(settingsDialog, &KConfigDialog::settingsChanged, this, &MainWindow::applyNewSettings);
    connect(settingsDialog->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &MainWindow::settingsDialogCanceled);
    settingsDialog->show();
}

void MainWindow::applyNewSettings()
{
    Settings::self()->setDummy(0);
    m_playerSettings->savePlayerSettings();
    if(!m_tempRandomArenaModeArenaList.isEmpty())
    {
        Settings::self()->setRandomArenaModeArenaList(m_tempRandomArenaModeArenaList);
    }
    initGame();
}

void MainWindow::settingsDialogCanceled()
{
    m_playerSettings->discardUnsavedSettings();
    m_tempRandomArenaModeArenaList.clear();
    if(m_currentThemeIdentifier != QString::fromLatin1(m_themeProvider->currentTheme()->identifier()))
    {
        const QList<const KgTheme*> themeList = m_themeProvider->themes();
        for(const auto& theme: themeList)
        {
            if(QString::fromLatin1(theme->identifier()) == m_currentThemeIdentifier)
            {
                m_themeProvider->setCurrentTheme(theme);
                break;
            }
        }
    }
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
#if KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(5, 100, 0)
    if(KMessageBox::warningTwoActions(this,
#else
    if(KMessageBox::warningYesNo(this,
#endif
                                 i18n("Are you sure you want to quit Granatier?"),
                                 i18nc("To quit Granatier", "Quit"),
                                 KStandardGuiItem::quit(), KStandardGuiItem::cancel())
#if KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(5, 100, 0)
       == KMessageBox::PrimaryAction)
#else
       == KMessageBox::Yes)
#endif
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

#include "moc_mainwindow.cpp"
