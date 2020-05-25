/*
 * Copyright 2009 Mathias Kraus <k.hias@gmx.de>
 * Copyright 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
 * Copyright 2007-2008 Pierre-Benoît Besse <besse.pb@gmail.com>
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

#include "character.h"
#include "arena.h"

Character::Character(qreal p_x, qreal p_y, Arena* p_arena) : Element(p_x, p_y, p_arena), m_xSpeed(0), m_ySpeed(0)
{
    initSpeed();
    m_maxSpeed = m_normalSpeed;	// To avoid bugs, but will be overridden in the Player constructors
}

Character::~Character()
{
}

void Character::move()
{
    // Take care of the Arena borders
    if (m_arena->getColFromX(m_x + m_xSpeed) == 0)                                  // First column
    {
        m_x = (m_arena->getNbColumns() - 1.5) * Granatier::CellSize;
    }
    else if (m_arena->getColFromX(m_x + m_xSpeed) == m_arena->getNbColumns() - 1)   // Last column
    {
        m_x = 1.5 * Granatier::CellSize;
    }
    else if (m_arena->getRowFromY(m_y + m_ySpeed) == 0)                            // First row
    {
        m_y = (m_arena->getNbRows() - 1.5) * Granatier::CellSize;
    }
    else if (m_arena->getRowFromY(m_y + m_ySpeed) == m_arena->getNbRows() - 1)     // Last row
    {
        m_y = 1.5 * Granatier::CellSize;
    }
    // Move the Character
    m_x += m_xSpeed;
    m_y += m_ySpeed;
    Q_EMIT moved(m_x, m_y);
}

void Character::die()
{
    Q_EMIT dead();
}

qreal Character::getXSpeed() const
{
    return m_xSpeed;
}

qreal Character::getYSpeed() const
{
    return m_ySpeed;
}

qreal Character::getSpeed() const
{
    return m_speed;
}

qreal Character::getNormalSpeed() const
{
    return m_normalSpeed;
}

void Character::setXSpeed(qreal p_xSpeed)
{
    m_xSpeed = p_xSpeed;
}

void Character::setYSpeed(qreal p_ySpeed)
{
    m_ySpeed = p_ySpeed;
}

void Character::initSpeed()
{
    // Player speed
    m_normalSpeed = 1;
    m_speed = m_normalSpeed;
}

bool Character::isInLineSight(Character* p_character) const
{
    int curCallerRow;       // The current row of the Character
    int curCallerCol;       // The current column of the Character
    int curCharacterRow;    // The current row of the other Character
    int curCharacterCol;    // The current column of the other Character

    curCallerRow = m_arena->getRowFromY(m_y);
    curCallerCol = m_arena->getColFromX(m_x);
    curCharacterRow = m_arena->getRowFromY(p_character->getY());
    curCharacterCol = m_arena->getColFromX(p_character->getX());

    // If the two Characters are on the same row
    if (curCallerRow == curCharacterRow )
    {
        // If The Character is on the right of the other one and goes to the left
        if (curCallerCol > curCharacterCol && m_xSpeed < 0)
        {
            // Check there is a wall between them
            for (int i = curCharacterCol; i < curCallerCol; ++i)
            {
                if (m_arena->getCell(curCallerRow, i).getType() != Granatier::Cell::GROUND)
                {
                    return false;
                }
            }
            // If not, the other Character is in the line sight
            return true;
        // If the Character is on the left of the other one and goes to the right
        }
        else if (curCallerCol < curCharacterCol && m_xSpeed > 0)
        {
            // Check there is a wall between them
            for (int i = curCallerCol; i < curCharacterCol; ++i)
            {
                if (m_arena->getCell(curCallerRow, i).getType() != Granatier::Cell::GROUND)
                {
                    return false;
                }
            }
            // If not, the other Character is in the line sight
            return true;
        }
    // If the two Characters are on the same column
    }
    else if (curCallerCol == curCharacterCol)
    {
        // If The Character is on the bottom of the other one and goes up
        if (curCallerRow > curCharacterRow && m_ySpeed < 0)
        {
            // Check there is a wall between them
            for (int i = curCharacterRow; i < curCallerRow; ++i)
            {
                if (m_arena->getCell(i, curCallerCol).getType() != Granatier::Cell::GROUND)
                {
                    return false;
                }
            }
            // If not, the other Character is in the line sight
            return true;
        // If the Character is on the top of the other one and goes down
        }
        else if (curCallerRow < curCharacterRow && m_ySpeed > 0)
        {
            // Check there is a wall between them
            for (int i = curCallerRow; i < curCharacterRow; ++i)
            {
                if (m_arena->getCell(i, curCallerCol).getType() != Granatier::Cell::GROUND)
                {
                    return false;
                }
            }
            // If not, the other Character is in the line sight
            return true;
        }
    }
    // If the two Characters are not on the same row or column, they are not in the line of sight
    return false;
}
