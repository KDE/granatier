/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PLAYERITEM_H
#define PLAYERITEM_H

#include "characteritem.h"

class QTimeLine;
class Player;
class BonusItem;
class KGameRenderer;

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
    QTimeLine* m_animationTimer;

    /** Counter for falling animation */
    int m_fallingAnimationCounter;

    /** Counter for resurrection animation */
    int m_resurrectionAnimationCounter;

    /** player ID */
    QString m_strPlayerId;

public:

    /**
      * Creates a new PlayerItem instance.
      * @param p_model the Player model
      * @param renderer the KGameRenderer
      */
    explicit PlayerItem(Player* p_model, KGameRenderer* renderer);

    /**
      * Deletes the PlayerItem instance.
      */
    ~PlayerItem() override;

public Q_SLOTS:

    /**
      * Rotates the image function of the Player direction.
      */
    void updateDirection();

    void updateGraphics(qreal svgScaleFactor) override;

    /**
      * Manages the collisions with any Element.
      */
    void manageCollision();

    /**
      * Updates the PlayerItem coordinates.
      * @param p_x the new x-coordinate
      * @param p_y the new y-coordinate
      */
    void update(qreal p_x, qreal p_y) override;

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
      * the animation when falling in a hole
      */
    void fallingAnimation();

    /**
      * Sets the given frame to the PlayerItem.
      * @param p_frame the frame to set
      */
    void setFrame(const int p_frame);

    /**
      * Implements the CharacterItem method.
      */
    void setDead() override;

    /**
      * resurrects the playeritem
      */
    void resurrect();

Q_SIGNALS:
    void bonusItemTaken(BonusItem* bonusItem);
};

#endif

