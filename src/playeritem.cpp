/*
 * Copyright 2009 Mathias Kraus <k.hias@gmx.de>
 * Copyright 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
 * Copyright 2007-2008 Nathalie Liesse <nathalie.liesse@gmail.com>
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


#include "playeritem.h"
#include "player.h"
#include "bonus.h"
#include "bonusitem.h"
#include "bomb.h"
#include "bombitem.h"
#include "bombexplosionitem.h"

#include <QTimeLine>

#include <KStandardDirs>
#include <KGameRenderer>

const int PlayerItem::NB_FRAMES = 13;
const int PlayerItem::ANIM_SPEED = 240;

PlayerItem::PlayerItem(Player* p_model, KGameRenderer* renderer) : CharacterItem(p_model, renderer)
{
    connect(p_model, SIGNAL(directionChanged()), this, SLOT(updateDirection()));
    connect(p_model, SIGNAL(gameUpdated()), this, SLOT(manageCollision()));
    connect(p_model, SIGNAL(stopped()), this, SLOT(stopAnim()));
    connect(p_model, SIGNAL(falling()), this, SLOT(fallingAnimation()));
    connect(p_model, SIGNAL(resurrected()), this, SLOT(resurrect()));

    // A timeLine for the Player animation
    m_animationTimer = new QTimeLine();
    m_animationTimer->setCurveShape(QTimeLine::LinearCurve);
    m_animationTimer->setLoopCount(0);
    m_animationTimer->setFrameRange(0, NB_FRAMES - 1);
    // Animation speed
    m_animationTimer->setDuration(PlayerItem::ANIM_SPEED);
    connect(m_animationTimer, SIGNAL(frameChanged(int)), this, SLOT(setFrame(int)));
    
    m_itemSize = QSize(Granatier::CellSize * 0.9, Granatier::CellSize * 0.9);
    
    if(m_renderer->spriteExists("player_0"))
    {
        setSpriteKey("player_0");
    }
    
    setZValue(250);
    
    m_fallingAnimationCounter = 0;
    m_resurrectionAnimationCounter = 0;
}

PlayerItem::~PlayerItem()
{
    delete m_animationTimer;
}

void PlayerItem::resurrect()
{
    int nDirection = dynamic_cast <Player*> (m_model)->direction();
    setZValue(250);
    m_fallingAnimationCounter = 0;
    m_resurrectionAnimationCounter = 10;
    setRenderSize(m_renderSize);
    resetTransform();
    if(m_renderer->spriteExists("player_0"))
    {
        setSpriteKey("player_0");
    }
    
    QTransform transform;
    transform.translate(m_itemSize.width() / 2.0, m_itemSize.width() / 2.0);
    // get the angle
    switch(nDirection)
    {
        case Granatier::Direction::EAST:
            transform.rotate(0);
            break;
        case Granatier::Direction::SOUTH:
            transform.rotate(90);
            break;
        case Granatier::Direction::NORTH:
            transform.rotate(-90);
            break;
        case Granatier::Direction::WEST:
            transform.rotate(180);
            break;
        default:
            transform.rotate(0);
            break;
    }
    transform.translate(-m_itemSize.width() / 2.0, -m_itemSize.width() / 2.0);
    setTransform(transform);
    
    startAnim();
    
    setVisible(true);
}

void PlayerItem::updateDirection()
{
    int nDirection = dynamic_cast <Player*> (m_model)->direction();
    // Rotate the item
    QTransform transform;
    transform.translate(m_itemSize.width() / 2.0, m_itemSize.width() / 2.0);
    // get the angle
    switch(nDirection)
    {
        case Granatier::Direction::EAST:
            transform.rotate(0);
            break;
        case Granatier::Direction::SOUTH:
            transform.rotate(90);
            break;
        case Granatier::Direction::NORTH:
            transform.rotate(-90);
            break;
        case Granatier::Direction::WEST:
            transform.rotate(180);
            break;
        default:
            transform.rotate(0);
            break;
    }
    transform.translate(-m_itemSize.width() / 2.0, -m_itemSize.width() / 2.0);
    setTransform(transform);
}

void PlayerItem::manageCollision()
{
    QList<QGraphicsItem*> collidingList = collidingItems();
    BonusItem* bonusItem;

    // The signal is emitted only if the list contains more than 1 items (to exclude the case
    // when the player only collides with the arena)
    if (collidingList.size() > 1)
    {
        for (int i = 0; i < collidingList.size(); ++i)
        {
            // The arena and the points labels have a negative zValue which allows to exclude them from the treatment of collisions
            if (collidingList[i]->zValue() >= 300 && collidingList[i]->zValue() < 400)
            {
                //((ElementItem*)collidingList[i])->getModel()->doActionOnCollision((Player*)getModel());
                int nExplosionID;
                if(collidingList[i]->zValue() == 315)
                {
                    BombItem* bombItem = dynamic_cast <BombItem*> (collidingList[i]);
                    nExplosionID = dynamic_cast <Bomb*> (bombItem->getModel())->explosionID();
                }
                else
                {
                    nExplosionID = dynamic_cast <BombExplosionItem*> (collidingList[i])->explosionID();
                }
                
                if(dynamic_cast <Player*> (m_model)->shield(nExplosionID) == false)
                {
                    setDead();
                    dynamic_cast <Player*> (m_model)->die();
                }
            }
            else if (collidingList[i]->zValue() == 100)
            {
                bonusItem = dynamic_cast <BonusItem*> (collidingList[i]);
                if(dynamic_cast <Bonus*> (bonusItem->getModel())->isTaken() == false)
                {
                    dynamic_cast <Bonus*> (bonusItem->getModel())->setTaken();
                    bonusItem->getModel()->doActionOnCollision(dynamic_cast <Player*> (this->getModel()));
                    emit bonusItemTaken(bonusItem);
                }
            }
        }
    }
}

void PlayerItem::update(qreal p_x, qreal p_y)
{
    ElementItem::update(p_x, p_y);
    
    // If the player is moving
    if (((Player*)getModel())->getXSpeed() != 0 || ((Player*)getModel())->getYSpeed() != 0)
    {
        startAnim();
    }
}

void PlayerItem::startAnim()
{
    // Start the animation timer if it is not active
    if (m_animationTimer->state() != QTimeLine::Running)
    {
        m_animationTimer->start();
    }
}

void PlayerItem::pauseAnim()
{
    if (m_animationTimer->state() == QTimeLine::Running)
    {
        m_animationTimer->setPaused(true);
    }
}

void PlayerItem::resumeAnim()
{
    if (m_animationTimer->state() == QTimeLine::Paused)
    {
        m_animationTimer->setPaused(false);
    }
}

void PlayerItem::stopAnim()
{
    if(m_renderer->spriteExists("player_0"))
    {
        setSpriteKey("player_0");
    }
    if (m_animationTimer->state() != QTimeLine::NotRunning && m_resurrectionAnimationCounter == 0)
    {
        m_animationTimer->stop();
    }
}

void PlayerItem::fallingAnimation()
{
    m_fallingAnimationCounter = 1;
}

void PlayerItem::setFrame(const int p_frame)
{
    if(m_renderer->spriteExists(QString("player_%1").arg(p_frame)))
    {
        setSpriteKey(QString("player_%1").arg(p_frame));
        
        if(m_fallingAnimationCounter > 0 || m_resurrectionAnimationCounter > 0)
        {
            int angle = 0;
            int nDirection = dynamic_cast <Player*> (m_model)->direction();

            // get the angle
            switch(nDirection)
            {
                case Granatier::Direction::EAST:
                    angle = 0;
                    break;
                case Granatier::Direction::SOUTH:
                    angle = 90;
                    break;
                case Granatier::Direction::NORTH:
                    angle = -90;
                    break;
                case Granatier::Direction::WEST:
                    angle = 180;
                    break;
                default:
                    angle = 0;
                    break;
            }
            
            if(m_fallingAnimationCounter > 0)
            {
                // set z-value below the ground
                setZValue(-2);
                // shrink the item
                QTransform transform;
                transform.translate(m_itemSize.width() / 2.0, m_itemSize.width() / 2.0);
                transform.rotate(angle);
                setRenderSize(m_renderSize * (1-m_fallingAnimationCounter*0.02));
                transform.translate(-m_itemSize.width() * (1-m_fallingAnimationCounter*0.02) / 2.0, -m_itemSize.width() * (1-m_fallingAnimationCounter*0.02) / 2.0);
                setTransform(transform);
                m_fallingAnimationCounter++;
                
                if(m_fallingAnimationCounter > 50)
                {
                    setDead();
                    dynamic_cast <Player*> (m_model)->die();
                    setVisible(false);
                }
            }
            
            if(m_resurrectionAnimationCounter > 0)
            {
                qreal resurrectionScale = 1;
                QTransform transform;
                transform.translate(m_itemSize.width() / 2.0, m_itemSize.width() / 2.0);
                transform.rotate(angle);
                if(m_resurrectionAnimationCounter > 9)
                {
                    resurrectionScale = 1.1;
                }
                else if(m_resurrectionAnimationCounter > 8)
                {
                    resurrectionScale = 0.9;
                }
                else if(m_resurrectionAnimationCounter > 7)
                {
                    resurrectionScale = 0.6;
                }
                else if(m_resurrectionAnimationCounter > 6)
                {
                    resurrectionScale = 0.7;
                }
                else if(m_resurrectionAnimationCounter > 5)
                {
                    resurrectionScale = 0.85;
                }
                else if(m_resurrectionAnimationCounter > 4)
                {
                    resurrectionScale = 1;
                }
                else if(m_resurrectionAnimationCounter > 3)
                {
                    resurrectionScale = 1.05;
                }
                else if(m_resurrectionAnimationCounter > 2)
                {
                    resurrectionScale = 1.1;
                }
                else if(m_resurrectionAnimationCounter > 1)
                {
                    resurrectionScale = 1.05;
                }
                else if(m_resurrectionAnimationCounter > 0)
                {
                    resurrectionScale = 1;
                }
                
                m_resurrectionAnimationCounter--;
                if(m_resurrectionAnimationCounter == 0)
                {
                    resurrectionScale = 1;
                    stopAnim();
                }
                
                setRenderSize(m_renderSize * resurrectionScale);
                transform.translate(-m_itemSize.width() * resurrectionScale / 2.0, -m_itemSize.width() * resurrectionScale / 2.0);
                setTransform(transform);
            }
        }
    }
}

void PlayerItem::setDead()
{
    stopAnim();
    setZValue(1);
    if(m_resurrectionAnimationCounter != 0)
    {
        setRenderSize(m_renderSize);
        resetTransform();
    }
    if(m_renderer->spriteExists("player_death"))
    {
        setSpriteKey("player_death");
    }
}
