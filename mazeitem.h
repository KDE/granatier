/**
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

#ifndef __MAZEITEM_H
#define __MAZEITEM_H

#include <QGraphicsSvgItem>

/**
 * This class represents the view of the maze
 */
class MazeItem : public QGraphicsSvgItem {
 
 	Q_OBJECT
 	
	public:

		/**
		 * Creates a new MazeItem
		 * @param p_imagePath the path to the maze image
		 */
		MazeItem(QString & p_imagePath);

		/**
		 * Deletes the MazeItem
		 */
		~MazeItem();
};

#endif
