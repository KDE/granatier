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

#ifndef PLAYERITEM_H
#define PLAYERITEM_H

#include "characteritem.h"

class QTimeLine;
class Player;

/**
 * @brief This class manage the display of the Player.
 */
class PlayerItem : public CharacterItem
{
  Q_OBJECT

private:

    /** Number of frames to animate the PlayerItem */
    static const int NB_FRAMES;

    /** Animation update interval */
    static const int ANIM_SPEED;

    /** Timer used to animate the PlayerItem */
    QTimeLine * m_animationTimer;
    
    /** player ID */
    QString m_strPlayerId;

public:

    /**
      * Creates a new PlayerItem instance.
      * @param p_model the Player model
      */
    PlayerItem(Player* p_model);

    /**
      * Deletes the PlayerItem instance.
      */
    ~PlayerItem();
    
    /**
      * Player ID to get the element ID from the SVG
      */
    void setPlayerId(QString strPlayerId);

public slots:

    /**
      * Rotates the image function of the Player direction.
      */
    void updateDirection();

    /**
      * Manages the collisions with any Element.
      */
    void manageCollision();
    
    /**
      * Updates the PlayerItem coordinates.
      * @param p_x the new x-coordinate
      * @param p_y the new y-coordinate
      */
    void update(qreal p_x, qreal p_y);

    /**
      * Starts the PlayerItem animation.
      */
    void startAnim();

    /**
      * Pauses the PlayerItem animation.
      */
    void pauseAnim();

    /**
      * Resumes the PlayerItem animation.
      */
    void resumeAnim();

    /**
      * Stops the PlayerItem animation.
      */
    void stopAnim();

    /**
      * Sets the given frame to the PlayerItem.
      * @param p_frame the frame to set
      */
    void setFrame(const int p_frame);

    /**
      * Implements the CharacterItem method.
      */
    void setDead();
        
signals:
    void bonusItemTaken(ElementItem* bonusItem);
};

#endif

