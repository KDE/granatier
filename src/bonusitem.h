/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BONUSITEM_H
#define BONUSITEM_H

#include "elementitem.h"

class Bonus;
class QTimer;
class KGameRenderer;

/**
 * @brief This class is the graphical representation of a Bonus.
 */
class BonusItem : public ElementItem
{

    Q_OBJECT

private:

    /** The explosion ID that destroyed the block that contained this bonus and therefore cannot destroy this bonus */
    int m_undestroyableExplosionID;

    /** Timer used to animate explosion */
    QTimer* m_destructionTimer;

    /** Number of frames for the destruction */
    int m_destructionCounter;

public:

    /**
     * Creates a new BonusItem instance.
     * @param p_model the Bonus model
     * @param renderer the KGameRenderer
     */
    BonusItem(Bonus* p_model, KGameRenderer* renderer);

    /**
     * Deletes the BonusItem instance.
     */
    ~BonusItem() override;

    /**
     * Sets the undestroyable explosion IDs
     * @param nExplosionID the explosion that destroyed the block that contained this bonus and therefore can't destroy this bonus
     */
    void setUndestroyable(int nExplosionID);

    /**
     * Inits the destruction
     * @param nExplosionID the explosion ID from the blast that hit the bonus
     */
    void initDestruction(int nExplosionID);

private Q_SLOTS:
    /**
     * destruction animation
     */
    void destructionAnimation();

Q_SIGNALS:
    /**
     * signals the end of the destruction animation
     * @param bonusItem this bonus item
     */
    void bonusItemDestroyed(BonusItem* bonusItem);
};

#endif
