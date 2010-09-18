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
#include "characteritem.h"
#include "player.h"
#include "bonus.h"
#include "bonusitem.h"
#include "bomb.h"
#include "bombitem.h"
#include "bombexplosionitem.h"

#include <QTimeLine>

#include <KStandardDirs>
#include <KGameRenderer>

static int nCounter = 0;

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
    
    if(m_renderer->spriteExists("player_0"))
    {
        setSpriteKey("player_0");
    }
    
    setZValue(250);
    
    m_fallingAnimationCounter = 0;
    m_ressurectionAnimationCounter = 0;
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
    m_ressurectionAnimationCounter = 10;
    resetTransform();
    if(m_renderer->spriteExists("player_0"))
    {
        setSpriteKey("player_0");
    }
    QTransform transform;
    transform.translate(renderer()->boundsOnSprite(spriteKey()).width() / 2, renderer()->boundsOnSprite(spriteKey()).height() / 2);
    // get the angle
    switch(nDirection)
    {
        case Element::EAST:
            transform.rotate(0);
            break;
        case Element::SOUTH:
            transform.rotate(90);
            break;
        case Element::NORTH:
            transform.rotate(-90);
            break;
        case Element::WEST:
            transform.rotate(180);
            break;
        default:
            transform.rotate(0);
            break;
    }
    transform.scale(1, 1);
    transform.translate(-renderer()->boundsOnSprite(spriteKey()).width() / 2, -renderer()->boundsOnSprite(spriteKey()).height() / 2);
    setTransform(transform);
    
    startAnim();
    
    setVisible(true);
}

void PlayerItem::updateDirection()
{
    int nDirection = dynamic_cast <Player*> (m_model)->direction();
    // Rotate the item
    QTransform transform;
    transform.translate(renderer()->boundsOnSprite(spriteKey()).width() / 2, renderer()->boundsOnSprite(spriteKey()).height() / 2);
    // get the angle
    switch(nDirection)
    {
        case Element::EAST:
            transform.rotate(0);
            break;
        case Element::SOUTH:
            transform.rotate(90);
            break;
        case Element::NORTH:
            transform.rotate(-90);
            break;
        case Element::WEST:
            transform.rotate(180);
            break;
        default:
            transform.rotate(0);
            break;
    }
    transform.translate(-renderer()->boundsOnSprite(spriteKey()).width() / 2, -renderer()->boundsOnSprite(spriteKey()).height() / 2);
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
    if (m_animationTimer->state() != QTimeLine::NotRunning && m_ressurectionAnimationCounter == 0)
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
        
        if(m_fallingAnimationCounter > 0 || m_ressurectionAnimationCounter > 0)
        {
            int angle = 0;
            int nDirection = dynamic_cast <Player*> (m_model)->direction();

            // get the angle
            switch(nDirection)
            {
                case Element::EAST:
                    angle = 0;
                    break;
                case Element::SOUTH:
                    angle = 90;
                    break;
                case Element::NORTH:
                    angle = -90;
                    break;
                case Element::WEST:
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
                transform.translate(renderer()->boundsOnSprite(spriteKey()).width() / 2, renderer()->boundsOnSprite(spriteKey()).height() / 2);
                transform.rotate(angle);
                transform.scale(1-m_fallingAnimationCounter*0.02, 1-m_fallingAnimationCounter*0.02);
                transform.translate(-renderer()->boundsOnSprite(spriteKey()).width() / 2, -renderer()->boundsOnSprite(spriteKey()).height() / 2);
                setTransform(transform);
                m_fallingAnimationCounter++;
                
                if(m_fallingAnimationCounter > 50)
                {
                    setDead();
                    dynamic_cast <Player*> (m_model)->die();
                    setVisible(false);
                }
            }
            
            if(m_ressurectionAnimationCounter > 0)
            {
                QTransform transform;
                transform.translate(renderer()->boundsOnSprite(spriteKey()).width() / 2, renderer()->boundsOnSprite(spriteKey()).height() / 2);
                transform.rotate(angle);
                if(m_ressurectionAnimationCounter > 9)
                {
                    transform.scale(1.1, 1.1);
                }
                else if(m_ressurectionAnimationCounter > 8)
                {
                    transform.scale(0.9, 0.9);
                }
                else if(m_ressurectionAnimationCounter > 7)
                {
                    transform.scale(0.6, 0.6);
                }
                else if(m_ressurectionAnimationCounter > 6)
                {
                    transform.scale(0.7, 0.7);
                }
                else if(m_ressurectionAnimationCounter > 5)
                {
                    transform.scale(0.85, 0.85);
                }
                else if(m_ressurectionAnimationCounter > 4)
                {
                    transform.scale(1.0, 1.0);
                }
                else if(m_ressurectionAnimationCounter > 3)
                {
                    transform.scale(1.05, 1.05);
                }
                else if(m_ressurectionAnimationCounter > 2)
                {
                    transform.scale(1.1, 1.1);
                }
                else if(m_ressurectionAnimationCounter > 1)
                {
                    transform.scale(1.05, 1.05);
                }
                else if(m_ressurectionAnimationCounter > 0)
                {
                    transform.scale(1.0, 1.0);
                }
                
                m_ressurectionAnimationCounter--;
                if(m_ressurectionAnimationCounter == 0)
                {
                    transform.scale(1.0, 1.0);
                    stopAnim();
                }
                
                transform.translate(-renderer()->boundsOnSprite(spriteKey()).width() / 2, -renderer()->boundsOnSprite(spriteKey()).height() / 2);
                setTransform(transform);
            }
        }
    }
}

void PlayerItem::setDead()
{
    stopAnim();
    setZValue(1);
    if(m_ressurectionAnimationCounter != 0)
    {
        resetTransform();
    }
    if(m_renderer->spriteExists("player_death"))
    {
        setSpriteKey("player_death");
    }
}
