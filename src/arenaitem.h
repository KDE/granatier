/*
 * Copyright 2009 Mathias Kraus <k.hias@gmx.de>
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

#ifndef ARENAITEM_H
#define ARENAITEM_H

#include <KGameRenderedItem>

class KGameRenderer;

/**
 * @brief This class is the graphical view of the Arena.
 */
class ArenaItem : public KGameRenderedItem
{
//    Q_OBJECT

public:

    /**
      * Creates a new ArenaItem instances.
      */
    ArenaItem(qreal p_x, qreal p_y, KGameRenderer* renderer, const QString& spriteKey);

    /**
      * Deletes the ArenaItem instances.
      */
    ~ArenaItem();
};

#endif

