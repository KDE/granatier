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
#include <KComponentData>
#include <KConfig>
#ifdef GRANATIER_USE_GLUON
#include <KDE/KALEngine>
#include <KDE/KALSound>
#include <KDE/KALBuffer>
#else
#include <Phonon/MediaObject>
#endif

const int Game::FPS = 40;

Game::Game(PlayerSettings* playerSettings) : m_isCheater(false), m_level(1)
{
    m_playerSettings = playerSettings;
    
    // Initialize the sound state
    setSoundsEnabled(Settings::sounds());
    m_wilhelmScream = Settings::useWilhelmScream();
    
    //init KALEngine
    #ifdef GRANATIER_USE_GLUON
    soundEngine = KALEngine::instance();
    soundPutBomb = new KALSound(new KALBuffer(KStandardDirs::locate("appdata", "sounds/putbomb.ogg")), soundEngine);
    soundExplode = new KALSound(new KALBuffer(KStandardDirs::locate("appdata", "sounds/explode.ogg")), soundEngine);
    soundBonus = new KALSound(new KALBuffer(KStandardDirs::locate("appdata", "sounds/wow.ogg")), soundEngine);
    soundBufferDie = new KALBuffer(KStandardDirs::locate("appdata", "sounds/die.ogg"));
    soundBufferWilhelmScream = new KALBuffer(KStandardDirs::locate("appdata", "sounds/wilhelmscream.ogg"));
    soundDie = new KALSound(soundBufferDie, soundEngine);
    soundWilhelmScream = new KALSound(soundBufferWilhelmScream, soundEngine);
    #else
    for(int i = 0; i < 3; i++)
    {
        m_phononPutBomb.append(Phonon::createPlayer(Phonon::GameCategory));
        m_phononPutBomb.last()->setCurrentSource(KStandardDirs::locate("appdata", "sounds/putbomb.ogg"));
    }
    m_phononPutBombTimer = new QTimer(this);
    m_phononPutBombTimer->setSingleShot(true);
    for(int i = 0; i < 3; i++)
    {
        m_phononExplode.append(Phonon::createPlayer(Phonon::GameCategory));
        m_phononExplode.last()->setCurrentSource(KStandardDirs::locate("appdata", "sounds/explode.ogg"));
    }
    m_phononExplodeTimer = new QTimer(this);
    m_phononExplodeTimer->setSingleShot(true);
    for(int i = 0; i < 3; i++)
    {
        m_phononBonus.append(Phonon::createPlayer(Phonon::GameCategory));
        m_phononBonus.last()->setCurrentSource(KStandardDirs::locate("appdata", "sounds/wow.ogg"));
    }
    m_phononBonusTimer = new QTimer(this);
    m_phononBonusTimer->setSingleShot(true);
    for(int i = 0; i < 2; i++)
    {
        m_phononDie.append(Phonon::createPlayer(Phonon::GameCategory));
        m_phononDie.last()->setCurrentSource(KStandardDirs::locate("appdata", "sounds/die.ogg"));
    }
    m_phononDieTimer = new QTimer(this);
    m_phononDieTimer->setSingleShot(true);
    #endif
    
    m_arena = 0;
    m_gameScene = 0;
    m_winPoints = Settings::self()->pointsToWin();
    
    QStringList strPlayerIDs = m_playerSettings->playerIDs();
    for(int i = 0; i < strPlayerIDs.count(); i++)
    {
        if(m_playerSettings->enabled(strPlayerIDs[i]))
        {
            Player* player = new Player(qreal(Cell::SIZE * (-0.5)),qreal(Cell::SIZE * 0.5), strPlayerIDs[i], playerSettings, m_arena);
            m_players.append(player);
            connect(player, SIGNAL(dying(Player*)), this, SLOT(playerDeath(Player*)));
        }
    }
    
    init();
    
    for (int i = 0; i < m_players.size(); i++)
    {
        connect(m_players[i], SIGNAL(bombDropped(Player*, qreal, qreal)), this, SLOT(createBomb(Player*, qreal, qreal)));
    }
    
    m_gameOver = false;
}

