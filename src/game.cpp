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

#include <KStandardDirs>

const int Game::FPS = 40;
int Game::s_bonusDuration;
int Game::s_preyStateDuration;
qreal Game::s_durationRatio;

Game::Game(KGameDifficulty::standardLevel p_difficulty) : m_isCheater(false), m_lives(3), m_points(0), m_level(1), m_nbEatenGhosts(0), m_media1(0), m_media2(0) {
	// Initialize the sound state
	setSoundsEnabled(Settings::sounds());
	// Initialize the game difficulty
    	Settings::setGameDifficulty((int) p_difficulty);
	Settings::self()->writeConfig();

	// Timers for medium difficulty
	s_bonusDuration = 7000;
	s_preyStateDuration = 10000;
	// Difference ratio between low/high and medium speed
	s_durationRatio = 1.0;

	// Tells the KGameDifficulty singleton that the game is not running
	KGameDifficulty::setRunning(false);

	// Create the Maze instance
	m_maze = new Maze();
	connect(m_maze, SIGNAL(allElementsEaten()), this, SLOT(nextLevel()));

	// Create the parser that will parse the XML file in order to initialize the Maze instance
	// This also creates all the characters
	MapParser mapParser(this);
	// Set the XML file as input source for the parser
	QFile mazeXmlFile(KStandardDirs::locate("appdata", "defaultmaze.xml"));
	QXmlInputSource source(&mazeXmlFile);
	// Create the XML file reader
	QXmlSimpleReader reader;
	reader.setContentHandler(&mapParser);
	// Parse the XML file
	reader.parse(source);

	//connect(m_kapman, SIGNAL(sWinPoints(Element*)), this, SLOT(winPoints(Element*)));

	

	// Initialize the characters speed timers duration considering the difficulty level
	switch (p_difficulty) {
		case KGameDifficulty::Easy:
			// Ratio low/medium speed
			s_durationRatio = Character::MEDIUM_SPEED / Character::LOW_SPEED;
			break;
		case KGameDifficulty::Medium:
			s_durationRatio = 1;
			break;
		case KGameDifficulty::Hard:
			// Ratio high/medium speed
			s_durationRatio = Character::MEDIUM_SPEED / Character::HIGH_SPEED;
			break;
		default:
			break;
	}	

	for (int i = 0; i < m_ghosts.size(); ++i) {
		connect(m_ghosts[i], SIGNAL(lifeLost()), this, SLOT(kapmanDeath()));
		connect(m_ghosts[i], SIGNAL(ghostEaten(Ghost*)), this, SLOT(ghostDeath(Ghost*)));
		// Initialize the ghosts speed and the ghost speed increase considering the characters speed
		m_ghosts[i]->initSpeedInc();
	}
    
    for (int i = 0; i < m_players.size(); i++)
    {
        connect(m_players[i], SIGNAL(bombDropped(qreal, qreal)), this, SLOT(createBomb(qreal, qreal)));
        m_players[i]->initSpeedInc();
    }

	// Initialize Bonus timer from the difficulty level
	m_bonusTimer = new QTimer(this);
	m_bonusTimer->setInterval( (int)(s_bonusDuration * s_durationRatio) );
	m_bonusTimer->setSingleShot(true);
	connect(m_bonusTimer, SIGNAL(timeout()), this, SLOT(hideBonus()));
	// Initialize the Preys timer from the difficulty level
	m_preyTimer = new QTimer(this);
	m_preyTimer->setInterval( (int)(s_preyStateDuration * s_durationRatio) );
	m_preyTimer->setSingleShot(true);
	connect(m_preyTimer, SIGNAL(timeout()), this, SLOT(endPreyState()));

	// Start the Game timer
	m_timer = new QTimer(this);
	m_timer->setInterval(int(1000 / Game::FPS));
	connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
	m_timer->start();
	m_state = RUNNING;
	// Init the characters coordinates on the Maze
	initCharactersPosition();
    
    
    //init KALEngine
    soundEngine = KALEngine::getInstance();
    soundSourceExplode = new KALSource(KALBuffer::fromOgg(KStandardDirs::locate("data", "granatier/sounds/explode.ogg")), soundEngine);
}

