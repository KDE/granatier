/*
 * Copyright 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
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

#ifndef GAME_H
#define GAME_H

#include "maze.h"
#include "kapman.h"
#include "ghost.h"
#include "bonus.h"
#include "bomb.h"
#include "block.h"

#include <QPointF>
#include <QTimer>
#include <QKeyEvent>
#include <KGameDifficulty>
#include <Phonon/MediaObject>

#include <KDE/KALEngine>
#include <KDE/KALSource>
#include <KDE/KALBuffer>

/**
 * @brief This class manages the game main loop : it regularly checks the key press events, computes the character moves and updates their coordinates.
 */
class Game : public QObject {

	Q_OBJECT

	public :
		/** Ratio which modify the timers function of the difficulty */
		static qreal s_durationRatio;

		/** Timer duration for prey state in medium difficulty */
		static int s_preyStateDuration;

		/** Timer duration for bonus apparition in medium difficulty */
		static int s_bonusDuration;

	private :

		/** Number of FPS */
		static const int FPS;

		/** The game different states : RUNNING, PAUSED_LOCKED, PAUSED_UNLOCKED */
		enum State {
			RUNNING,			// Game running
			PAUSED_LOCKED,		// Game paused and user is not allowed to unpause
			PAUSED_UNLOCKED		// Game paused and user is allowed to unpause
		};
		/** A flag for the State enum */
		Q_DECLARE_FLAGS(GameStates, State)

		/** The game state */
		State m_state;

		/** The Game main timer */
		QTimer* m_timer;
		
		/** The Bonus timer to make it disappear if it is not eaten after a given time */
		QTimer* m_bonusTimer;

		/** Timer to manage the prey state of the ghosts */
		QTimer* m_preyTimer;
		
		/** The Maze */
		Maze* m_maze;

        /** The Players */
        QList<Kapman*> m_players;
		
		/** The Ghosts */
		QList<Ghost*> m_ghosts;
		
        /** The Bombs */
        QList<Bomb*> m_bombs;
        
        /** The Blocks */
        QList<Block*> m_blocks;
        
		/** The Bonuses */
		QList<Bonus*> m_bonus;

		/** A flag to know if the player has cheated during the game */
		bool m_isCheater;

		/** The remaining number of lives */
		int m_lives;
		
		/** The won points */
		long m_points;

		/** The current game level */
		int m_level;

		/** The number of eaten ghosts since the beginning of the current level */
		int m_nbEatenGhosts;

		/** A first MediaObject to play sounds */
		Phonon::MediaObject* m_media1;
		
		/** A second MediaObject to play sounds */
		Phonon::MediaObject* m_media2;
        
        KALEngine* soundEngine;
        KALSource* soundSourceExplode;
		
	public:

		/** The different types of information about the game */
		enum Information { NoInfo = 0,
				   ScoreInfo = 1,	// Score
				   LivesInfo = 2,	// Number of remaining lives
				   LevelInfo = 4,	// Current level
				   AllInfo = ScoreInfo | LivesInfo | LevelInfo };
		/** A flag for the Information enum */
		Q_DECLARE_FLAGS(InformationTypes, Information)

		/**
		 * Creates a new Game instance.
		 * @param p_difficulty the KGameDifficulty level of the Game
		 */
		Game(KGameDifficulty::standardLevel p_difficulty = KGameDifficulty::Medium);

		/**
		 * Deletes the Game instance.
		 */
		~Game();

		/**
		 * Starts the Game.
		 */
		void start();

		/**
		 * Pauses the Game.
		 * @param p_locked if true the player will be unable to unset the pause.
		 */
		void pause(bool p_locked = false);

		/**
		 * Pauses / unpauses the game.
		 * @param p_locked if true the player will be unable to unset the pause.
		 */
		void switchPause(bool p_locked = false);

		/**
		 * @return the Maze instance
		 */
		Maze* getMaze() const;
		
        /**
         * @return the Player models
         */
        QList<Kapman*> getPlayers() const;
        
		/**
		 * @return the Ghost models
		 */
		QList<Ghost*> getGhosts () const; 
		
        /**
         * @return the Bonus instance
         */
        QList<Bonus*> getBonus();

		/**
		 * @return the main timer
		 */
		QTimer* getTimer() const;
		 
		/**
		 * @return true if the Game is paused, false otherwise
		 */
		bool isPaused() const;
		
		/**
		 * @return true if the player has cheated during the game, false otherwise
		 */
		bool isCheater() const;

		/**
		 * @return the score
		 */
		int getScore () const;

		/**
		 * @return the number of remaining lives
		 */
		int getLives() const;

		/**
		 * @return the current level
		 */
		int getLevel() const;

		/**
		 * Sets the level to the given number.
		 * @param p_level the new level
		 */
		void setLevel(int p_level);

        /**
          * Create the hidden Bonuses
          */
        void createBonus();
        
