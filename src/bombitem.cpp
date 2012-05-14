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

const int nBombPulseTime = 100;

BombItem::BombItem(Bomb* p_model, KGameRenderer* renderer) : ElementItem (p_model, renderer)
{
    setSpriteKey("bomb");
    setZValue(210);
    connect(p_model, SIGNAL(bombDetonated(Bomb*)), this, SLOT(startDetonation(Bomb*)));
    connect(p_model, SIGNAL(falling()), this, SLOT(fallingAnimation()));
    connect(this, SIGNAL(bombItemFinished(BombItem*)), p_model, SLOT(slot_detonationCompleted()));
    
    m_itemSize = QSize(Granatier::CellSize * 0.9, Granatier::CellSize * 0.9);
    
    m_animationCounter = 0;
    // Define the timer which sets the puls frequency
    m_pulseTimer = new QTimer(this);
    m_pulseTimer->setInterval(nBombPulseTime);
    m_pulseTimer->start();
    connect(m_pulseTimer, SIGNAL(timeout()), this, SLOT(pulse()));
    
    m_explosionTimer = NULL;
    m_listExplosionTiming.append(Settings::self()->blastTime1());
    m_listExplosionTiming.append(Settings::self()->blastTime2());
    m_listExplosionTiming.append(Settings::self()->blastTime3());
    m_listExplosionTiming.append(Settings::self()->blastTime4());
    m_listExplosionTiming.append(Settings::self()->blastTime5());
    
    m_fallingAnimationCounter = 0;
    m_dudBomb = false;
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
    qreal x = p_x - m_itemSize.width() / 2;
    qreal y = p_y - m_itemSize.height() / 2;
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
    
    m_itemSize = QSize(Granatier::CellSize * 1.1, Granatier::CellSize * 1.1);
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
            int width = m_renderSize.width() * 0.96;
            int height = m_renderSize.height() * 0.96;
            if(m_renderSize.width() - width < 2)
            {
                width = m_renderSize.width() - 2;
            }
            if(m_renderSize.height() - height < 2)
            {
                height = m_renderSize.height() - 2;
            }
            qreal scale =  width / (qreal) (m_renderSize.width());
            // shrink the item
            QTransform transform;
            transform.translate(m_itemSize.width() / 2.0, m_itemSize.height() / 2.0);
            setRenderSize(QSize(width, height));
            transform.translate(-m_itemSize.width() * scale / 2.0, -m_itemSize.height() * scale / 2.0);
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
        transform.translate(m_itemSize.width() / 2.0, m_itemSize.height() / 2.0);
        setRenderSize(m_renderSize * (1-m_fallingAnimationCounter*0.02));
        transform.translate(-m_itemSize.width() * (1-m_fallingAnimationCounter*0.02) / 2.0, -m_itemSize.height() * (1-m_fallingAnimationCounter*0.02) / 2.0);
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

void BombItem::updateMortar(int nMortarState, int nMortarRampEnd, int nMortarPeak, int nMortarGround)
{
    if(m_pulseTimer)
    {
        m_pulseTimer->stop();
        delete m_pulseTimer;
        m_pulseTimer = 0;
    }
    
    if(nMortarState <= 0)
    {
        setVisible(false);
        setZValue(-1);
    }
    else if(nMortarState <= nMortarGround)
    {
        updateMortarAnimation(nMortarState, nMortarRampEnd, nMortarPeak);
    }
    else
    {
        if(!m_pulseTimer)
        {
            m_pulseTimer = new QTimer(this);
            m_pulseTimer->setInterval(nBombPulseTime);
            m_animationCounter = 1; // set to one, to start pulsing with a small bomb
            m_pulseTimer->start();
            connect(m_pulseTimer, SIGNAL(timeout()), this, SLOT(pulse()));
        }
        setRenderSize(m_renderSize);
        resetTransform();
        setVisible(true);
        setZValue(210);
    }
}

void BombItem::updateMortarAnimation(int animationCounter, int nMortarRampEnd, int nMortarPeak)
{
    qreal mortarScale = 1;
    int mortarZValue = 210;
    QTransform transform;

    if(animationCounter < nMortarRampEnd)
    {
        mortarZValue = 210;
    }
    else
    {
        mortarZValue = 800;
    }
    
    int frame = animationCounter - nMortarRampEnd;
    int peak = nMortarPeak - nMortarRampEnd;
    
    mortarScale = 1.5 - (frame-peak) * (frame-peak) / (qreal) (peak*peak) * 0.5;

    transform.translate(m_itemSize.width() / 2.0, m_itemSize.height() / 2.0);
    setRenderSize(m_renderSize * mortarScale);
    transform.translate(-m_itemSize.width() * mortarScale / 2.0, -m_itemSize.height() * mortarScale / 2.0);
    setTransform(transform);
    
    setVisible(true);
    setZValue(mortarZValue);
}

void BombItem::fallingAnimation()
{
    m_fallingAnimationCounter = 1;
    // set z-value below the ground
    setZValue(-2);
    m_pulseTimer->setInterval(1000 / Granatier::FPS);
}
