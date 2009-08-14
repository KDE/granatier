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

#include "bombitem.h"

#include <KDebug>

BombItem::BombItem(Bomb* p_model) : ElementItem (p_model)
{
    setElementId("bomb");
    connect(p_model, SIGNAL(bombDetonated(Bomb*)), this, SLOT(startDetonation(Bomb*)));
    connect(this, SIGNAL(bombItemFinished(BombItem*)), p_model, SLOT(slot_detonationCompleted()));
    
    // Define the timer which sets the puls frequency
    m_pulseTimer = new QTimer(this);
    m_pulseTimer->setInterval(100);
    m_pulseTimer->start();
    connect(m_pulseTimer, SIGNAL(timeout()), this, SLOT(pulse()));
    
    m_explosionTimer = NULL;
}

BombItem::~BombItem()
{
    if(m_pulseTimer)
    {
        delete m_pulseTimer;
    }
    if(m_explosionTimer)
    {
        delete m_explosionTimer;
    }
}

QPainterPath BombItem::shape() const
{
    QPainterPath path;
    // Temporary variable to keep the boundingRect available
    QRectF rect = boundingRect();

    // Calculation of the shape
    QRectF shapeRect = QRectF( rect.x()+rect.width()/4, rect.y()+rect.height()/4, rect.width()/2, rect.height()/2 );
    path.addEllipse(shapeRect);
    return path;
}

void BombItem::update(qreal p_x, qreal p_y)
{
    // Compute the top-right coordinates of the item
    qreal x = p_x - boundingRect().width() / 2;
    qreal y = p_y - boundingRect().height() / 2;
    // Updates the view coordinates
    setPos(x, y);
    m_x = p_x;
    m_y = p_y;
}

void BombItem::startDetonation(Bomb* bomb)
{
    m_numberPulse = 0;
    m_pulseTimer->stop();
    
    // Define the timer which sets the explosion frequency
    m_explosionCounter = 0;
    m_explosionTimer = new QTimer(this);
    m_explosionTimer->setInterval(600);
    m_explosionTimer->setSingleShot(true);
    m_explosionTimer->start();
    connect(m_explosionTimer, SIGNAL(timeout()), this, SLOT(explode()));
    
    setElementId("bomb_exploded");
    setZValue(300+13); //300+maxBombRange+3
    update(m_x, m_y);
}

void BombItem::pulse()
{
    m_numberPulse++;
    QTransform transform;
    if (m_numberPulse % 2 == 0)
    {
        // shrink the item
        transform.translate(boundingRect().width() / 2, boundingRect().height() / 2);
        transform.scale(0.95, 0.95);
        transform.translate(-boundingRect().width() / 2, -boundingRect().height() / 2);
        setTransform(transform);
    }
    else
    {
        resetTransform();
    }
}

void BombItem::explode()
{
    emit bombItemFinished(this);
}
