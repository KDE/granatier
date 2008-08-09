/*
 * Copyright 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
 * Copyright 2007-2008 Gaël Courcelle <gael.courcelle@gmail.com>
 * Copyright 2007-2008 Alexia Allanic <alexia_allanic@yahoo.fr>
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

#ifndef __PILL_H
#define __PILL_H

#include "element.h"

/**
 * @brief This class represents a Pill enabling to earn points.
 */
class Pill : public Element {

	public:

		/** The Pill value */
		static const int POINTS;

	public:

		/**
		 * Creates a new Pill instance.
		 */
		Pill(qreal p_x, qreal p_y, Maze* p_maze, const QString& p_imageUrl);

		/**
		 * Deletes the Pill instance.
		 */
		~Pill();

		/**
		 * Computes an action on a collision with the Kapman.
		 * @param p_kapman the instance of Kapman which collides with the Pill
		 */
		void doActionOnCollision(Kapman* p_kapman);
};

#endif

