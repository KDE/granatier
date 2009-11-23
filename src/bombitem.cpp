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
#include "bomb.h"
#include "settings.h"

#include <QTimer>

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
    m_listExplosionTiming.append(Settings::self()->blastTime1());
    m_listExplosionTiming.append(Settings::self()->blastTime2());
    m_listExplosionTiming.append(Settings::self()->blastTime3());
    m_listExplosionTiming.append(Settings::self()->blastTime4());
    m_listExplosionTiming.append(Settings::self()->blastTime5());
}

BombItem::~BombItem()
{
    delete m_pulseTimer;
    delete m_explosionTimer;
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

void BombItem::pauseAnim()
{
    dynamic_cast <Bomb*> (m_model)->pause();
    if(m_pulseTimer)
    {
        m_pulseTimer->stop();
    }
    if(m_explosionTimer)
    {
        m_explosionTimer->stop();
    }    
}

void BombItem::resumeAnim()
{
    if(m_pulseTimer)
    {
        m_pulseTimer->start();
    }
    if(m_explosionTimer)
    {
        m_explosionTimer->start();
    }
    dynamic_cast <Bomb*> (m_model)->resume();
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
    m_animationCounter = 0;
    m_pulseTimer->stop();
    delete m_pulseTimer;
    m_pulseTimer = 0;
    
    dynamic_cast <Bomb*> (m_model)->setXSpeed(0);
    dynamic_cast <Bomb*> (m_model)->setYSpeed(0);
    
    // Define the timer which sets the explosion frequency
    m_explosionTimer = new QTimer(this);
    m_explosionTimer->setInterval(m_listExplosionTiming.at(0));
    m_explosionTimer->setSingleShot(true);
    m_explosionTimer->start();
    connect(m_explosionTimer, SIGNAL(timeout()), this, SLOT(updateAnimation()));
    
    setElementId("bomb_blast_core_0");
    setZValue(300+15); //300+maxBombPower+5
    update(m_x, m_y);
}

void BombItem::pulse()
{
    m_animationCounter++;
    QTransform transform;
    if (m_animationCounter % 2 == 0)
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

void BombItem::updateAnimation()
{
    m_animationCounter++;
    if (m_animationCounter > 4)
    {
        emit bombItemFinished(this);
        m_animationCounter = 0;
        return;
    }
    QString strElemetId = QString("bomb_blast_core_%1").arg(m_animationCounter);
    setElementId(strElemetId);
    emit animationFrameChanged(this, m_animationCounter);
    m_explosionTimer->setInterval(m_listExplosionTiming.at(m_animationCounter));
    m_explosionTimer->start();
}
