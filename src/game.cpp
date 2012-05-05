/*
 * Copyright 2009 Mathias Kraus <k.hias@gmx.de>
 * Copyright 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
 * Copyright 2007-2008 Pierre-Benoit Besse <besse@gmail.com>
 * Copyright 2007-2008 Alexandre Galinier <alex.galinier@hotmail.com>
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

#include "game.h"
#include "mapparser.h"
#include "settings.h"
#include "gamescene.h"
#include "arena.h"
#include "player.h"
#include "bonus.h"
#include "bomb.h"
#include "block.h"
#include "playersettings.h"

#include <QPointF>
#include <QTimer>
#include <QKeyEvent>

#include <KStandardDirs>
#include <KConfig>
#include <KComponentData>
#include <kgsound.h>

const int Game::FPS = 40;

Game::Game(PlayerSettings* playerSettings)
{
    m_playerSettings = playerSettings;
    
    // Initialize the sound state
    setSoundsEnabled(Settings::sounds());
    m_wilhelmScream = Settings::useWilhelmScream();
    
    m_soundPutBomb = new KgSound(KStandardDirs::locate("appdata", "sounds/putbomb.wav"));
    m_soundExplode = new KgSound(KStandardDirs::locate("appdata", "sounds/explode.wav"));
    m_soundBonus = new KgSound(KStandardDirs::locate("appdata", "sounds/wow.wav"));
    m_soundFalling = new KgSound(KStandardDirs::locate("appdata", "sounds/deepfall.wav"));
    m_soundDie = new KgSound(KStandardDirs::locate("appdata", "sounds/die.wav"));
    
    m_arena = 0;
    m_gameScene = 0;
    m_winPoints = Settings::self()->pointsToWin();
    
    QStringList strPlayerIDs = m_playerSettings->playerIDs();
    for(int i = 0; i < strPlayerIDs.count(); i++)
    {
        if(m_playerSettings->enabled(strPlayerIDs[i]))
        {
            Player* player = new Player(qreal(Granatier::CellSize * (-0.5)),qreal(Granatier::CellSize * 0.5), strPlayerIDs[i], playerSettings, m_arena);
            m_players.append(player);
            connect(player, SIGNAL(dying(Player*)), this, SLOT(playerDeath(Player*)));
            connect(player, SIGNAL(falling()), this, SLOT(playerFalling()));
            connect(player, SIGNAL(resurrectBonusTaken()), this, SLOT(resurrectBonusTaken()));
        }
    }
    
    init();
    
    for (int i = 0; i < m_players.size(); i++)
    {
        connect(m_players[i], SIGNAL(bombDropped(Player*,qreal,qreal,bool,int)), this, SLOT(createBomb(Player*,qreal,qreal,bool,int)));
    }
    
    m_gameOver = false;
}

void Game::init()
{
    // Create the Arena instance
    m_arena = new Arena();

    m_roundFinished = 0;
    m_remainingTime = Settings::roundTime();
    m_bombCount = 0;
    
    // Create the parser that will parse the XML file in order to initialize the Arena instance
    // This also creates all the characters
    MapParser mapParser(m_arena);
    // Set the XML file as input source for the parser
    QString filePath;
    if(Settings::self()->randomArenaMode())
    {
        QStringList arenasAvailable;
        QStringList randomArenaModeArenaList = Settings::self()->randomArenaModeArenaList();
        
        KGlobal::dirs()->addResourceType("arenaselector", "data", KGlobal::mainComponent().componentName() + "/arenas/");
        KGlobal::dirs()->findAllResources("arenaselector", "*.desktop", KStandardDirs::Recursive, arenasAvailable);
        
        QStringList::Iterator i = randomArenaModeArenaList.begin();
        while(i != randomArenaModeArenaList.end())
        {
            if(arenasAvailable.contains(*i))
            {
                i++;
            }
            else
            {
                i = randomArenaModeArenaList.erase(i);
            }
        }
        
        if(randomArenaModeArenaList.isEmpty())
        {
            randomArenaModeArenaList = arenasAvailable;
        }
        
        qsrand(QDateTime::currentDateTime().toTime_t());
        int nIndex = ((double) qrand() / RAND_MAX) * randomArenaModeArenaList.count();
        if(nIndex < 0)
        {
            nIndex = 0;
        }
        else if(nIndex >= randomArenaModeArenaList.count())
        {
            nIndex = randomArenaModeArenaList.count() - 1;
        }
        filePath = KStandardDirs::locate("appdata", "arenas/" + randomArenaModeArenaList.at(nIndex));
    }
    else
    {
        filePath = KStandardDirs::locate("appdata", Settings::self()->arena());
    }
    
    if(!QFile::exists(filePath))
    {
        Settings::self()->useDefaults(true);
        filePath = KStandardDirs::locate("appdata", Settings::self()->arena());
        Settings::self()->useDefaults(false);
    }
    
    KConfig arenaConfig(filePath, KConfig::SimpleConfig);
    KConfigGroup group = arenaConfig.group("Arena");
    QString arenaFileName = group.readEntry("FileName");
    
    QFile arenaXmlFile(KStandardDirs::locate("appdata", QString("arenas/%1").arg(arenaFileName)));
    //QFile arenaXmlFile(KStandardDirs::locate("appdata", "arenas/granatier.xml"));
    QXmlInputSource source(&arenaXmlFile);
    // Create the XML file reader
    QXmlSimpleReader reader;
    reader.setContentHandler(&mapParser);
    // Parse the XML file
    reader.parse(source);
    
    QString arenaName = group.readEntry("Name");
    m_arena->setName(arenaName);
    
    //create the block items
    for (int i = 0; i < m_arena->getNbRows(); ++i)
    {
        for (int j = 0; j < m_arena->getNbColumns(); ++j)
        {
            if(m_arena->getCell(i,j).getType() == Granatier::Cell::BLOCK)
            {
                Block* block = new Block(i, j, m_arena, "arena_block");
                m_blocks.append(block);
                m_arena->setCellElement(i, j, block);
            }
        }
    }
    
    // Start the Game timer
    m_timer = new QTimer(this);
    m_timer->setInterval(int(1000 / Game::FPS));
    connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
    m_timer->start();
    m_state = RUNNING;
    
    m_roundTimer  = new QTimer(this);
    m_roundTimer->setInterval(1000);
    connect(m_roundTimer, SIGNAL(timeout()), this, SLOT(decrementRemainingRoundTime()));
    m_roundTimer->start();
    
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
    
    //bombs, bonuses and blocks are deletet by the destructor of their elementitem
    
    cleanUp();
    
    delete m_soundPutBomb;
    delete m_soundExplode;
    delete m_soundBonus;
    delete m_soundFalling;
    delete m_soundDie;
}

void Game::cleanUp()
{
    m_blocks.clear();
    m_bonus.clear();
    m_bombs.clear();
    delete m_arena;
    m_arena = 0;
    delete m_timer;
    m_timer = 0;
    delete m_roundTimer;
    m_roundTimer = 0;
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
    emit(pauseChanged(false, false));
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
    emit(pauseChanged(true, false));
}

void Game::switchPause(bool p_locked)
{
    // If the Game is not already paused
    if (m_state == RUNNING)
    {
        // Pause the Game
        pause(p_locked);
        emit(pauseChanged(true, true));
    }
    // If the Game is already paused
    else
    {
        // Resume the Game
        start();
        emit(pauseChanged(false, true));
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
    Bonus* bonus;
    qsrand(QDateTime::currentDateTime().toTime_t());
    int nBonusCount = 0.3 * m_blocks.size();
    int nBadBonusCount = 0.1 * m_blocks.size();
    int nNeutralBonusCount = static_cast <int> ((qrand()/1.0)/RAND_MAX * 2);
    QList<Granatier::Bonus::Type> bonusTypeList;
    Granatier::Bonus::Type bonusType;
    for (int i = 0; i < m_blocks.size(); i++)
    {
        bonusType = Granatier::Bonus::NONE;
        if(i < nBonusCount)
        {
            int nNumberOfBonuses = 6;
            switch (static_cast <int> ((qrand()/1.0)/RAND_MAX * nNumberOfBonuses))
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
            switch (static_cast <int> ((qrand()/1.0)/RAND_MAX * 5))
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
        }
        bonusTypeList.append(bonusType);
    }
    
    int nShuffle;
    for (int i = 0; i < m_blocks.size(); i++)
    {
        nShuffle = m_blocks.size() * (qrand()/1.0)/RAND_MAX;
        if(nShuffle >= m_blocks.size())
        {
            nShuffle = m_blocks.size() - 1;
        }
        else if(nShuffle < 0)
        {
            nShuffle = 0;
        }
        bonusTypeList.swap(i, nShuffle);
    }
    
    for (int i = 0; i < m_blocks.size(); ++i)
    {
        if(bonusTypeList[i] != Granatier::Bonus::NONE)
        {
            bonus = new Bonus(m_blocks[i]->getX(), m_blocks[i]->getY(), m_arena, bonusTypeList[i]);
            m_bonus.append(bonus);
            m_blocks[i]->setBonus(bonus);
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
    Settings::self()->writeConfig();
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
        for(int i = 0; i < m_players.size(); i++)
        {
            m_players[i]->initCoordinate();
            m_players[i]->init();
        }
        // Initialize the Block coordinates
        QList<Element*> blockElements;
        for (int i = 0; i < m_arena->getNbRows(); ++i)
        {
            for (int j = 0; j < m_arena->getNbColumns(); ++j)
            {
                blockElements =  m_arena->getCell(i, j).getElements(Granatier::Element::BLOCK);
                foreach(Element* element, blockElements)
                {
                    element->setX(Granatier::CellSize * (j + 0.5));
                    element->setY(Granatier::CellSize * (i + 0.5));
                }
            }
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
                emit(gameStarted());
            }
            else if (m_state == PAUSED_LOCKED)
            {
                // if the game is over, start a new game
                if (m_gameOver)
                {
                    emit(gameOver(true));
                    return;
                }
                else
                {
                    m_gameScene->cleanUp();
                    cleanUp();
                    init();
                    m_gameScene->init();
                    for(int i = 0; i < m_players.length(); i++)
                    {
                        m_players[i]->resurrect();
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
    for(int i = 0; i < m_players.size(); i++)
    {
        m_players[i]->keyPressed(p_event);
    }
}

void Game::keyReleaseEvent(QKeyEvent* p_event)
{
    if(p_event->isAutoRepeat())
    {
        return;
    }
    //TODO: make signal
    for(int i = 0; i < m_players.size(); i++)
    {
        m_players[i]->keyReleased(p_event);
    }
}

void Game::update()
{
    //update Bombs
    for (int i = 0; i < m_bombs.size(); ++i)
    {
        m_bombs[i]->updateMove();
    }
    
    //update Player
    for(int i = 0; i < m_players.size(); i++)
    {
        m_players[i]->updateMove();
        m_players[i]->emitGameUpdated();
    }
}

void Game::decrementRemainingRoundTime()
{
    m_remainingTime--;
    if(m_remainingTime >= 0)
    {
        emit(infoChanged(Granatier::Info::TimeInfo));
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
                nRow = m_arena->getNbRows() * (qrand()/1.0)/RAND_MAX;
                nCol = m_arena->getNbColumns() * (qrand()/1.0)/RAND_MAX;
                cellType = m_arena->getCell(nRow, nCol).getType();
                if(cellType != Granatier::Cell::WALL && cellType != Granatier::Cell::HOLE && cellType != Granatier::Cell::BLOCK)
                {
                    bFound = true;
                }
            }
            while (!bFound);
            
            m_bombCount++;
            Bomb* bomb = new Bomb((nCol + 0.5) * Granatier::CellSize, (nRow + 0.5) * Granatier::CellSize, m_arena, m_bombCount, 1000);    // time in ms
            bomb->setBombPower(1);
            emit bombCreated(bomb);
            connect(bomb, SIGNAL(bombDetonated(Bomb*)), this, SLOT(bombDetonated(Bomb*)));
            m_bombs.append(bomb);
            if(m_remainingTime > -100 && m_roundTimer->interval() > 150)
            {
                m_roundTimer->setInterval(m_roundTimer->interval() + m_remainingTime);
            }
            else if (m_roundTimer->interval() > 40)
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

void Game::playerDeath(Player* player)
{
    //wait some time until the game stops
    QTimer::singleShot(1500, this, SLOT(checkRoundFinished()));   

    if(m_soundEnabled)
    {
        m_soundDie->start();
    }
}

void Game::resurrectBonusTaken()
{
    for(int i = 0; i < m_players.length(); i++)
    {
        if(!(m_players[i]->isAlive()))
        {
            m_players[i]->resurrect();
        }
    }
}

void Game::checkRoundFinished()
{
    int nPlayerAlive = 0;
    int nIndex = 0;;
    if(m_gameOver)
    {
        return;
    }
    for(int i = 0; i < m_players.length(); i++)
    {
        if(m_players[i]->isAlive())
        {
            nPlayerAlive++;
            nIndex = i;
        }
    }
    if (nPlayerAlive > 1)
    {
        return;
    }

    if(!m_roundFinished)
    {
        // Start the timer
        start();
        
        if (nPlayerAlive == 1)
        {
            m_roundFinished = 1;
            m_players[nIndex]->addPoint();
        }
        
        pause(true);
        
        for(int i = 0; i < m_players.length(); i++)
        {
            // check if a player reaches the win points
            if (m_players[i]->points() >= m_winPoints)
            {
                m_gameOver = true;
                m_strWinner = m_players[i]->getPlayerName();
                break;
            }
        }
        m_gameScene->showScore();
    }
    
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
                foreach(Element* element, bombElements)
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
    emit bombCreated(bomb);
    connect(bomb, SIGNAL(bombDetonated(Bomb*)), this, SLOT(bombDetonated(Bomb*)));
    connect(bomb, SIGNAL(releaseBombArmory()), player, SLOT(slot_refillBombArmory()));
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

void Game::bombDetonated(Bomb* bomb)
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
