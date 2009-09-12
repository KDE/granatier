/*
 * Copyright 2008 Mathias Kraus <k.hias@gmx.de>
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

#include <QPointF>
#include <QTimer>
#include <QKeyEvent>

#include <KStandardDirs>

#ifdef GRANATIER_USE_GLUON
#include <KDE/KALEngine>
#include <KDE/KALSound>
#include <KDE/KALBuffer>
#endif
//TODO: #else
#include <Phonon/MediaObject>
//#endif

const int Game::FPS = 40;

Game::Game() : m_isCheater(false), m_points(0), m_level(1), m_media1(0), m_media2(0)
{
    // Initialize the sound state
    setSoundsEnabled(Settings::sounds());
    
    //init KALEngine
    #ifdef GRANATIER_USE_GLUON
    soundEngine = KALEngine::instance();
    soundPutBomb = new KALSound(new KALBuffer(KStandardDirs::locate("appdata", "sounds/putbomb.ogg")), soundEngine);
    soundExplode = new KALSound(new KALBuffer(KStandardDirs::locate("appdata", "sounds/explode.ogg")), soundEngine);
    soundExplode->setGain(1);
    soundBonus = new KALSound(new KALBuffer(KStandardDirs::locate("appdata", "sounds/wow.ogg")), soundEngine);
    soundBufferDie = new KALBuffer(KStandardDirs::locate("appdata", "sounds/die.ogg"));
    soundBufferWilhelmScream = new KALBuffer(KStandardDirs::locate("appdata", "sounds/wilhelmscream.ogg"));
    soundDie = new KALSound(soundBufferDie, soundEngine);
    soundWilhelmScream = new KALSound(soundBufferWilhelmScream, soundEngine);
    #else
    m_phononPutBomb = Phonon::createPlayer(Phonon::GameCategory);
    m_phononPutBomb->setCurrentSource(KStandardDirs::locate("appdata", "sounds/putbomb.ogg"));
    m_phononExplode = Phonon::createPlayer(Phonon::GameCategory);
    m_phononExplode->setCurrentSource(KStandardDirs::locate("appdata", "sounds/explode.ogg"));
    m_phononBonus = Phonon::createPlayer(Phonon::GameCategory);
    m_phononBonus->setCurrentSource(KStandardDirs::locate("appdata", "sounds/wow.ogg"));
    m_phononDie = Phonon::createPlayer(Phonon::GameCategory);
    m_phononDie->setCurrentSource(KStandardDirs::locate("appdata", "sounds/die.ogg"));
    m_phononWilhelmScream = Phonon::createPlayer(Phonon::GameCategory);
    m_phononWilhelmScream->setCurrentSource(KStandardDirs::locate("appdata", "sounds/wilhelmscream.ogg"));
    #endif
    
    m_arena = 0;
    m_gameScene = 0;
    m_winPoints = Settings::self()->pointsToWin();
    
    int nNumberOfPlayers = Settings::self()->players();
    for (int i = 0; i < nNumberOfPlayers; i++)
    {
        createPlayer(QPointF(-0.5, 0.5), QString("player%1").arg(i+1));
    }
    
    init();
    
    for (int i = 0; i < m_players.size(); i++)
    {
        connect(m_players[i], SIGNAL(bombDropped(Player*, qreal, qreal)), this, SLOT(createBomb(Player*, qreal, qreal)));
        #ifdef GRANATIER_USE_GLUON
        m_players[i]->setSoundDie(soundBufferDie);
        m_players[i]->setSoundWilhelmScream(soundBufferWilhelmScream);
        #endif
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
    delete m_media1;
    delete m_media2;
    
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
    delete m_phononPutBomb;
    delete m_phononExplode;
    delete m_phononBonus;
    delete m_phononDie;
    delete m_phononWilhelmScream;
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

int Game::getScore() const {
	return m_points;
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
    #ifdef GRANATIER_USE_GLUON
    soundBonus->play();
    #else
    m_phononBonus->play();
    #endif
}

void Game::createBlock(QPointF p_position, const QString& p_imageId)
{
    Block* block = new Block(p_position.y(), p_position.x(), m_arena, p_imageId);
    m_blocks.append(block);
    m_arena->setCellElement(p_position.y(), p_position.x(), block);
}

void Game::createPlayer(QPointF p_position, const QString& p_imageId)
{
    Player* player = new Player(qreal(Cell::SIZE * p_position.x()),qreal(Cell::SIZE * p_position.y()), p_imageId, m_arena);
    if(p_imageId.compare("player2") == 0)
    {
        Character::Shortcuts keys;
        keys.moveLeft = Qt::Key_A;
        keys.moveRight = Qt::Key_D;
        keys.moveUp = Qt::Key_W;
        keys.moveDown = Qt::Key_S;
        keys.dropBomb = Qt::Key_Q;
        player->setShortcuts(keys);
    }
    if(p_imageId.compare("player3") == 0)
    {
        Character::Shortcuts keys;
        keys.moveLeft = Qt::Key_J;
        keys.moveRight = Qt::Key_L;
        keys.moveUp = Qt::Key_I;
        keys.moveDown = Qt::Key_K;
        keys.dropBomb = Qt::Key_Space;
        player->setShortcuts(keys);
    }
    if(p_imageId.compare("player4") == 0)
    {
        Character::Shortcuts keys;
        keys.moveLeft = Qt::Key_R;
        keys.moveRight = Qt::Key_Z;
        keys.moveUp = Qt::Key_T;
        keys.moveDown = Qt::Key_G;
        keys.dropBomb = Qt::Key_C;
        player->setShortcuts(keys);
    }
    m_players.append(player);
    connect(player, SIGNAL(dying(Player*)), this, SLOT(playerDeath(Player*)));
}

void Game::setSoundsEnabled(bool p_enabled) {
	if (p_enabled) {
		if (!m_media1) {
			m_media1 = Phonon::createPlayer(Phonon::GameCategory);
		}
		if (!m_media2) {
			m_media2 = Phonon::createPlayer(Phonon::GameCategory);
		}
	} else {
		delete m_media1;
		delete m_media2;
		m_media1 = 0;
		m_media2 = 0;
	}
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

void Game::playSound(const QString& p_sound) {
	Phonon::MediaObject* m_usedMedia;

	if (Settings::sounds()) {
		// Choose the media object with the smallest remaining time
		if (m_media1->remainingTime() <= m_media2->remainingTime()) {
			m_usedMedia = m_media1;
		} else {
			m_usedMedia = m_media2;
		}
		if (m_usedMedia->currentSource().fileName() != p_sound) {
			m_usedMedia->setCurrentSource(p_sound);
		}
		m_usedMedia->play();
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
                        m_strWinner = m_players[i]->getImageId();
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
    //if(player->getImageId() == "player1")
    //{
    //    soundSourceDie->play();
    //}
    //else
    //{
    //    soundSourceWilhelmScream->play();
    //}
    #ifndef GRANATIER_USE_GLUON
    if(player->getImageId() == "player1")
    {
        m_phononWilhelmScream->play();
    }
    else
    {
        m_phononDie->play();
    }
    #endif
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

void Game::winPoints(Element* p_element) {

	// The value of won Points
	long wonPoints;

	wonPoints = p_element->getPoints();

	// Update of the points value
	m_points += wonPoints;

	// For each 10000 points we get a life more
	if (m_points / 10000 > (m_points - wonPoints) / 10000) {
		playSound(KStandardDirs::locate("sound", "kapman/life.ogg"));
		//m_lives++;
		emit(dataChanged(LivesInfo));
	}

	if (p_element->getType() == Element::PILL) {
		playSound(KStandardDirs::locate("sound", "kapman/pill.ogg"));
		emit(elementEaten(p_element->getX(), p_element->getY()));
	} else if (p_element->getType() == Element::BONUS) {
		playSound(KStandardDirs::locate("sound", "kapman/bonus.ogg"));
		// Get the position of the Bonus
		qreal xPos = p_element->getX();
		qreal yPos = p_element->getY();

		// Sends to the scene the number of points to display and its position
		emit(pointsToDisplay(wonPoints, xPos, yPos));
	
		emit(bonusOff());
	}
	emit(dataChanged(ScoreInfo));
}

void Game::nextLevel() {
	playSound(KStandardDirs::locate("sound", "kapman/levelup.ogg"));
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
    #ifdef GRANATIER_USE_GLUON
    soundPutBomb->play();
    #else
    m_phononPutBomb->play();
    #endif
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
    //playSound(KStandardDirs::locate("data", "granatier/sounds/explode.ogg"));
    #ifdef GRANATIER_USE_GLUON
    soundExplode->setMaxGain(10);
    
    if(soundExplode->elapsedTime() == 0)
    {
        soundExplode->setGain(1);
    }
    
    soundExplode->play();
    soundExplode->setGain(soundExplode->gain()*1.2);
    #else
    m_phononExplode->play();
    #endif
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
