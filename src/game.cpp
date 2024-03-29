/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
    SPDX-FileCopyrightText: 2007-2008 Pierre-Benoit Besse <besse@gmail.com>
    SPDX-FileCopyrightText: 2007-2008 Alexandre Galinier <alex.galinier@hotmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "game.h"
#include "mapparser.h"
#include "settings.h"
#include "gamescene.h"
#include "arena.h"
#include "player.h"
#include "bonus.h"
#include "bomb.h"
#include "block.h"
#include "config/playersettings.h"
#include "granatier_random.h"

#include <QPointF>
#include <QTimer>
#include <QKeyEvent>
#include <QDir>
#include <KConfig>
#include <KGameSound>
#include <QStandardPaths>


Game::Game(PlayerSettings* playerSettings)
{
    m_playerSettings = playerSettings;

    // Initialize the sound state
    setSoundsEnabled(Settings::sounds());
    m_wilhelmScream = Settings::useWilhelmScream();

    m_soundPutBomb = new KGameSound(QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("sounds/putbomb.wav")));
    m_soundExplode = new KGameSound(QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("sounds/explode.wav")));
    m_soundBonus = new KGameSound(QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("sounds/wow.wav")));
    m_soundFalling = new KGameSound(QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("sounds/deepfall.wav")));
    m_soundDie = new KGameSound(QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("sounds/die.wav")));

    m_arena = nullptr;
    m_randomArenaModeArenaList.clear();
    m_gameScene = nullptr;
    m_winPoints = Settings::self()->pointsToWin();

    QStringList strPlayerIDs = m_playerSettings->playerIDs();
    for(int i = 0; i < strPlayerIDs.count(); i++)
    {
        if(m_playerSettings->enabled(strPlayerIDs[i]))
        {
            auto* player = new Player(qreal(Granatier::CellSize * (-0.5)),qreal(Granatier::CellSize * 0.5), strPlayerIDs[i], playerSettings, m_arena);
            m_players.append(player);
            connect(player, &Player::dying, this, &Game::playerDeath);
            connect(player, &Player::falling, this, &Game::playerFalling);
            connect(player, &Player::resurrectBonusTaken, this, &Game::resurrectBonusTaken);
        }
    }

    init();

    for (auto & player : m_players)
    {
        connect(player, &Player::bombDropped, this, &Game::createBomb);
    }

    m_gameOver = false;
}

