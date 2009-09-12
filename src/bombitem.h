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

#ifndef BOMBITEM_H
#define BOMBITEM_H

#include "elementitem.h"

class Bomb;
class QTimer;

/**
 * @brief This class is the graphical representation of a Bomb.
 */
class BombItem : public ElementItem
{

    Q_OBJECT

protected:

    /** Timer used to make the bomb pulse */
    QTimer* m_pulseTimer;
    
    /** Timer used to animate explosion */
    QTimer* m_explosionTimer;

    /** Number of ticks of the pulse timer */
    int m_numberPulse;
    
    int m_explosionCounter;
    
    int m_x;
    int m_y;

public:

    /**
     * Creates a new BombItem instance.
     * @param p_model the Bomb model
     */
    BombItem(Bomb* p_model);

    /**
     * Deletes the BombItem instance.
     */
    ~BombItem();

    /**
     * Overrides the default shape function to make it a small circle
     * This function is used to determinate collision between items
     * @return QPainterPath the new shape of the Bomb
     */
    QPainterPath shape() const;
    
    /**
     * Pauses the BombItem animation.
     */
    void pauseAnim();
    
    /**
     * Resumes the BombItem animation.
     */
    void resumeAnim();
    
public slots:

    /**
     * Updates the BombItem coordinates.
     * @param p_x the new x-coordinate
     * @param p_y the new y-coordinate
     */
    virtual void update(qreal p_x, qreal p_y);

private slots:
    /**
     * Starts the bomb detonation
     */
    virtual void startDetonation(Bomb* bomb);

    /**
     * Makes the bomb pulse
     */
    virtual void pulse();
    
    /**
    * Animates the explosion
    */
    virtual void explode();

signals:
    void bombItemFinished(BombItem* bombItem);  //explosion finished
};

#endif

