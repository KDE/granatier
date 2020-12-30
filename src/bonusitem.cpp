/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "bonusitem.h"
#include "bonus.h"

#include <QTimer>

#include <KGameRenderer>

BonusItem::BonusItem(Bonus* p_model, KGameRenderer* renderer) : ElementItem (p_model, renderer)
{
    setSpriteKey(p_model->getImageId());
    m_undestroyableExplosionID = 0;
    m_destructionTimer = nullptr;
    m_destructionCounter = 0;
}

BonusItem::~BonusItem()
{
    delete m_destructionTimer;
}

void BonusItem::setUndestroyable(int nExplosionID)
{
    m_undestroyableExplosionID = nExplosionID;
}

void BonusItem::initDestruction(int nExplosionID)
{
    if(m_undestroyableExplosionID == nExplosionID || dynamic_cast <Bonus*> (m_model)->isTaken())
    {
        return;
    }

    dynamic_cast <Bonus*> (m_model)->setDestroyed();

    m_destructionCounter = 0;
    m_destructionTimer = new QTimer(this);
    m_destructionTimer->setInterval(300);
    m_destructionTimer->start();
    connect(m_destructionTimer, &QTimer::timeout, this, &BonusItem::destructionAnimation);
}

void BonusItem::destructionAnimation()
{
    m_destructionCounter++;
    m_destructionTimer->setInterval(50);
    setOpacity((5-m_destructionCounter)/5.0);
    if(m_destructionCounter >= 5)
    {
        m_destructionTimer->stop();
        delete m_destructionTimer;
        m_destructionTimer = nullptr;
        Q_EMIT bonusItemDestroyed(this);
    }
}
