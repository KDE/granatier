/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    ~BlockItem() override;

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