void Game::init()
{
    // Create the Arena instance
    m_arena = new Arena();

    m_remainingTime = Settings::roundTime();
    m_bombCount = 0;

    // Create the parser that will parse the XML file in order to initialize the Arena instance
    // This also creates all the characters
    MapParser mapParser(m_arena);
    // Set the XML file as input source for the parser
    QString filePath;
    if(Settings::self()->randomArenaMode())
    {
        if(m_randomArenaModeArenaList.isEmpty())
        {
            auto randomArenaModeArenaList = Settings::self()->randomArenaModeArenaList();

            QStringList arenasAvailable;
            const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::AppDataLocation, QStringLiteral("arenas"), QStandardPaths::LocateDirectory);
            for(const auto& dir: dirs) {
                const QStringList fileNames = QDir(dir).entryList({QStringLiteral("*.desktop")});
                for(const auto& file: fileNames) {
                    arenasAvailable.append(dir + QLatin1Char('/') + file);
                }
            }

            // store the random arenas if they are available
            for(const auto& randomArena: randomArenaModeArenaList) {
                for(const auto& arena: arenasAvailable) {
                    if(arena.endsWith(randomArena)) {
                        m_randomArenaModeArenaList.append(arena);
                        break;
                    }
                }
            }

            if(m_randomArenaModeArenaList.isEmpty()){
                m_randomArenaModeArenaList = arenasAvailable;
            }
        }

        int nIndex = granatier::RNG::fromRange<int>(0, m_randomArenaModeArenaList.count() - 1);
        filePath = m_randomArenaModeArenaList.at(nIndex);
        m_randomArenaModeArenaList.removeAt(nIndex);
    }
    else
    {
        filePath = QStandardPaths::locate(QStandardPaths::AppDataLocation, Settings::self()->arena());
    }

    if(!QFile::exists(filePath))
    {
        Settings::self()->useDefaults(true);
        filePath = QStandardPaths::locate(QStandardPaths::AppDataLocation, Settings::self()->arena());
        Settings::self()->useDefaults(false);
    }

    KConfig arenaConfig(filePath, KConfig::SimpleConfig);
    KConfigGroup group = arenaConfig.group(QStringLiteral("Arena"));
    QString arenaFileName = group.readEntry("FileName");

    QFile arenaXmlFile(QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("arenas/%1").arg(arenaFileName)));
    if (!arenaXmlFile.open(QIODevice::ReadOnly)) {
        qWarning() << " impossible to open file " << arenaFileName;
    }
    if (!mapParser.parse(&arenaXmlFile)) {
        qWarning() << " failed to parse file " << arenaFileName;
    }

    QString arenaName = group.readEntry("Name");
    m_arena->setName(arenaName);

    //create the block items
    for (int i = 0; i < m_arena->getNbRows(); ++i)
    {
        for (int j = 0; j < m_arena->getNbColumns(); ++j)
        {
            if(m_arena->getCell(i,j).getType() == Granatier::Cell::BLOCK)
            {
                auto* block = new Block((j + 0.5) * Granatier::CellSize, (i + 0.5) * Granatier::CellSize, m_arena, QStringLiteral("arena_block"));
                m_blocks.append(block);
                m_arena->setCellElement(i, j, block);
            }
        }
    }

    // Start the Game timer
    m_timer = new QTimer(this);
    m_timer->setInterval(int(1000 / Granatier::FPS));
    connect(m_timer, &QTimer::timeout, this, &Game::update);
    m_timer->start();
    m_state = RUNNING;

    m_roundTimer  = new QTimer(this);
    m_roundTimer->setInterval(1000);
    connect(m_roundTimer, &QTimer::timeout, this, &Game::decrementRemainingRoundTime);
    m_roundTimer->start();

    m_setRoundFinishedTimer  = new QTimer(this);
    m_setRoundFinishedTimer->setSingleShot(true);
    connect(m_setRoundFinishedTimer, &QTimer::timeout, this, &Game::setRoundFinished);

    // Init the characters coordinates on the Arena
    for (int i = 0; i < m_players.size(); i++)
    {
        m_players[i]->setArena(m_arena);
        QPointF playerPosition = m_arena->getPlayerPosition(i);
        m_players[i]->setInitialCoordinates(qreal(Granatier::CellSize * playerPosition.x()), qreal(Granatier::CellSize * playerPosition.y()));
    }
    initCharactersPosition();

    // Create the hidden Bonuses
    createBonus();
}

Game::~Game()
{
    //pause is needed to stop all animations and therefore the access of the *items to their model
    pause(true);

    qDeleteAll(m_players);
    m_players.clear();

    cleanUp();

    delete m_soundPutBomb;
    delete m_soundExplode;
    delete m_soundBonus;
    delete m_soundFalling;
    delete m_soundDie;
}

void Game::cleanUp()
{
    qDeleteAll(m_blocks);
    m_blocks.clear();
    qDeleteAll(m_bonus);
    m_bonus.clear();
    qDeleteAll(m_bombs);
    m_bombs.clear();
    delete m_arena;
    m_arena = nullptr;
    delete m_timer;
    m_timer = nullptr;
    delete m_roundTimer;
    m_roundTimer = nullptr;
    delete m_setRoundFinishedTimer;
    m_setRoundFinishedTimer = nullptr;
}

void Game::setGameScene(GameScene* p_gameScene)
{
    m_gameScene = p_gameScene;
}

void Game::start()
{
    // Restart the Game timer
    m_timer->start();
    m_state = RUNNING;
    m_roundTimer->start();
    Q_EMIT pauseChanged(false, false);
}

void Game::pause(bool p_locked)
{
    // Stop the Game timer
    m_timer->stop();
    m_roundTimer->stop();
    if (p_locked)
    {
        m_state = PAUSED_LOCKED;
    }
    else
    {
        m_state = PAUSED_UNLOCKED;
    }
    Q_EMIT pauseChanged(true, false);
}

