/*
    SPDX-FileCopyrightText: 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
    SPDX-FileCopyrightText: 2007-2008 Gaël Courcelle <gael.courcelle@gmail.com>
    SPDX-FileCopyrightText: 2007-2008 Alexia Allanic <alexia_allanic@yahoo.fr>
    SPDX-FileCopyrightText: 2007-2008 Johann Hingue <yoan1703@hotmail.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "bonus.h"
#include "player.h"

Bonus::Bonus(qreal p_x, qreal p_y, Arena* p_arena, Granatier::Bonus::Type bonusType) : Element(p_x, p_y, p_arena)
{
    m_type = Granatier::Element::BONUS;
    m_bonusType = bonusType;
    m_taken = false;
    m_destroyed = false;
}

Bonus::~Bonus()
{

}

void Bonus::doActionOnCollision(Player* p_player)
{
    p_player->addBonus(this);
}

Granatier::Bonus::Type Bonus::getBonusType() const
{
  return m_bonusType;
}

void Bonus::setTaken()
{
    m_taken = true;
}

bool Bonus::isTaken() const
{
    return m_taken;
}

void Bonus::setDestroyed()
{
    m_destroyed = true;
    m_taken = true;
}

bool Bonus::isDestroyed() const
{
    return m_destroyed;
}
