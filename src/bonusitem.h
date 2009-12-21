/*
 * Copyright 2009 Mathias Kraus <k.hias@gmx.de>
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

#ifndef BONUSITEM_H
#define BONUSITEM_H

#include "elementitem.h"

class Bonus;
class QTimer;

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
     */
    BonusItem(Bonus* p_model);

    /**
     * Deletes the BonusItem instance.
     */
    ~BonusItem();
    
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

private slots:
    /**
     * destruction animation
     */
    void destructionAnimation();
    
signals:
    /**
     * signals the end of the destruction animation
     * @param bonusItem this bonus item
     */
    void bonusItemDestroyed(BonusItem* bonusItem);
};

#endif
