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

class QTimer;

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

    /** The Bomb detonation power */
    int m_bombPower;

    /** The Bomb ID */
    int m_bombID;

    /** The ID of the Bomb that causes the explosion */
    int m_explosionID;

    /** Timer used to make the bomb detonate */
    QTimer* m_detonationCountdownTimer;

    /** Timer used for the throw bonus and mortar */
    QTimer* m_mortarTimer;

    int m_mortarState;

    bool m_thrown;
    bool m_stopOnCenter;

    bool m_falling;

public:

    /**
    * Creates a new Bomb instance.
    * @param fX the initial x-coordinate
    * @param fY the initial y-coordinate
    * @param p_arena the Arena the Bomb is on
    * @param nBombID the unique bomb ID
    * @param nDetonationCountdown the time until detonation
    */
    Bomb(qreal fX, qreal fY, Arena* p_arena, int nBombID, int nDetonationCountdown);

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
    void move(qreal x, qreal y);

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
    * The direction to throw
    * @param nDirection direction
    */
    void setThrown(int nDirection);

    /**
    * The direction to move
    * @param nDirection direction
    */
    void setKicked(int nDirection);

    /**
    * Power of the bomb
    * @return the Bomb power
    */
    int bombPower();

    /**
    * Sets the Power of the bomb
    * @param bombPower the Bomb power
    */
    void setBombPower(int bombPower);

    /**
    * @return state if the bomb is already detonated
    */
    bool isDetonated();

    /**
    * sets the explosion ID and detonation countdown
    * @param nBombID the Bomb ID that causes the detonation
    * @param nDetonationTimeout the new detonation timeout
    */
    void initDetonation(int nBombID, int nDetonationTimeout);

    /**
    * @return the explosion ID
    */
    int explosionID();

    /**
    * Pauses bomb timer.
    */
    void pause();

    /**
    * Resumes bomb timer.
    */
    void resume();

public Q_SLOTS:
    /**
    * Manages the Bomb explosion
    */
    void detonate();

    void slot_detonationCompleted();

    void updateMortarState();

protected:

    /**
    * Checks the Bomb gets on a Cell center during its next movement.
    * @return true if the Bomb is on a Cell center, false otherwise
    */
    bool onCenter();

    /**
    * Moves the Bomb on the center of its current Cell.
    */
    void moveOnCenter();

Q_SIGNALS:
    /**
    * Emitted when the Bomb is exploded.
    * @param bomb this bomb
    */
    void bombDetonated(Bomb* bomb);

    /**
    * Emitted to refill the player bomb armory
    */
    void releaseBombArmory();

    /**
    * Emitted when the Bomb is thrown by the mortar or by the player.
    * @param nState the current state of the mortar
    */
    void mortar(int nMortarState, int nMortarRampEnd, int nMortarPeak, int nMortarGround);

    /**
    * Emitted when the bomb is falling in a hole
    */
    void falling();
};

#endif

