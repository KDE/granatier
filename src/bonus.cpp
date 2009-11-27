/*
 * Copyright 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
 * Copyright 2007-2008 Gaël Courcelle <gael.courcelle@gmail.com>
 * Copyright 2007-2008 Alexia Allanic <alexia_allanic@yahoo.fr>
 * Copyright 2007-2008 Johann Hingue <yoan1703@hotmail.fr>
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

#include "bonus.h"
#include "player.h"

Bonus::Bonus(qreal p_x, qreal p_y, Arena* p_arena, Bonus::BonusType bonusType) : Element(p_x, p_y, p_arena)
{
    m_type = Element::BONUS;
    m_bonusType = bonusType;
    m_taken = false;
}

Bonus::~Bonus()
{

}

void Bonus::doActionOnCollision(Player* p_player)
{
    p_player->addBonus(this);
}

Bonus::BonusType Bonus::getBonusType() const
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
