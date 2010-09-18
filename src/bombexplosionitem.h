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

#include <KGameRenderedItem>

class Bomb;
class BombItem;
class KGameRenderer;

/**
 * @brief This class is the graphical representation of a Bomb explosion.
 */
class BombExplosionItem : public QObject, public KGameRenderedItem
{

Q_OBJECT

public:
    /** The Explosion directions */
    enum Direction
    {
        NORTH,
        EAST,
        SOUTH,
        WEST
    };

protected:

    /** The direction of the blast */
    Direction m_direction;
    
    /** The bomb power */
    int m_bombPower;
   
    /** The ID of the Bomb that causes the explosion */
    int m_explosionID;
    
    /** The scale factor from the svg for the pixmap */
    qreal m_svgScaleFactor;

public:

    /**
     * Creates a new BombExplosionItem instance.
     * @param p_model the Bomb model
     * @param direction the dirction of the explosion
     * @param bombPower the power of the bomb at the position of this BombExplosionItem
     * @param renderer the KGameRenderer
     * @param svgScaleFactor the scale factor of the pixmap
     */
    BombExplosionItem(Bomb* p_model, BombExplosionItem::Direction direction, int bombPower, KGameRenderer* renderer, qreal svgScaleFactor);

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
    
    /**
    * @return the explosion ID
    */
    int explosionID();

    /**
     * Updates the BombExplosionItem coordinates.
     * @param p_x the new x-coordinate
     * @param p_y the new y-coordinate
     */
    virtual void setPosition(qreal p_x, qreal p_y);

    /**
     * updates the animation
     * @param nFrame the next animation frame
     */
     void updateAnimationn(int nFrame);
     
public slots:
    
    /**
     * Updates the graphics after a resize
     * @param svgScaleFactor the scaling factor between svg and rendered pixmap
     */
    virtual void updateGraphics(qreal svgScaleFactor);
};

#endif

