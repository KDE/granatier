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

#include "bomb.h"

#include <KGameDifficulty>
#include <KDebug>

Bomb::Bomb(qreal fX, qreal fY, Maze* pMaze, int nDetonationCountdown) : Element(fX, fY, pMaze), m_xSpeed(0), m_ySpeed(0)
{
    m_type = Element::BOMB;
    
    m_xInit = fX;
    m_yInit = fY;
    m_x = fX;
    m_y = fY;
    
    m_bombRange = 1;
    
    m_maze->setCellElement(m_maze->getRowFromY(m_y), m_maze->getColFromX(m_x), this);
    
    m_detonated = false;
    
    // Define the timer which sets the puls frequency
    m_detonationCountdownTimer = new QTimer(this);
    m_detonationCountdownTimer->setInterval(nDetonationCountdown);
    m_detonationCountdownTimer->setSingleShot(true);
    m_detonationCountdownTimer->start();
    connect(m_detonationCountdownTimer, SIGNAL(timeout()), this, SLOT(detonate()));
    
    moveOnCenter();
}

Bomb::~Bomb()
{
    delete m_detonationCountdownTimer;
}

void Bomb::goUp()
{
}

void Bomb::goDown()
{
}

void Bomb::goRight()
{
}

void Bomb::goLeft()
{
}

void Bomb::updateMove()
{
}

void Bomb::move()
{
    // Move the Bomb
    m_x += m_xSpeed;
    m_y += m_ySpeed;
    emit(moved(m_x, m_y));
}

qreal Bomb::getXSpeed() const
{
    return m_xSpeed;
}

qreal Bomb::getYSpeed() const
{
    return m_ySpeed;
}

qreal Bomb::getSpeed()
{
    return m_speed;
}

void Bomb::setXSpeed(qreal p_xSpeed)
{
    m_xSpeed = p_xSpeed;
}

void Bomb::setYSpeed(qreal p_ySpeed) 
{
    m_ySpeed = p_ySpeed;
}

Cell Bomb::getNextCell()
{
    Cell nextCell;
    // Get the current cell coordinates from the character coordinates
    int curCellRow = m_maze->getRowFromY(m_y);
    int curCellCol = m_maze->getColFromX(m_x);

    // Get the next cell function of the character direction
    if (m_xSpeed > 0) {
        nextCell = m_maze->getCell(curCellRow, curCellCol + 1);
    } else if (m_xSpeed < 0) {
        nextCell = m_maze->getCell(curCellRow, curCellCol - 1);
    } else if (m_ySpeed > 0) {
        nextCell = m_maze->getCell(curCellRow + 1, curCellCol);
    } else if (m_ySpeed < 0) {
        nextCell = m_maze->getCell(curCellRow - 1, curCellCol);
    }

    return nextCell;
}

bool Bomb::onCenter()
{
    // Get the current cell center coordinates
    qreal centerX = (m_maze->getColFromX(m_x) + 0.5) * Cell::SIZE;
    qreal centerY = (m_maze->getRowFromY(m_y) + 0.5) * Cell::SIZE;
    bool willGoPast = false;

    // Will the character go past the center of the cell it's on ?
    // If goes right
    if (m_xSpeed > 0) {
        willGoPast = (m_x <= centerX && m_x + m_xSpeed >= centerX);
    }
    // If goes left
    else if (m_xSpeed < 0) {
        willGoPast = (m_x >= centerX && m_x + m_xSpeed <= centerX);
    }
    // If goes down
    else if (m_ySpeed > 0) {
        willGoPast = (m_y <= centerY && m_y + m_ySpeed >= centerY);
    }
    // If goes up
    else if (m_ySpeed < 0) {
        willGoPast = (m_y >= centerY && m_y + m_ySpeed <= centerY);
    }
    // If does not move
    else {
        willGoPast = (m_x == centerX && m_y == centerY);
    }

    return willGoPast;
}

void Bomb::moveOnCenter()
{
    setX((m_maze->getColFromX(m_x) + 0.5) * Cell::SIZE);
    setY((m_maze->getRowFromY(m_y) + 0.5) * Cell::SIZE);
}

int Bomb::bombRange()
{
    return m_bombRange;
}

void Bomb::setBombRange(int bombRange)
{
    m_bombRange = bombRange;
}

void Bomb::detonate()
{
    if(!m_detonated)
    {
        m_detonated = true;
        emit bombDetonated(this);
    }
}

bool Bomb::isDetonated()
{
    return m_detonated;
}

void Bomb::setDetonationCountdown(int nDetonationTimeout)
{
    if(m_detonationCountdownTimer->interval() > nDetonationTimeout)
    {
        m_detonationCountdownTimer->setInterval(nDetonationTimeout);
    }
}

void Bomb::slot_detonationCompleted()
{
    //TODO: call from game
    m_maze->removeCellElement(m_maze->getRowFromY(m_y), m_maze->getColFromX(m_x), this);
}
