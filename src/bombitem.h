/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BOMBITEM_H
#define BOMBITEM_H

#include "elementitem.h"

class Bomb;
class QTimer;
class KGameRenderer;

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

    /** Timing for the explosion */
    QList <int> m_listExplosionTiming;

    /** Counter for the animaton frames */
    int m_animationCounter;

    /** Counter for falling animation */
    int m_fallingAnimationCounter;

    /** Flag to stop the animation if the bomb is fallin in a hole or TODO: a dud bomb */
    bool m_dudBomb;

    int m_x;
    int m_y;

public:

    /**
     * Creates a new BombItem instance.
     * @param p_model the Bomb model
     * @param renderer the KGameRenderer
     */
    BombItem(Bomb* p_model, KGameRenderer* renderer);

    /**
     * Deletes the BombItem instance.
     */
    ~BombItem();

    /**
     * Overrides the default shape function to make it a small circle
     * This function is used to determinate collision between items
     * @return QPainterPath the new shape of the Bomb
     */
    QPainterPath shape() const override;

    /**
     * Pauses the BombItem animation.
     */
    void pauseAnim();

    /**
     * Resumes the BombItem animation.
     */
    void resumeAnim();

public Q_SLOTS:

    /**
     * Updates the BombItem coordinates.
     * @param p_x the new x-coordinate
     * @param p_y the new y-coordinate
     */
    void update(qreal p_x, qreal p_y) override;

private Q_SLOTS:
    /**
     * Starts the bomb detonation
     */
    virtual void startDetonation();

    /**
     * Makes the bomb pulse
     */
    virtual void pulse();

    /**
    * Animates the explosion
    */
    virtual void updateAnimation();

    /**
     * Animates the mortar
     */
    void updateMortarAnimation(int animationCounter, int nMortarRampEnd, int nMortarPeak);

    /**
    * Animates the explosion
    */
    virtual void updateMortar(int nMortarState, int nMortarRampEnd, int nMortarPeak, int nMortarGround);

    /**
    * the animation when falling in a hole
    */
    virtual void fallingAnimation();

Q_SIGNALS:
    /**
     * signals end of the explosion
     * @param bombItem this bombItem
     */
    void bombItemFinished(BombItem* bombItem);  //explosion finished

    /**
     * signals next animation frame
     * @param bombItem this BombItem
     * @param nFrame the next animation frame
     */
    void animationFrameChanged(BombItem* bombItem, int nFrame);
};

#endif

