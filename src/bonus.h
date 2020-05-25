/*
 * Copyright 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
 * Copyright 2007-2008 Gaël Courcelle <gael.courcelle@gmail.com>
 * Copyright 2007-2008 Alexia Allanic <alexia_allanic@yahoo.fr>
 * Copyright 2007-2008 Johann Hingue <yoan1703@hotmail.fr>
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

#ifndef BONUS_H
#define BONUS_H

#include "element.h"

/**
 * @brief This class represents a Bonus for the Player.
 */
class Bonus : public Element {
    Q_OBJECT
public:
    /**
      * Creates a new Bonus instance.
      * @param p_x the initial x-coordinate
      * @param p_y the initial y-coordinate
      * @param p_arena a reference to the Arena the Bonus will be on
      * @param bonusType the Bonus Type
      */
    Bonus(qreal p_x, qreal p_y, Arena* p_arena, Granatier::Bonus::Type bonusType);

    /**
      * Deletes the Bonus instance.
      */
    ~Bonus();

    /**
      * Computes an action on a collision with the Player.
      * @param p_player the Player instance that collides with the Bonus
      */
    void doActionOnCollision(Player* p_player) override;

    Granatier::Bonus::Type getBonusType() const;

    void setTaken();

    bool isTaken() const;

    void setDestroyed();

    bool isDestroyed() const;

private:
    /** The Bonus type */
    Granatier::Bonus::Type m_bonusType;

    bool m_taken;
    bool m_destroyed;
};

#endif