void Game::init()
{
    // Create the Arena instance
    m_arena = new Arena();

    m_roundOver = 0;
    
    // Create the parser that will parse the XML file in order to initialize the Arena instance
    // This also creates all the characters
    MapParser mapParser(this);
    // Set the XML file as input source for the parser
    QString filePath = KStandardDirs::locate("appdata", Settings::self()->arena());
    KConfig arenaConfig(filePath, KConfig::SimpleConfig);
    KConfigGroup group = arenaConfig.group("Arena");
    QString arenaName = group.readEntry("FileName");
    QFile arenaXmlFile(KStandardDirs::locate("appdata", QString("arenas/%1").arg(arenaName)));
    //QFile arenaXmlFile(KStandardDirs::locate("appdata", "arenas/granatier.xml"));
    QXmlInputSource source(&arenaXmlFile);
    // Create the XML file reader
    QXmlSimpleReader reader;
    reader.setContentHandler(&mapParser);
    // Parse the XML file
    reader.parse(source);
    
    // Start the Game timer
    m_timer = new QTimer(this);
    m_timer->setInterval(int(1000 / Game::FPS));
    connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
    m_timer->start();
    m_state = RUNNING;
    
    // Init the characters coordinates on the Arena
    for (int i = 0; i < m_players.size(); i++)
    {
        m_players[i]->setArena(m_arena);
        m_players[i]->setInitialCoordinates(qreal(Cell::SIZE * m_arena->getPlayerPosition(i+1).x()), qreal(Cell::SIZE * m_arena->getPlayerPosition(i+1).y()));
    }
    initCharactersPosition();
    
    // Create the hidden Bonuses
    createBonus();
}

Game::~Game()
{
    for (int i = 0; i < m_players.size(); i++)
    {
        delete m_players[i];
    }
        
    //bombs, bonuses and blocks are deletet by the destructor of their elementitem
    
    cleanUp();
    
    #ifdef GRANATIER_USE_GLUON
    delete soundPutBomb;
    delete soundExplode;
    delete soundBonus;
    delete soundWilhelmScream;
    delete soundDie;
    delete soundBufferWilhelmScream;
    delete soundBufferDie;
    #else
    while(!(m_phononPutBomb.isEmpty()))
    {
        delete m_phononPutBomb.takeLast();
    }
    delete m_phononPutBombTimer;
    while(!(m_phononExplode.isEmpty()))
    {
        delete m_phononExplode.takeLast();
    }
    delete m_phononExplodeTimer;
    while(!(m_phononBonus.isEmpty()))
    {
        delete m_phononBonus.takeLast();
    }
    delete m_phononBonusTimer;
    while(!(m_phononDie.isEmpty()))
    {
        delete m_phononDie.takeLast();
    }
    delete m_phononDieTimer;
    #endif
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
}

void Game::setGameScene(GameScene* p_gameScene)
{
    m_gameScene = p_gameScene;
}

void Game::start() {
	// Restart the Game timer
	m_timer->start();
	m_state = RUNNING;
	emit(pauseChanged(false, false));
}

void Game::pause(bool p_locked) {
	// Stop the Game timer
	m_timer->stop();
	if (p_locked) {
		m_state = PAUSED_LOCKED;
	} else {
		m_state = PAUSED_UNLOCKED;
	}
	emit(pauseChanged(true, false));
}

void Game::switchPause(bool p_locked) {
	// If the Game is not already paused
	if (m_state == RUNNING) {
		// Pause the Game
		pause(p_locked);
		emit(pauseChanged(true, true));
	}
	// If the Game is already paused
	else {
		// Resume the Game
		start();
		emit(pauseChanged(false, true));
	}
}

QList<Player*> Game::getPlayers() const 
{
    return m_players;
}

QTimer* Game::getTimer() const {
	return m_timer;
}

Arena* Game::getArena() const {
	return m_arena;
}

bool Game::isPaused() const {
	return (m_state != RUNNING);
}

bool Game::isCheater() const {
	return m_isCheater;
}

int Game::getLives() const {
	return 3;//m_lives;
}

int Game::getLevel() const {
	return m_level;
}

void Game::setLevel(int p_level) {
	m_isCheater = true;
	m_level = p_level;
	m_timer->start();	// Needed to reinit character positions
	initCharactersPosition();
    
    for (int i = 0; i < m_players.size(); i++)
    {
        m_players[i]->initSpeed();
    }

	//m_bonus->setPoints(m_level * 100);
	emit(dataChanged(AllInfo));
	emit(pauseChanged(false, true));
	emit(levelStarted(true));
}

QString Game::getWinner() const
{
    return m_strWinner;
}

QList<Bonus*> Game::getBonus()
{
    return m_bonus;
}