void Game::switchPause()
{
    // If the Game is not already paused
    if (m_state == RUNNING)
    {
        // Pause the Game
        pause();
        Q_EMIT pauseChanged(true, true);
    }
    // If the Game is already paused
    else
    {
        // Resume the Game
        start();
        Q_EMIT pauseChanged(false, true);
    }
}

QList<Player*> Game::getPlayers() const
{
    return m_players;
}

QTimer* Game::getTimer() const
{
    return m_timer;
}

int Game::getRemainingTime() const
{
    return m_remainingTime;
}

Arena* Game::getArena() const
{
    return m_arena;
}

bool Game::isPaused() const
{
    return (m_state != RUNNING);
}

bool Game::getGameOver() const
{
    return m_gameOver;
}

QString Game::getWinner() const
{
    return m_strWinner;
}

int Game::getWinPoints() const
{
    return m_winPoints;
}

QList<Bonus*> Game::getBonus() const
{
    return m_bonus;
}

void Game::createBonus()
{
    int nRemainingNeutralBonuses = 1;
    for(int nQuarter = 0; nQuarter < 4; nQuarter++)
    {
        Bonus* bonus;
        int nBonusCount = static_cast<int>(0.3 * m_blocks.size() / 4);
        int nBadBonusCount = static_cast<int>(0.1 * m_blocks.size() / 4);
        int nNeutralBonusCount = granatier::RNG::fromRange(0, nRemainingNeutralBonuses);
        QList<Granatier::Bonus::Type> bonusTypeList;
        Granatier::Bonus::Type bonusType;
        int nFullSize = m_blocks.size();
        int nQuarterSize = (nQuarter < 3 ? nFullSize / 4 : nFullSize - 3 * (nFullSize / 4));

        for (int i = 0; i < nQuarterSize; ++i)
        {
            if(i < nBonusCount)
            {
                constexpr int NumberOfBonuses = 6;
                switch (granatier::RNG::fromRange(0, NumberOfBonuses-1))
                {
                    case 0: bonusType = Granatier::Bonus::SPEED;
                            break;
                    case 1: bonusType = Granatier::Bonus::BOMB;
                            break;
                    case 2: bonusType = Granatier::Bonus::POWER;
                            break;
                    case 3: bonusType = Granatier::Bonus::SHIELD;
                            break;
                    case 4: bonusType = Granatier::Bonus::THROW;
                            break;
                    case 5: bonusType = Granatier::Bonus::KICK;
                            break;
                    default: bonusType = Granatier::Bonus::SPEED;
                }
            }
            else if (i-nBonusCount < nBadBonusCount)
            {
                constexpr int NumberOfBadBonuses = 5;
                switch (granatier::RNG::fromRange(0, NumberOfBadBonuses-1))
                {
                    case 0: bonusType = Granatier::Bonus::HYPERACTIVE;
                            break;
                    case 1: bonusType = Granatier::Bonus::SLOW;
                            break;
                    case 2: bonusType = Granatier::Bonus::MIRROR;
                            break;
                    case 3: bonusType = Granatier::Bonus::SCATTY;
                            break;
                    case 4: bonusType = Granatier::Bonus::RESTRAIN;
                            break;
                    default: bonusType = Granatier::Bonus::HYPERACTIVE;
                }
            }
            else if(i-nBonusCount-nBadBonusCount < nNeutralBonusCount)
            {
                bonusType = Granatier::Bonus::RESURRECT;
                nRemainingNeutralBonuses--;
            }
            else {
                bonusType = Granatier::Bonus::NONE;
            }

            bonusTypeList.append(bonusType);
        }

        int nShuffle;
        for (int i = 0; i < nQuarterSize; ++i)
        {
            nShuffle = granatier::RNG::fromRange(0, nQuarterSize-1);

            bonusTypeList.swapItemsAt(i, nShuffle);
        }

        for (int i = 0; i < nQuarterSize; ++i)
        {
            if(bonusTypeList[i] != Granatier::Bonus::NONE)
            {
                int nIndex = nQuarter * (nFullSize/4) + i;
                bonus = new Bonus(m_blocks[nIndex]->getX(), m_blocks[nIndex]->getY(), m_arena, bonusTypeList[i]);
                m_bonus.append(bonus);
                m_blocks[nIndex]->setBonus(bonus);
            }
        }
    }
}