Game::~Game() {
	delete m_media1;
	delete m_media2;
	delete m_timer;
	delete m_bonusTimer;
	delete m_maze;
    
    for (int i = 0; i < m_ghosts.size(); i++)
    {
        delete m_players[i];
    }
    
	for (int i = 0; i < m_ghosts.size(); ++i) {
		delete m_ghosts[i];
	}
    
    for (int i = 0; i < m_bombs.size(); ++i) {
        delete m_bombs[i];
    }
	delete m_bonus;
    
    delete soundSourceExplode;
    KALEngine::kill();
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

QList<Kapman*> Game::getPlayers() const 
{
    return m_players;
}

		
QList<Ghost*> Game::getGhosts () const {
	return m_ghosts;
}

QTimer* Game::getTimer() const {
	return m_timer;
}

Maze* Game::getMaze() const {
	return m_maze;
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
	return m_lives;
}

int Game::getLevel() const {
	return m_level;
}

void Game::setLevel(int p_level) {
	m_isCheater = true;
	m_level = p_level;
	m_maze->resetNbElem();
	m_timer->start();	// Needed to reinit character positions
	initCharactersPosition();
	for (int i = 0; i < m_ghosts.size(); ++i) {
		m_ghosts[i]->initSpeed();
	}
    
    for (int i = 0; i < m_players.size(); i++)
    {
        m_players[i]->initSpeed();
    }
    for (int i = 0; i < m_level; ++i) {
        for (int j = 0; j < m_ghosts.size(); ++j) {
            m_ghosts[j]->increaseCharactersSpeed();
        }
        
        for (int j = 0; j < m_players.size(); j++)
        {
            m_players[j]->increaseCharactersSpeed();
        }
    }
	setTimersDuration();
	m_bonus->setPoints(m_level * 100);
	emit(dataChanged(AllInfo));
	emit(pauseChanged(false, true));
	emit(levelStarted(true));
}

Bonus* Game::getBonus() {
	return m_bonus;
}

void Game::createBonus(QPointF p_position){
	m_bonus = new Bonus(qreal(Cell::SIZE * p_position.x()),qreal(Cell::SIZE * p_position.y()), m_maze, 100);
}

void Game::createPlayer(QPointF p_position, const QString& p_imageId)
{
    Kapman* player = new Kapman(qreal(Cell::SIZE * p_position.x()),qreal(Cell::SIZE * p_position.y()), p_imageId, m_maze);
    if(p_imageId.compare("player2") == 0)
    {
        Character::Shortcuts keys;
        keys.moveLeft = Qt::Key_A;
        keys.moveRight = Qt::Key_D;
        keys.moveUp = Qt::Key_W;
        keys.moveDown = Qt::Key_S;
        keys.dropBomb = Qt::Key_Space;
        player->setShortcuts(keys);
    }
    m_players.append(player);
}

void Game::createGhost(QPointF p_position, const QString & p_imageId){
	//m_ghosts.append(new Ghost(qreal(Cell::SIZE * p_position.x()),qreal(Cell::SIZE * p_position.y()), p_imageId, m_maze));
}


void Game::initMaze(const int p_nbRows, const int p_nbColumns){
	m_maze->init(p_nbRows, p_nbColumns);
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
		// Initialize Ghost coordinates and state
		//m_ghosts[0]->initCoordinate();
		//m_ghosts[1]->initCoordinate();
		//m_ghosts[2]->initCoordinate();
		//m_ghosts[3]->initCoordinate();
		//m_ghosts[0]->setState(Ghost::HUNTER);
		//m_ghosts[1]->setState(Ghost::HUNTER);
		//m_ghosts[2]->setState(Ghost::HUNTER);
		//m_ghosts[3]->setState(Ghost::HUNTER);
        for(int i = 0; i < m_players.size(); i++)
        {
            m_players[i]->initCoordinate();
            m_players[i]->init();
        }
        // Initialize the Block coordinates
        for (int i = 0; i < m_maze->getNbRows(); ++i)
        {
            for (int j = 0; j < m_maze->getNbColumns(); ++j)
            {
                if (m_maze->getCell(i,j).getElement() != NULL && m_maze->getCell(i,j).getElement()->getType() == Element::BLOCK)
                {
                    m_maze->getCell(i,j).getElement()->setX(Cell::SIZE * (j + 0.5));
                    m_maze->getCell(i,j).getElement()->setY(Cell::SIZE * (i + 0.5));
                }
            }
        }
    }
}

