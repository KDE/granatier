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

#include <QTimeLine>
#include <QGraphicsScene>
#include <KSvgRenderer>
#include <KStandardDirs>

const int PlayerItem::NB_FRAMES = 13;
const int PlayerItem::ANIM_SPEED = 240;

PlayerItem::PlayerItem(Player* p_model) : CharacterItem(p_model)
{
    connect(p_model, SIGNAL(directionChanged()), this, SLOT(updateDirection()));
    connect(p_model, SIGNAL(gameUpdated()), this, SLOT(manageCollision()));
    connect(p_model, SIGNAL(stopped()), this, SLOT(stopAnim()));

    // load the SVG
    QString strPlayerId = ((Player*) p_model)->getGraphicsPath();
    m_renderer->load(KStandardDirs::locate("appdata", QString("players/%1").arg(strPlayerId)));
    // A timeLine for the Player animation	
    m_animationTimer = new QTimeLine();
    m_animationTimer->setCurveShape(QTimeLine::LinearCurve);
    m_animationTimer->setLoopCount(0);
    m_animationTimer->setFrameRange(0, NB_FRAMES - 1);
    // Animation speed
    m_animationTimer->setDuration(PlayerItem::ANIM_SPEED);
    connect(m_animationTimer, SIGNAL(frameChanged(int)), this, SLOT(setFrame(int)));
    
    setElementId("player_0");
}

PlayerItem::~PlayerItem()
{
    delete m_animationTimer;
}

void PlayerItem::setPlayerId(QString strPlayerId)
{
    m_strPlayerId = strPlayerId;
    setElementId("player_0");
}

void PlayerItem::updateDirection()
{
    QTransform transform;
    int angle = 0;
    Player* model = (Player*)getModel();

    // Compute the angle
    if (model->getXSpeed() > 0)
    {
        angle = 0;
    }
    else if (model->getXSpeed() < 0)
    {
        angle = 180;	// The default image is right oriented
    }
    
    if (model->getYSpeed() > 0)
    {
        angle = 90;
    }
    else if (model->getYSpeed() < 0)
    {
        angle = -90;
    }
    
    // Rotate the item
    transform.translate(boundingRect().width() / 2, boundingRect().height() / 2);
    transform.rotate(angle);
    transform.translate(-boundingRect().width() / 2, -boundingRect().height() / 2);
    setTransform(transform);
}

void PlayerItem::manageCollision()
{
    QList<QGraphicsItem*> collidingList = collidingItems();
    ElementItem* elementItem;

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
                setElementId("player_death");
                dynamic_cast <Player*> (m_model)->die();
            }
            else if (collidingList[i]->zValue() == 100)
            {
                elementItem = dynamic_cast <ElementItem*> (collidingList[i]);
                if(dynamic_cast <Bonus*> (elementItem->getModel())->isTaken() == false)
                {
                    dynamic_cast <Bonus*> (elementItem->getModel())->setTaken();
                    elementItem->getModel()->doActionOnCollision(dynamic_cast <Player*> (this->getModel()));
                    emit bonusItemTaken(elementItem);
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
    if (m_animationTimer->state() == QTimeLine::Running)
    {
        m_animationTimer->setPaused(false);
    }
}

void PlayerItem::stopAnim()
{
    setElementId("player_0");
    if (m_animationTimer->state() == QTimeLine::Running)
    {
        m_animationTimer->stop();
    }
}

void PlayerItem::setFrame(const int p_frame)
{
    setElementId(QString("player_%1").arg(p_frame));
}

void PlayerItem::setDead()
{
    stopAnim();
    setElementId("player_death");
}