/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "blockitem.h"
#include "block.h"

#include <QTimer>

#include <KGameRenderer>

BlockItem::BlockItem(Block* p_model, KGameRenderer* renderer) : ElementItem (p_model, renderer)
{
    setSpriteKey(p_model->getImageId());
    m_destructionTimer = nullptr;
    connect(p_model, &Block::startDestructionAnimation, this, &BlockItem::startDestructionAnimation);
}

BlockItem::~BlockItem()
{
    delete m_destructionTimer;
}

void BlockItem::startDestructionAnimation()
{
    m_destructionCounter = 0;
    m_destructionTimer = new QTimer(this);
    m_destructionTimer->setInterval(300);
    m_destructionTimer->start();
    connect(m_destructionTimer, &QTimer::timeout, this, &BlockItem::destructionAnimation);

    setSpriteKey(QStringLiteral("arena_block_highlight"));
}

void BlockItem::destructionAnimation()
{
    m_destructionCounter++;
    m_destructionTimer->setInterval(50);
    setOpacity((5-m_destructionCounter)/5.0);
    if(m_destructionCounter >= 5)
    {
        m_destructionTimer->stop();
        delete m_destructionTimer;
        m_destructionTimer = nullptr;
        Q_EMIT blockItemDestroyed(this);
    }
}
