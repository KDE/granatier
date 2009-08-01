/*
 * Copyright 2008 Mathias Kraus <k.hias@gmx.de>
 * Copyright 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
 * Copyright 2007-2008 Pierre-Benoît Besse <besse.pb@gmail.com>
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

#ifndef MAZEITEM_H
#define MAZEITEM_H

#include <QGraphicsSvgItem>

/**
 * @brief This class is the graphical view of the Maze.
 */
class MazeItem : public QGraphicsSvgItem {
 
 	Q_OBJECT
 	
	public:

		/**
		 * Creates a new MazeItem instance.
		 */
		MazeItem(qreal p_x, qreal p_y);

		/**
		 * Deletes the MazeItem instance.
		 */
		~MazeItem();
};

#endif