void Game::removeBonus(Bonus* bonus)
{
    m_bonus.removeAt(m_bonus.indexOf(bonus));
    //do not delete the Bonus, because the ElementItem will delete it
    if(m_soundEnabled && !bonus->isDestroyed())
    {
        m_soundBonus->start();
    }
}

void Game::setSoundsEnabled(bool p_enabled)
{
    m_soundEnabled = p_enabled;
    Settings::setSounds(p_enabled);
    Settings::self()->save();
}

void Game::initCharactersPosition()
{
    // If the timer is stopped, it means that collisions are already being handled
    if (m_timer->isActive())
    {
        // At the beginning, the timer is stopped but the Game isn't paused (to allow keyPressedEvent detection)
        m_timer->stop();
        m_roundTimer->stop();
        m_state = RUNNING;
        // Initialize the Player coordinates
        for(auto & player : m_players)
        {
            player->initCoordinate();
            player->init();
        }
    }
}

void Game::keyPressEvent(QKeyEvent* p_event)
{
    if(p_event->isAutoRepeat())
    {
        return;
    }

    // At the beginning or when paused, we start the timer when a key is pressed
    if (!m_timer->isActive())
    {
        if(p_event->key() == Qt::Key_Space)
        {
            // If paused
            if (m_state == PAUSED_UNLOCKED)
            {
                switchPause();
            }
            else if (m_state == RUNNING)      // At the game beginning
            {
                // Start the game
                m_timer->start();
                m_roundTimer->start();
                Q_EMIT gameStarted();
            }
            else if (m_state == PAUSED_LOCKED)
            {
                // if the game is over, start a new game
                if (m_gameOver)
                {
                    Q_EMIT gameOver();
                    return;
                }
                else
                {
                    m_gameScene->cleanUp();
                    cleanUp();
                    init();
                    m_gameScene->init();
                    for(auto & player : m_players)
                    {
                        player->resurrect();
                    }
                }
            }
        }
        return;
    }
    // Behaviour when the game has begun
    switch (p_event->key())
    {
        case Qt::Key_P:
        case Qt::Key_Escape:
            switchPause();
            return;
        default:
            break;
    }

    //TODO: make signal
    for(auto & player : m_players)
    {
        player->keyPressed(p_event);
    }
}

void Game::keyReleaseEvent(QKeyEvent* p_event)
{
    if(p_event->isAutoRepeat())
    {
        return;
    }
    //TODO: make signal
    for(auto & player : m_players)
    {
        player->keyReleased(p_event);
    }
}

void Game::update()
{
    //update Bombs
    for (auto & bomb : m_bombs)
    {
        bomb->updateMove();
    }

    //update Player
    for(auto & player : m_players)
    {
        player->updateMove();
        player->emitGameUpdated();
    }
}

void Game::decrementRemainingRoundTime()
{
    m_remainingTime--;
    if(m_remainingTime >= 0)
    {
        Q_EMIT infoChanged(Granatier::Info::TimeInfo);
    }
    else
    {
        if(m_remainingTime % 2 == 0)
        {
            //create bombs at randoms places
            int nRow;
            int nCol;
            Granatier::Cell::Type cellType;
            bool bFound = false;
            do
            {
                nRow = granatier::RNG::fromRange(0, m_arena->getNbRows()-1);
                nCol = granatier::RNG::fromRange(0, m_arena->getNbColumns()-1);
                cellType = m_arena->getCell(nRow, nCol).getType();
                if(cellType != Granatier::Cell::WALL && cellType != Granatier::Cell::HOLE && m_arena->getCell(nRow, nCol).isWalkable(nullptr))
                {
                    bFound = true;
                }
            }
            while (!bFound);

            m_bombCount++;
            Bomb* bomb = new Bomb((nCol + 0.5) * Granatier::CellSize, (nRow + 0.5) * Granatier::CellSize, m_arena, m_bombCount, 1000);    // time in ms
            bomb->setBombPower(static_cast<int>(granatier::RNG::fromRange(1.0, 2.5)));
            Q_EMIT bombCreated(bomb);
            connect(bomb, &Bomb::bombDetonated, this, &Game::bombDetonated);
            m_bombs.append(bomb);
            if(m_remainingTime > -100 && m_roundTimer->interval() > 150)
            {
                m_roundTimer->setInterval(m_roundTimer->interval() + m_remainingTime);
            }
            else if (m_roundTimer->interval() > 30)
            {
                m_roundTimer->setInterval(m_roundTimer->interval() - 1);
            }
        }
    }
}

