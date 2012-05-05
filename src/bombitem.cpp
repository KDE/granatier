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

#include <KGameRenderer>

BombItem::BombItem(Bomb* p_model, KGameRenderer* renderer) : ElementItem (p_model, renderer)
{
    setSpriteKey("bomb");
    setZValue(210);
    connect(p_model, SIGNAL(bombDetonated(Bomb*)), this, SLOT(startDetonation(Bomb*)));
    connect(p_model, SIGNAL(falling()), this, SLOT(fallingAnimation()));
    connect(this, SIGNAL(bombItemFinished(BombItem*)), p_model, SLOT(slot_detonationCompleted()));
    
    m_animationCounter = 0;
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
    
    m_mortarAnimationTimer = new QTimer(this);
    m_mortarAnimationTimer->setInterval(25);
    connect(m_mortarAnimationTimer, SIGNAL(timeout()), this, SLOT(updateMortarAnimation()));
    m_fallingAnimationCounter = 0;
    m_mortarAnimationCounter = 0;
    m_dudBomb = false;
}

BombItem::~BombItem()
{
    delete m_pulseTimer;
    delete m_explosionTimer;
    delete m_mortarAnimationTimer;
}

QPainterPath BombItem::shape() const
{
    QPainterPath path;
    // Temporary variable to keep the boundingRect available
    QRectF rect = boundingRect();

    // Calculation of the shape
    QRectF shapeRect = QRectF( rect.x()+rect.width()/8, rect.y()+rect.height()/8, rect.width()*3.0/4.0, rect.height()*3.0/4.0 );
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
    if(m_pulseTimer && !m_dudBomb)
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
    qreal x = p_x - renderer()->boundsOnSprite(spriteKey()).width() / 2;
    qreal y = p_y - renderer()->boundsOnSprite(spriteKey()).height() / 2;
    // Updates the view coordinates
    setPos(x, y);
    m_x = p_x;
    m_y = p_y;
}

void BombItem::startDetonation(Bomb* bomb)
{
    if(m_pulseTimer)
    {
        m_pulseTimer->stop();
        delete m_pulseTimer;
        m_pulseTimer = 0;
    }
    m_animationCounter = 0;
    
    dynamic_cast <Bomb*> (m_model)->setXSpeed(0);
    dynamic_cast <Bomb*> (m_model)->setYSpeed(0);
    
    // Define the timer which sets the explosion frequency
    m_explosionTimer = new QTimer(this);
    m_explosionTimer->setInterval(m_listExplosionTiming.at(0));
    m_explosionTimer->setSingleShot(true);
    m_explosionTimer->start();
    connect(m_explosionTimer, SIGNAL(timeout()), this, SLOT(updateAnimation()));
    
    setSpriteKey("bomb_blast_core_0");
    setZValue(300+15); //300+maxBombPower+5
    updateGraphics(scale());
}

void BombItem::pulse()
{
    if(m_fallingAnimationCounter == 0)
    {
        m_animationCounter++;
        if (m_animationCounter % 2 == 0)
        {
            m_animationCounter = 0;
            // shrink the item
            QTransform transform;
            transform.translate(renderer()->boundsOnSprite(spriteKey()).width() / 2, renderer()->boundsOnSprite(spriteKey()).height() / 2);
            setRenderSize(m_renderSize * 0.95);
            transform.translate(-renderer()->boundsOnSprite(spriteKey()).width() * 0.95 / 2, -renderer()->boundsOnSprite(spriteKey()).height() * 0.95 / 2);
            setTransform(transform);
        }
        else
        {
            setRenderSize(m_renderSize);
            resetTransform();
        }
    }
    else
    {
        // shrink the item
        QTransform transform;
        transform.translate(renderer()->boundsOnSprite(spriteKey()).width() / 2, renderer()->boundsOnSprite(spriteKey()).height() / 2);
        setRenderSize(m_renderSize * (1-m_fallingAnimationCounter*0.02));
        transform.translate(-renderer()->boundsOnSprite(spriteKey()).width() * (1-m_fallingAnimationCounter*0.02) / 2, -renderer()->boundsOnSprite(spriteKey()).height() * (1-m_fallingAnimationCounter*0.02) / 2);
        setTransform(transform);
        m_fallingAnimationCounter++;
        
        if(m_fallingAnimationCounter > 50)
        {
            m_pulseTimer->stop();
            m_dudBomb = true;
            emit bombItemFinished(this);
        }
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
    setSpriteKey(strElemetId);
    update(m_x, m_y);
    
    emit animationFrameChanged(this, m_animationCounter);
    m_explosionTimer->setInterval(m_listExplosionTiming.at(m_animationCounter));
    m_explosionTimer->start();
}

void BombItem::updateMortar(int nState)
{
    if(m_pulseTimer)
    {
        m_pulseTimer->stop();
        delete m_pulseTimer;
        m_pulseTimer = 0;
    }
    
    switch(nState)
    {
      case 0:
            m_mortarAnimationCounter = 0;
            setVisible(false);
            setZValue(-1);
            break;
        case 1:
            {
                m_mortarAnimationCounter = 0;
                if(!m_mortarAnimationTimer->isActive())
                {
                    m_mortarAnimationTimer->start();
                }
            }
            break;
        case 2:
            {
                if(!m_mortarAnimationTimer->isActive())
                {
                    m_mortarAnimationCounter = 2;
                    m_mortarAnimationTimer->start();
                }
            }
            break;
        case 3:
        default:
            if(!m_pulseTimer)
            {
                m_pulseTimer = new QTimer(this);
                m_pulseTimer->setInterval(100);
                m_pulseTimer->start();
                connect(m_pulseTimer, SIGNAL(timeout()), this, SLOT(pulse()));
            }
            m_mortarAnimationTimer->stop();
            m_mortarAnimationCounter = 0;
            setRenderSize(m_renderSize);
            resetTransform();
            setVisible(true);
            setZValue(210);
            break;
    }
}

void BombItem::updateMortarAnimation()
{
    qreal mortarScale = 1;
    int mortarZValue = 210;
    QTransform transform;
    

    if(m_mortarAnimationCounter < 2)
    {
        mortarZValue = 210;
    }
    else
    {
        mortarZValue = 800;
    }
    
    int frame = m_mortarAnimationCounter - 2;
    mortarScale = 1.5 - (frame-16) * (frame-16) / (qreal) (16*16) * 0.5;

    transform.translate(renderer()->boundsOnSprite(spriteKey()).width() / 2, renderer()->boundsOnSprite(spriteKey()).height() / 2);
    setRenderSize(m_renderSize * mortarScale);
    transform.translate(-renderer()->boundsOnSprite(spriteKey()).width() * mortarScale / 2, -renderer()->boundsOnSprite(spriteKey()).height() * mortarScale / 2);
    setTransform(transform);
    
    setVisible(true);
    setZValue(mortarZValue);
    
    m_mortarAnimationCounter++;
}

void BombItem::fallingAnimation()
{
    m_fallingAnimationCounter = 1;
    // set z-value below the ground
    setZValue(-2);
    m_pulseTimer->setInterval(25);
}
