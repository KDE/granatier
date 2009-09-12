/*
 * Copyright 2009 Mathias Kraus <k.hias@gmx.de>
 * Copyright 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
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

#ifndef BOMBEXPLOSIONITEM_H
#define BOMBEXPLOSIONITEM_H

#include <QGraphicsSvgItem>

class Bomb;
class QTimer;

/**
 * @brief This class is the graphical representation of a Bomb explosion.
 */
class BombExplosionItem : public QGraphicsSvgItem
{

    Q_OBJECT

public:
    /** The Explosion directions */
    enum Direction {
        NORTH,
        EAST,
        SOUTH,
        WEST
    };

protected:

   Direction m_direction;

public:

    /**
     * Creates a new BombExplosionItem instance.
     * @param p_model the Bomb model
     */
    BombExplosionItem(Bomb* p_model, Direction direction, int i);

    /**
     * Deletes the BombExplosionItem instance.
     */
    ~BombExplosionItem();

    /**
     * Overrides the default shape function to make it a small circle
     * This function is used to determinate collision between items
     * @return QPainterPath the new shape of the Bomb
     */
    QPainterPath shape() const;
    
public slots:

    /**
     * Updates the BombExplosionItem coordinates.
     * @param p_x the new x-coordinate
     * @param p_y the new y-coordinate
     */
    virtual void update(qreal p_x, qreal p_y);
};

#endif

