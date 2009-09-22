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

#include <QObject>

#ifndef GRANATIER_USE_GLUON
#include <Phonon>
#endif

class GameScene;
class Arena;
class Player;
class Bonus;
class Bomb;
class Block;
class Element;
class PlayerSettings;
class QPointF;
class QTimer;
class QKeyEvent;

#ifdef GRANATIER_USE_GLUON
class KALEngine;
class KALBuffer;
class KALSound;
#endif

/**
 * @brief This class manages the game main loop : it regularly checks the key press events, computes the character moves and updates their coordinates.
 */
class Game : public QObject {

	Q_OBJECT

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
		
		/** The Arena */
		Arena* m_arena;

        /** The Players */
        QList<Player*> m_players;

        /** The Bombs */
        QList<Bomb*> m_bombs;
        
        /** The Blocks */
        QList<Block*> m_blocks;
        
		/** The Bonuses */
		QList<Bonus*> m_bonus;

		/** A flag to know if the player has cheated during the game */
		bool m_isCheater;

		/** The points which are needed to win */
		int m_winPoints;
		
		/** The won points */
		long m_points;
        
        /** Flag if the round is over */
        int m_roundOver;
        
		/** The current game level */
		int m_level;
        
        #ifdef GRANATIER_USE_GLUON
        /** User KALEngine for sound */
        KALEngine* soundEngine;
        KALSound* soundPutBomb;
        KALSound* soundExplode;
        KALSound* soundBonus;
        KALBuffer* soundBufferDie;
        KALBuffer* soundBufferWilhelmScream;
        KALSound* soundDie;
        KALSound* soundWilhelmScream;
        #else
        /** Use Phonon for sound */
        QTimer* m_phononPutBombTimer;
        QList <Phonon::MediaObject*> m_phononPutBomb;
        QTimer* m_phononExplodeTimer;
        QList <Phonon::MediaObject*> m_phononExplode;
        QTimer* m_phononBonusTimer;
        QList <Phonon::MediaObject*> m_phononBonus;
        QTimer* m_phononDieTimer;
        QList <Phonon::MediaObject*> m_phononDie;
        #endif
        
        bool m_soundEnabled;
        bool m_gameOver;
        bool m_wilhelmScream;
        QString m_strWinner;
        
        GameScene* m_gameScene;
        
        PlayerSettings* m_playerSettings;
		
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
         * @param playerSettings the player settings
		 */
		Game(PlayerSettings* playerSettings);

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
		 * @return the Arena instance
		 */
		Arena* getArena() const;
		
        /**
         * @return the Player models
         */
        QList<Player*> getPlayers() const;
        
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
          * @return the winner of the game
          */
        QString getWinner() const;
        
        /**
          * Create the hidden Bonuses
          */
        void createBonus();
        
        /**
         * @param bonus the Bonus to remove
         */
        void removeBonus(Bonus* bonus);
        
        /**
         * Create a new Block
         * @param p_position the Block position
         * @param p_imageId the image of the Block
         */
        void createBlock(QPointF p_position, const QString& p_imageId);

		/**
		 * Enables / disables the sounds.
		 * @param p_enabled if true the sounds will be enabled, otherwise they will be disabled
		 */
		void setSoundsEnabled(bool p_enabled);
        
        /**
         * remove Block from list and decide to give bonus
         */
        void blockDestroyed(const int row, const int col, Block* block);
        
        /**
         * Removes exploded bombs from the bomb list
         */
        void removeBomb(Bomb* bomb);
        
        /**
         * Sets the games gamescene
         * @param p_gameScene the gamescene
         */
        void setGameScene(GameScene* p_gameScene);
		
	private:
	
        /**
         * Initializes class
         */
        void init();
        /**
         * Cleans class
         */
        void cleanUp();
        
		/**
		 * Initializes the character coordinates.
		 */
		void initCharactersPosition();

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
		 * Resumes the Game after the Player death.
		 */
		void resumeAfterPlayerDeath();
        
        /**
         * Creates a bomb in the Cell with the coordinates x and y
         */
        void createBomb(Player* player, qreal x, qreal y);

	private slots:

		/**
		 * Updates the Game data.
		 */
		void update();
		
		/**
		 * Manages the loss of a life.
		 */
		void playerDeath(Player* player);

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
         * Plays the detonation sound
         */
        void slot_bombDetonated(Bomb* bomb);
		
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
		 * Emitted when a bonus is eaten. It tells to the scene to
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

