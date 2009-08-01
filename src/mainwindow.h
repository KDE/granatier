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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "game.h"
#include "gameview.h"

#include <KXmlGuiWindow>
#include <QGraphicsView>
#include <KScoreDialog>

/**
 * @brief This class enables to create the main window for Kapman.
 */
class MainWindow : public KXmlGuiWindow {

	Q_OBJECT

	private :
		
		/** The GameView instance that manages the game drawing and the collisions */
		GameView* m_view;

		/** The Game instance that manages the main loop and events */
		Game* m_game;

		/** The highscores dialog */
		KScoreDialog* m_kScoreDialog;
		
	public:

		/**
		 * Creates a new MainWindow instance.
		 */
		MainWindow();

		/**
		 * Deletes the MainWindow instance.
		 */
		~MainWindow();

	private slots:

		/**
		 * Initializes the MainWindow for a new game.
		 * Creates a new Game instance and a new GameView instance that displays the game.
		 */
		void initGame();

		/**
		 * Starts a new game.
		 * @param p_gameOver true if the game was over, false if a game is running
		 */
		void newGame(const bool p_gameOver = false);

		/**
		 * Shows the highscores dialog.
		 */
		void showHighscores();

		/**
		 * Shows a dialog enabling to change the current level.
		 */
		void changeLevel();

		/**
		 * Sets the sounds enabled / disabled.
		 * @param p_enabled if true the sounds will be enabled, otherwise they will be disabled
		 */
		void setSoundsEnabled(bool p_enabled);

		/**
		 * Shows the settings dialog.
		 */
		void showSettings();

		/**
		 * Loads the settings.
		 */
		void loadSettings();

		/**
		 * Closes the MainWindow.
		 */
		void close();
};

#endif