        /**
         * Create a new Block
         * @param p_position the Block position
         * @param p_imageId the image of the Block
         */
        void createBlock(QPointF p_position, const QString& p_imageId);

        /**
         * Create the new Player
         * @param p_position the Player position
         * @param p_imageId the image of the Player
         */
        void createPlayer(QPointF p_position, const QString& p_imageId);

		/**
		 * Create the new Ghost
		 * @param p_position the Ghost position
		 * @param p_imageId the image of the Ghost
		 */
		void createGhost(QPointF p_position, const QString & p_imageId);

		/**
		 * Initializes a Maze
		 * @param p_nbRows the number of rows
		 * @param p_nbColumns the number of columns
		 */
		void initMaze(const int p_nbRows, const int p_nbColumns);

		/**
		 * Initializes a Ghost
		 */
		void initGhost();

		/**
		 * Initializes a Kapman
		 */
		void initKapman();

		/**
		 * Enables / disables the sounds.
		 * @param p_enabled if true the sounds will be enabled, otherwise they will be disabled
		 */
		void setSoundsEnabled(bool p_enabled);
        
        /**
         * remove Block from list and decide to give bonus
         */
        void blockDestroyed(const int row, const int col, Block* block);
		
	private:
	
		/**
		 * Initializes the character coordinates.
		 */
		void initCharactersPosition();

		/**
		 * Calculates and update the ghosts speed depending on the ghosts speed
		 * The value is in Ghost::s_speed
		 */
		void setTimersDuration();

		/**
		 * Plays the given sound.
		 * @param p_sound the path to the sound to play
		 */
		void playSound(const QString& p_sound);

	public slots:

		/**
		 * Manages the key press events.
		 * @param p_event the key press event
		 */
		void keyPressEvent(QKeyEvent* p_event);
		
        /**
         * Manages the key release events.
         * @param p_event the key release event
         */
        void keyReleaseEvent(QKeyEvent* p_event);
        
		/**
		 * Resumes the Game after the Kapman death.
		 */
		void resumeAfterKapmanDeath();
        
        /**
         * Creates a bomb in the Cell with the coordinates x and y
         */
        void createBomb(qreal x, qreal y);

	private slots:

		/**
		 * Updates the Game data.
		 */
		void update();
		
		/**
		 * Manages the loss of a life.
		 */
		void kapmanDeath();

		/**
		 * Manages the death of a Ghost.
		 */
		void ghostDeath(Ghost* p_ghost);

		/**
		 * Increases the score considering the eaten Element.
		 * @param p_element the eaten Element
		 */
		void winPoints(Element* p_element);

		/**
		 * Starts the next level.
		 */
		void nextLevel();
		
		/**
		 * Hides the Bonus.
		 */
		void hideBonus();

		/**
		 * Ends the Ghosts prey state.
		 */
		void endPreyState();	
        
        /**
         * Plays the detonation sound
         */
        void slot_bombDetonated(Bomb* bomb);
        
        /**
         * Removes exploded bombs from the bomb list
         */
        void slot_cleanupBombs(Bomb* bomb);
		
	signals:
	
		/**
		 * Emitted when the Game is started.
		 */
		void gameStarted();
		
		/**
		 * Emitted when the Game is over.
		 * @param p_unused this parameter must always be true !
		 */
		void gameOver(const bool p_unused);

		/**
		 * Emitted when a level begins, if level up or if a life has been lost.
		 * @param p_newLevel true if a new level is beginning, false otherwise
		 */
		void levelStarted(const bool p_newLevel);
		
		/**
		 * Emitted when the pause state has changed.
		 * @param p_pause true if the Game is paused, false otherwise
		 * @param p_fromUser true if the Game has been paused due to an action the player has done, false otherwise
		 */
		void pauseChanged(const bool p_pause, const bool p_fromUser);
		
		/**
		 * Emitted when an Element has been eaten.
		 * @param p_x the Element x-coordinate
		 * @param p_y the Element y-coordinate
		 */
		void elementEaten(const qreal p_x, const qreal p_y);
		
		/**
		 * Emitted when the Bonus has to be displayed.
		 */
		void bonusOn();

		/**
		 * Emitted when the Bonus has to disappear.
		 */
		void bonusOff();
		
		/**
		 * Emitted when the Game data (score, level, lives) have changed.
		 * @param p_infoType the type of data that have changed
		 */
		void dataChanged(Game::InformationTypes p_infoType);

		/**
		 * Emitted when a ghost or a bonus is eaten. It tells to the scene to
		 * display the number of won points
		 * @param p_wonPoints the value to display
		 * @param p_xPos the x position of the label
		 * @param p_yPos the y position of the label
		 */
		void pointsToDisplay(long p_wonPoints, qreal p_xPos, qreal p_yPos);
        
        /**
         * Emitted when a bomb was created.
         */
        void bombCreated(Bomb* bomb);
        
        /**
         * Emitted when a bomb was removed.
         */
        void bombRemoved(Bomb* bomb);
};

#endif