void Game::playerFalling()
{
    if(m_soundEnabled)
    {
        m_soundFalling->start();
    }
}

void Game::playerDeath()
{
    if(m_soundEnabled)
    {
        m_soundDie->start();
    }

    //check if at most one player is alive if not already finished
    if(!m_setRoundFinishedTimer->isActive())
    {
        int nPlayerAlive = 0;
        for(auto & player : m_players)
        {
            if(player->isAlive())
            {
                nPlayerAlive++;
            }
        }
        if(nPlayerAlive <= 1)
        {
            //wait some time until the game stops
            m_setRoundFinishedTimer->start(1500);
        }
   }
}

void Game::resurrectBonusTaken()
{
    for(auto & player : m_players)
    {
        if(!(player->isAlive()))
        {
            player->resurrect();
        }
    }
}

void Game::setRoundFinished()
{
    int nPlayerAlive = 0;
    int nIndex = 0;;
    if(m_gameOver)
    {
        return;
    }
    for(int i = 0; i < m_players.length(); ++i)
    {
        if(m_players[i]->isAlive())
        {
            nPlayerAlive++;
            nIndex = i;
        }
    }
    //this check is needed, if in the meantime the resurrect bonus was taken
    if (nPlayerAlive > 1)
    {
        return;
    }

    if (nPlayerAlive == 1)
    {
        m_players[nIndex]->addPoint();
    }

    pause(true);

    for(auto & player : m_players)
    {
        // check if a player reaches the win points
        if (player->points() >= m_winPoints)
        {
            m_gameOver = true;
            m_strWinner = player->getPlayerName();
            break;
        }
    }
    m_gameScene->showScore();
}

void Game::createBomb(Player* player, qreal x, qreal y, bool newBomb, int throwDistance)
{
    int col = m_arena->getColFromX(x);
    int row = m_arena->getRowFromY(y);
    if(col >= 0 && col < m_arena->getNbColumns() && row >= 0 && row < m_arena->getNbRows())
    {
        QList<Element*> bombElements =  m_arena->getCell(row, col).getElements(Granatier::Element::BOMB);
        if (!bombElements.isEmpty())
        {
            if(player->hasThrowBomb() && throwDistance > 0)
            {
                for(auto& element: bombElements)
                {
                    dynamic_cast <Bomb*> (element)->setThrown(player->direction());
                }
            }
            return;
        }
    }

    if(!newBomb)
    {
        return;
    }

    m_bombCount++;
    Bomb* bomb = new Bomb((col + 0.5) * Granatier::CellSize, (row + 0.5) * Granatier::CellSize, m_arena, m_bombCount, 2500);    // time in ms
    bomb->setBombPower(player->getBombPower());
    Q_EMIT bombCreated(bomb);
    connect(bomb, &Bomb::bombDetonated, this, &Game::bombDetonated);
    connect(bomb, &Bomb::releaseBombArmory, player, &Player::slot_refillBombArmory);
    m_bombs.append(bomb);
    player->decrementBombArmory();

    if(m_soundEnabled)
    {
        m_soundPutBomb->start();
    }
}

void Game::removeBomb(Bomb* bomb)
{
    // Find the Bomb
    int index = m_bombs.indexOf(bomb);
    //remove the bomb
    if(index != -1)
    {
        //do not delete the bomb because it will be deleted through the destructor of elementitem
        m_bombs.removeAt(index);
    }
}

void Game::bombDetonated()
{
    if(m_soundEnabled)
    {
        m_soundExplode->start();
    }
}

void Game::blockDestroyed(const int row, const int col, Block* block)
{
    // find the Block
    int index = m_blocks.indexOf(block);
    // remove the Block
    if(index != -1)
    {
        //do not delete the block because it will be deleted through the destructor of elementitem
        m_arena->removeCellElement(row, col, block);
    }
}

#include "moc_game.cpp"
