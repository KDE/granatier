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

#include "blockitem.h"
#include "block.h"

#include <QTimer>

#include <KGameRenderer>

BlockItem::BlockItem(Block* p_model, KGameRenderer* renderer) : ElementItem (p_model, renderer)
{
    setSpriteKey(p_model->getImageId());
    m_destructionTimer = NULL;
    connect(p_model, SIGNAL(startDestructionAnimation()), this, SLOT(startDestructionAnimation()));
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
    
    setSpriteKey("arena_block_highlight");
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
        m_destructionTimer = 0;
        emit blockItemDestroyed(this);
    }
}