void Game::setTimersDuration() {
	// Updates the timers duration ratio with the ghosts speed
	s_durationRatio = Character::MEDIUM_SPEED / m_ghosts[0]->getNormalSpeed();

	// Updates the timers duration
	m_bonusTimer->setInterval( (int)(s_bonusDuration * s_durationRatio) );
	m_preyTimer->setInterval( (int)(s_preyStateDuration * s_durationRatio) );
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

void Game::keyPressEvent(QKeyEvent* p_event) {
	// At the beginning or when paused, we start the timer when a key is pressed
	if (!m_timer->isActive()) {
		// If paused
		if (m_state == PAUSED_UNLOCKED) {
			switchPause();
		} else if (m_state == RUNNING) {	// At the game beginning
			// Start the game
			m_timer->start();
			emit(gameStarted());
		}
		// Tells the KGameDifficulty singleton that the game now runs
		KGameDifficulty::setRunning(true);
	}
	// Behaviour when the game has begun
	switch (p_event->key()) {
		case Qt::Key_P:
		case Qt::Key_Escape:
			switchPause();
            return;
		case Qt::Key_K:
			// Cheat code to get one more life
			if (p_event->modifiers() == (Qt::AltModifier | Qt::ControlModifier | Qt::ShiftModifier)) {
				m_lives++;
				m_isCheater = true;
				emit(dataChanged(LivesInfo));
			}
            return;
		case Qt::Key_L:
			// Cheat code to go to the next level
			if (p_event->modifiers() == (Qt::AltModifier | Qt::ControlModifier | Qt::ShiftModifier)) {
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

void Game::update() {
	int curKapmanRow, curKapmanCol;
	
	// Check if the kapman is in the line of sight of a ghost
	//curKapmanRow = m_maze->getRowFromY(m_kapman->getY());
	//curKapmanCol = m_maze->getColFromX(m_kapman->getX());
	
	for (int i = 0; i < m_ghosts.size(); ++i) {
		//if (m_ghosts[i]->getState() == Ghost::HUNTER && m_ghosts[i]->isInLineSight(m_kapman)) {
		//	m_ghosts[i]->updateMove(curKapmanRow, curKapmanCol);
		//}
		//else {
			m_ghosts[i]->updateMove();
		//}
	}
    
    //update Bombs
    for (int i = 0; i < m_bombs.size(); ++i) {
        m_bombs[i]->updateMove();
    }
    
    //update Player
    for(int i = 0; i < m_players.size(); i++)
    {
        m_players[i]->updateMove();
        m_players[i]->emitGameUpdated();
    }
}

void Game::kapmanDeath() {
	playSound(KStandardDirs::locate("sound", "kapman/gameover.ogg"));
	m_lives--;
	//m_kapman->die();
	// Make a 2 seconds pause while the kapman is blinking
	pause(true);
	QTimer::singleShot(2500, this, SLOT(resumeAfterKapmanDeath()));
}

void Game::resumeAfterKapmanDeath() {
	emit(dataChanged(LivesInfo));
	// Start the timer
	start();
	// Remove a possible bonus
	emit(bonusOff());
	// If their is no lives left, we start a new game
	if (m_lives <= 0) {
		emit(gameOver(true));
	} else {
		emit(levelStarted(false));
		// Move all characters to their initial positions
		initCharactersPosition();
	}
}

void Game::ghostDeath(Ghost* p_ghost) {
	m_nbEatenGhosts++;
	p_ghost->setState(Ghost::EATEN);
	winPoints(p_ghost);
}

void Game::winPoints(Element* p_element) {

	// The value of won Points
	long wonPoints;

	// If the eaten element is a ghost, win 200 * number of eaten ghosts since the energizer was eaten
	if (p_element->getType() == Element::GHOST) {
		playSound(KStandardDirs::locate("sound", "kapman/ghost.ogg"));
		// Get the position of the ghost
		qreal xPos = p_element->getX();
		qreal yPos = p_element->getY();
		// Add points to the score
		wonPoints = p_element->getPoints() * m_nbEatenGhosts;
		// Send to the scene the number of points to display and its position 
		emit(pointsToDisplay(wonPoints, xPos, yPos));
	}
	// Else you just win the value of the element
	else {
		wonPoints = p_element->getPoints();
	}

	// Update of the points value
	m_points += wonPoints;

	// For each 10000 points we get a life more
	if (m_points / 10000 > (m_points - wonPoints) / 10000) {
		playSound(KStandardDirs::locate("sound", "kapman/life.ogg"));
		m_lives++;
		emit(dataChanged(LivesInfo));
	}
	// If the eaten element is an energyzer we change the ghosts state
	if (p_element->getType() == Element::ENERGYZER) {
		// We start the prey timer
		m_preyTimer->start();
		playSound(KStandardDirs::locate("sound", "kapman/energizer.ogg"));
		for (int i = 0; i < m_ghosts.size(); ++i) {
			if(m_ghosts[i]->getState() != Ghost::EATEN) {
				m_ghosts[i]->setState(Ghost::PREY);
			}
		}
		// Reset the number of eaten ghosts
		m_nbEatenGhosts = 0;
		emit(elementEaten(p_element->getX(), p_element->getY()));
	} else if (p_element->getType() == Element::PILL) {
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
	// If 1/3 or 2/3 of the pills are eaten
	if (m_maze->getNbElem() == m_maze->getTotalNbElem() / 3 || m_maze->getNbElem() == (m_maze->getTotalNbElem() * 2 / 3)) {
		// Display the Bonus
		emit(bonusOn());
		m_bonusTimer->start();
	}	
	emit(dataChanged(ScoreInfo));
}

void Game::nextLevel() {
	playSound(KStandardDirs::locate("sound", "kapman/levelup.ogg"));
	// Increment the level
	m_level++;
	// Initialize the maze items
	m_maze->resetNbElem();
	// Update Bonus
	m_bonus->setPoints(m_level * 100);
	// Move all characters to their initial positions
	initCharactersPosition();
	// Increase the ghosts speed
	for (int i = 0; i < m_ghosts.size(); ++i) {
		// Increase the ghosts speed increase
		m_ghosts[i]->increaseCharactersSpeed();
	}
	//m_kapman->increaseCharactersSpeed();
	// Update the timers duration with the new speed
	setTimersDuration();
	// Update the score, level and lives labels
	emit(dataChanged(AllInfo));
	// Update the view
	emit(levelStarted(true));
}

void Game::hideBonus() {
	emit(bonusOff());
}

void Game::endPreyState() {
	for (int i = 0; i < m_ghosts.size(); ++i) {
		if(m_ghosts[i]->getState() != Ghost::EATEN) {
			m_ghosts[i]->setState(Ghost::HUNTER);
		}
	}
}

void Game::createBomb(qreal x, qreal y)
{
    int col = m_maze->getColFromX(x);
    int row = m_maze->getRowFromY(y);
    if(col >= 0 && col < m_maze->getNbColumns() && row >= 0 && row < m_maze->getNbRows())
    {
        if(m_maze->getCell(row, col).getElement() != NULL && m_maze->getCell(row, col).getElement()->getType() == Element::BOMB)
        {
            return;
        }
    }
    Bomb* bomb = new Bomb((col + 0.5) * Cell::SIZE, (row + 0.5) * Cell::SIZE, m_maze, Game::FPS * 2);
    bomb->setBombRange(3);
    emit bombCreated(bomb);
    connect(bomb, SIGNAL(bombFinished(Bomb*)), this, SLOT(slot_cleanupBombs(Bomb*)));
    connect(bomb, SIGNAL(bombDetonated(Bomb*)), this, SLOT(slot_bombDetonated(Bomb*)));
    m_bombs.append(bomb);
}

void Game::slot_cleanupBombs(Bomb* bomb)
{
    // Find the Bomb
    int index = m_bombs.indexOf(bomb);
    //remove the bomb
    if(index != -1)
    {
        //do not delete the bomb because it will be deleted through the destructor of elementitem
        emit bombRemoved(m_bombs.at(index));
        m_bombs.removeAt(index);
    }
}

void Game::slot_bombDetonated(Bomb* bomb)
{
    //playSound(KStandardDirs::locate("data", "granatier/sounds/explode.ogg"));
    soundSourceExplode->play();
}

void Game::blockDestroyed(const int row, const int col, Block* block)
{
    m_maze->removeCellElement(row, col, block);
    //do not delete the block because it will be deleted through the destructor of elementitem 
}
