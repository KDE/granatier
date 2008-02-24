/**
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

#ifndef __GAME_H
#define __GAME_H

#include <QTimer>
#include <QKeyEvent>

#include "maze.h"
#include "kapman.h"
#include "ghost.h"

/**
 * This class represents the game manager
 */
class Game : public QObject {

	Q_OBJECT

	private :

		/** Game timer */
		QTimer* m_timer;
		
		/** Energyzers Timer */
		QTimer* m_energyzerTimer;
		
		/** Game Maze */
		Maze* m_maze;

		/** Main character */
		Kapman* m_kapman;
		
		/** Ghosts */
		QList<Ghost*> m_ghostList;

		/** pause flag */
		bool m_isPaused;

		/** Player's lives */
		int m_lives;

		/** Player's points */
		int m_points;

		/** Current level */
		int m_level;
		
	public:

		/**
		 * Creates a new Game instance
		 */
		Game();

		/**
		 * Deletes the Game instance
		 */
		~Game();

		/**
		 * Starts the game
		 */
		void start();

		/**
		 * Pauses the game
		 */
		void pause();
		
		/**
		 * @return the kapman model
		 */
		Kapman* getKapman() const;
		
		/**
		 * @return the list of ghosts models
		 */
		QList<Ghost*> getGhostList () const;
		
		/**
		 * @return the timer
		 */
		QTimer * getTimer() const;
		 
		 /**
		  * @return the maze
		  */
		 Maze * getMaze() const;
		 
		/**
		 * @return m_isPaused attribute
		 */
		bool isPaused() const; 

		/**
		 * Manage the game pause when 'P' key is pressed
		 */
		void doPause();

		/**
		 * @return the score
		*/
		qreal getScore () const;

		/**
		 * @return the lifes
		*/
		int getLives() const;

		/**
		 * @return the current level
		 */
		int getLevel() const;

	private:
	
		/**
		 * Initialize the characters position when the game begin or when the kapman lose a life
		 */
		void initCharactersPosition();
		
		/**
		 * Change all ghost's state to PREY
		 */
		void changeGhostsToPrey();

	public slots:

		/**
		 * Called on key press event
		 * @param p_event the key press event
		 */
		void keyPressEvent(QKeyEvent* p_event);

		/**
		 * Called on each timer signal to update the game data
		 */
		void update();
		
		/**
		 * Manages the loss of a life
		 */
		void kapmanDeath();
		
		/**
		 * Manages the death of a ghost
		 */
		void ghostDeath(Ghost* p_ghost);

		/**
		 * Manages the points won
		 * @param p_elements reference to the eaten element
		 */
		void winPoints(Element* p_element);

		/**
		 * Start the next level
		 */
		void nextLevel();
		
		/**
		 * Makes all ghosts go back to 'hunter' state
		 */
		void changeGhostsToHunter();
		
	signals:
	
		/**
		 * Signals to the scene to display/remove the 'PAUSE' label
		 * @param p_pauseGame indicates if the game is to be paused or not
		 */
		void managePause(bool p_pauseGame);
		
		/**
		 * Signals to the scene to remove the 'INTRO' label
		 */
		void removeIntro();
		
		/**
		 * Signals to the scene to remove a Pill or Energizer
		 * @param p_x x coordinate of the element
		 * @param p_y y coordinate of the element
		 */
		void sKillElement(qreal p_x, qreal p_y);
		
		/**
		 * Signals to the scene to update the score and lives' labels
		 */
		void updatingInfos();

		/**
		 * Signal to the kapmanmainwindow to start a newgame when there isn't more lives or when a level is finished
		 * @param p_gameFinished true if a level was finished, false if a Game Over was reached
		 */
		void startnewgame(bool p_gameFinished);

		/**
		 * Emitted when the level has been finished
		 */
		void leveled();
};

#endif
