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

#ifndef BLOCKITEM_H
#define BLOCKITEM_H

#include "elementitem.h"

class Block;
class QTimer;
class KGameRenderer;

/**
 * @brief This class is the graphical representation of a Block.
 */
class BlockItem : public ElementItem
{

    Q_OBJECT

protected:

    /** Timer used to animate explosion */
    QTimer* m_destructionTimer;

    /** Number of frames for the destruction */
    int m_destructionCounter;

public:

    /**
     * Creates a new BlockItem instance.
     * @param p_model the Block model
     * @param renderer the KGameRenderer
     */
    BlockItem(Block* p_model, KGameRenderer* renderer);

    /**
     * Deletes the BlockItem instance.
     */
    ~BlockItem();

private Q_SLOTS:
    /**
     * Starts the destruction animation
     */
     void startDestructionAnimation();

     /**
     * destruction animation
     */
     void destructionAnimation();

Q_SIGNALS:
     /**
     * signals the end of the destruction animation
     * @param blockItem this block item
     */
     void blockItemDestroyed(BlockItem* blockItem);
};

#endif
