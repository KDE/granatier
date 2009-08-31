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

#ifndef BOMB_H
#define BOMB_H

#include "element.h"

#include <QTimer>

/**
 * @brief This class describes the common characteristics and behaviour of the bomb item.
 */
class Bomb : public Element
{

    Q_OBJECT

protected:
    
    /** The Bomb x-speed */
    qreal m_xSpeed;

    /** The Bomb y-speed */
    qreal m_ySpeed;

    /** The Bomb speed */
    qreal m_speed;
    
    /** The Bomb detonation has already started */
    bool m_detonated;

    /** The Bomb detonation range */
    int m_bombRange;
    
    /** Timer used to make the bomb detonate */
    QTimer* m_detonationCountdownTimer;

public:

    /**
    * Creates a new Bomb instance.
    * @param fX the initial x-coordinate
    * @param fY the initial y-coordinate
    * @param p_arena the Arena the Bomb is on
    * @param nDetonationCountdown the time until detonation
    */
    Bomb(qreal fX, qreal fY, Arena* p_arena, int nDetonationCountdown);

    /**
    * Deletes the Bomb instance.
    */
    ~Bomb();

    /**
    * Makes the Bomb go up.
    */
    void goUp();

    /**
    * Makes the Bomb go down.
    */
    void goDown();

    /**
    * Makes the Bomb go to the right.
    */
    void goRight();

    /**
    * Makes the Bomb go to the left.
    */
    void goLeft();

    /**
    * Updates the Bomb move.
    */
    void updateMove();

    /**
    * Moves the Bomb function of its current coordinates and speed.
    * If the Bomb reaches a border, it circles around the arena and continue its way from the other side.
    */
    void move();

    /**
    * Gets the Bomb x-speed value.
    * @return the x-speed value
    */
    qreal getXSpeed() const;

    /**
    * Gets the Bomb y-speed value.
    * @return the y-speed value
    */
    qreal getYSpeed() const;

    /**
    * Gets the Bomb speed.
    * @return the Bomb speed
    */
    qreal getSpeed();
    
    /**
    * Set the Bomb x-speed value.
    * @param p_xSpeed the x-speed to set
    */
    void setXSpeed(qreal p_xSpeed);
        
    /**
    * Set the Bomb y-speed value.
    * @param p_ySpeed the y-speed to set
    */
    void setYSpeed(qreal p_ySpeed);
    
    /**
    * Range of the bomb
    * @return the Bomb range
    */
    int bombRange();
    
    /**
    * Sets the Range of the bomb
    * @param range the Bomb range
    */
    void setBombRange(int bombRange);
    
    /**
    * returns if the bomb is already detonated
    */
    bool isDetonated();
    
    /**
    * sets the detonation countdown
    */
    void setDetonationCountdown(int nDetonationTimeout);
    
    /**
    * Pauses bomb timer.
    */
    void pause();
    
    /**
    * Resumes bomb timer.
    */
    void resume();

public slots:
    /**
    * Manages the Bomb explosion
    */
    void detonate();
    
    void slot_detonationCompleted();

protected:

    /**
    * Gets the next Cell the Bomb is going to reach.
    * @return the next Cell the Bomb is going to reach
    */
    Cell getNextCell();

    /**
    * Checks the Bomb gets on a Cell center during its next movement.
    * @return true if the Bomb is on a Cell center, false otherwise
    */
    bool onCenter();

    /**
    * Moves the Bomb on the center of its current Cell.
    */
    void moveOnCenter();

signals:
    /**
    * Emitted when the Bomb is exploded.
    */
    void bombDetonated(Bomb* bomb);
};

#endif