void Game::createBonus()
{
    Bonus* bonus;
    qsrand(QDateTime::currentDateTime().toTime_t());
    int nBonusCount = 0.3 * m_blocks.size();
    QList<Bonus::BonusType> bonusTypeList;
    Bonus::BonusType bonusType;
    for (int i = 0; i < m_blocks.size(); i++)
    {
        bonusType = Bonus::NONE;
        if(i < nBonusCount)
        {
            switch (static_cast <int> ((qrand()/1.0)/RAND_MAX * 3/*5*/))
            {
                case 0: bonusType = Bonus::SPEED;
                        break;
                case 1: bonusType = Bonus::BOMB;
                        break;
                case 2: bonusType = Bonus::RANGE;
                        break;
                case 3: bonusType = Bonus::THROW;
                        break;
                case 4: bonusType = Bonus::MOVE;
                        break;
                default: bonusType = Bonus::SPEED;
            }
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
        if(bonusTypeList[i] != Bonus::NONE)
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
    if(m_soundEnabled)
    {
        #ifdef GRANATIER_USE_GLUON
        soundBonus->play();
        #else
        qint64 nLastRemainingTime;
        int nIndex = 0;
        if(m_phononBonusTimer->isActive())
        {
            return;
        }
        nLastRemainingTime = m_phononBonus.first()->remainingTime();
        nIndex = 0;
        for(int i = 0; i < m_phononBonus.count(); i++)
        {
            if(nLastRemainingTime > m_phononBonus.at(i)->remainingTime())
            {
                nIndex = i;
            }
        }
        
        m_phononBonus.at(nIndex)->play();
        m_phononBonusTimer->start(50);
        #endif
    }
}

void Game::createBlock(QPointF p_position, const QString& p_imageId)
{
    Block* block = new Block(p_position.y(), p_position.x(), m_arena, p_imageId);
    m_blocks.append(block);
    m_arena->setCellElement(p_position.y(), p_position.x(), block);
}

void Game::setSoundsEnabled(bool p_enabled)
{
    m_soundEnabled = p_enabled;
    Settings::setSounds(p_enabled);
    Settings::self()->writeConfig();
}

void Game::initCharactersPosition() {
	// If the timer is stopped, it means that collisions are already being handled
	if (m_timer->isActive()) {	
		// At the beginning, the timer is stopped but the Game isn't paused (to allow keyPressedEvent detection)
		m_timer->stop();
		m_state = RUNNING;
        // Initialize the Player coordinates
        for(int i = 0; i < m_players.size(); i++)
        {
            m_players[i]->initCoordinate();
            m_players[i]->init();
        }
        // Initialize the Block coordinates
        for (int i = 0; i < m_arena->getNbRows(); ++i)
        {
            for (int j = 0; j < m_arena->getNbColumns(); ++j)
            {
                if (m_arena->getCell(i,j).getElement() != NULL && m_arena->getCell(i,j).getElement()->getType() == Element::BLOCK)
                {
                    m_arena->getCell(i,j).getElement()->setX(Cell::SIZE * (j + 0.5));
                    m_arena->getCell(i,j).getElement()->setY(Cell::SIZE * (i + 0.5));
                }
            }
        }
    }
}

void Game::keyPressEvent(QKeyEvent* p_event)
{
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
                emit(gameStarted());
            }
            else if (m_state == PAUSED_LOCKED)
            {
                m_gameScene->cleanUp();
                cleanUp();
                init();
                m_gameScene->init();
                for(int i = 0; i < m_players.length(); i++)
                {
                    m_players[i]->resurrect();
                    // If a player reaches the win points start a new game
                    if (m_players[i]->points() >= m_winPoints)
                    {
                        m_gameOver = true;
                        m_strWinner = m_players[i]->getPlayerName();
                        emit(gameOver(true));
                        return;
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
        case Qt::Key_E:
            // Cheat code to get one more life
            if (p_event->modifiers() == (Qt::AltModifier | Qt::ControlModifier | Qt::ShiftModifier))
            {
                //m_lives++;
                m_isCheater = true;
                emit(dataChanged(LivesInfo));
            }
            return;
        case Qt::Key_N:
            // Cheat code to go to the next level
            if (p_event->modifiers() == (Qt::AltModifier | Qt::ControlModifier | Qt::ShiftModifier))
            {
                m_isCheater = true;
                nextLevel();
            }
            return;
        default:
            break;
    }
    
    for(int i = 0; i < m_players.size(); i++)
    {
        m_players[i]->keyPressed(p_event);
    }
}

void Game::keyReleaseEvent(QKeyEvent* p_event)
{
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

void Game::playerDeath(Player* player)
{
    //m_player->die();
    //wait some time until the game stops
    QTimer::singleShot(1500, this, SLOT(resumeAfterPlayerDeath()));   

    if(m_soundEnabled)
    {
        #ifdef GRANATIER_USE_GLUON
        if(m_wilhelmScream)
        {
            soundWilhelmScream->play();
        }
        else
        {
            soundDie->play();
        }
        #else
        qint64 nLastRemainingTime;
        int nIndex = 0;
        if(m_phononDieTimer->isActive())
        {
            return;
        }
        nLastRemainingTime = m_phononDie.first()->remainingTime();
        nIndex = 0;
        for(int i = 0; i < m_phononDie.count(); i++)
        {
            if(nLastRemainingTime > m_phononDie.at(i)->remainingTime())
            {
                nIndex = i;
            }
        }
        
        m_phononDie.at(nIndex)->play();
        m_phononDieTimer->start(50);
        #endif
    }
}

void Game::resumeAfterPlayerDeath()
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

    if(!m_roundOver)
    {
        // Start the timer
        start();
        
        if (nPlayerAlive == 1)
        {
            m_roundOver = 1;
            m_players[nIndex]->addPoint();
        }
        
        pause(true);
        m_gameScene->showPoints(m_winPoints);
    }
    
}

void Game::winPoints(Element* p_element)
{
    if (p_element->getType() == Element::BONUS)
    {
        // Get the position of the Bonus
        qreal xPos = p_element->getX();
        qreal yPos = p_element->getY();

        emit(bonusOff());
    }
}

void Game::nextLevel() {
	// Increment the level
	m_level++;
	// Update Bonus
	//->setPoints(m_level * 100);
	// Move all characters to their initial positions
	initCharactersPosition();
	// Update the score, level and lives labels
	emit(dataChanged(AllInfo));
	// Update the view
	emit(levelStarted(true));
}

void Game::createBomb(Player* player, qreal x, qreal y)
{
    int col = m_arena->getColFromX(x);
    int row = m_arena->getRowFromY(y);
    if(col >= 0 && col < m_arena->getNbColumns() && row >= 0 && row < m_arena->getNbRows())
    {
        if(m_arena->getCell(row, col).getElement() != NULL && m_arena->getCell(row, col).getElement()->getType() == Element::BOMB)
        {
            return;
        }
    }
    Bomb* bomb = new Bomb((col + 0.5) * Cell::SIZE, (row + 0.5) * Cell::SIZE, m_arena, 2500);    // time in ms
    bomb->setBombRange(player->getBombRange());
    emit bombCreated(bomb);
    connect(bomb, SIGNAL(bombDetonated(Bomb*)), this, SLOT(slot_bombDetonated(Bomb*)));
    connect(bomb, SIGNAL(bombDetonated(Bomb*)), player, SLOT(slot_refillBombArmory()));
    m_bombs.append(bomb);
    player->decrementBombArmory();
    
    if(m_soundEnabled)
    {
        #ifdef GRANATIER_USE_GLUON
        soundPutBomb->play();
        #else
        qint64 nLastRemainingTime;
        int nIndex = 0;
        if(m_phononPutBombTimer->isActive())
        {
            return;
        }
        nLastRemainingTime = m_phononPutBomb.first()->remainingTime();
        nIndex = 0;
        for(int i = 0; i < m_phononPutBomb.count(); i++)
        {
            if(nLastRemainingTime > m_phononPutBomb.at(i)->remainingTime())
            {
                nIndex = i;
            }
        }
        
        m_phononPutBomb.at(nIndex)->play();
        m_phononPutBombTimer->start(25);
        #endif
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

void Game::slot_bombDetonated(Bomb* bomb)
{
    if(m_soundEnabled)
    {
        #ifdef GRANATIER_USE_GLUON
        soundExplode->play();
        #else
        qint64 nLastRemainingTime;
        int nIndex = 0;
        if(m_phononExplodeTimer->isActive())
        {
            return;
        }
        nLastRemainingTime = m_phononExplode.first()->remainingTime();
        nIndex = 0;
        for(int i = 0; i < m_phononExplode.count(); i++)
        {
            if(nLastRemainingTime > m_phononExplode.at(i)->remainingTime())
            {
                nIndex = i;
            }
        }
        
        m_phononExplode.at(nIndex)->play();
        m_phononExplodeTimer->start(50);
        #endif
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
