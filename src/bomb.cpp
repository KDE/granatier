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
#include "cell.h"
#include "arena.h"
#include "settings.h"

#include <QTimer>

Bomb::Bomb(qreal fX, qreal fY, Arena* p_arena, int nBombID, int nDetonationCountdown) : Element(fX, fY, p_arena), m_xSpeed(0), m_ySpeed(0)
{
    m_type = Element::BOMB;
    
    m_xInit = fX;
    m_yInit = fY;
    m_x = fX;
    m_y = fY;
    
    m_bombPower = 1;
    
    int currentRow = m_arena->getRowFromY(m_y);
    int currentCol = m_arena->getColFromX(m_x);
    m_arena->setCellElement(currentRow, currentCol, this);
    
    m_detonated = false;
    
    m_bombID = nBombID;
    m_explosionID = nBombID;
    
    // Define the timer which sets the puls frequency
    m_detonationCountdownTimer = new QTimer(this);
    m_detonationCountdownTimer->setInterval(nDetonationCountdown);
    m_detonationCountdownTimer->setSingleShot(true);
    m_detonationCountdownTimer->start();
    connect(m_detonationCountdownTimer, SIGNAL(timeout()), this, SLOT(detonate()));
    
    m_mortarTimer = 0;
    m_mortarState = -1;
    
    moveOnCenter();
}

Bomb::~Bomb()
{
    delete m_detonationCountdownTimer;
    delete m_mortarTimer;
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
    if(m_detonated)
    {
        return;
    }
    
    int currentRow = m_arena->getRowFromY(m_y);
    int currentCol = m_arena->getColFromX(m_x);
    //check if the bomb is on an arrow or mortar
    if(m_mortarState == -1)
    {
        switch (m_arena->getCell(currentRow, currentCol).getType())
        {
            case Cell::ARROWUP:
                setYSpeed(-5);
                break;
            case Cell::ARROWRIGHT:
                setXSpeed(5);
                break;
            case Cell::ARROWDOWN:
                setYSpeed(5);
                break;
            case Cell::ARROWLEFT:
                setXSpeed(-5);
                break;
            case Cell::BOMBMORTAR:
                m_mortarTimer = new QTimer;
                m_mortarTimer->setSingleShot(true);
                m_mortarTimer->setInterval(1500);
                m_mortarTimer->start();
                m_detonationCountdownTimer->stop();
                connect(m_mortarTimer, SIGNAL(timeout()), this, SLOT(updateMortarState()));
                m_mortarState = 0;
                setXSpeed(0);
                setYSpeed(0);
                emit mortar(m_mortarState);
            default:
                break;
        }
    }
    
    if(m_xSpeed != 0 || m_ySpeed != 0)
    {
        int xDirection = 0;
        if (m_xSpeed > 0)
        {
            xDirection = 1;
        }
        else if (m_xSpeed < 0)
        {
            xDirection = -1;
        }
        int yDirection = 0;
        if (m_ySpeed > 0)
        {
            yDirection = 1;
        }
        else if (m_ySpeed < 0)
        {
            yDirection = -1;
        }
        int newRow = m_arena->getRowFromY(m_y + m_ySpeed);
        int newCol = m_arena->getColFromX(m_x + m_xSpeed);
        int nextRow = m_arena->getRowFromY(m_y + m_ySpeed + yDirection * Cell::SIZE/2);
        int nextCol = m_arena->getColFromX(m_x + m_xSpeed + xDirection * Cell::SIZE/2);
        
        if((newRow != currentRow || newCol != currentCol) && m_mortarState == -1)
        {
            //move to next cell
            if(m_arena->getCell(newRow, newCol).isWalkable())
            {
                m_arena->removeCellElement(currentRow, currentCol, this);
                move(m_x + m_xSpeed, m_y + m_ySpeed);
                m_arena->setCellElement(newRow, newCol, this);
            }
            else    //move to last cell center
            {
                move((currentCol+0.5) * Cell::SIZE, (currentRow+0.5) * Cell::SIZE);
                setXSpeed(0);
                setYSpeed(0);
            }
        }
        else
        {
            move(m_x + m_xSpeed, m_y + m_ySpeed);
        }
    }
}

void Bomb::move(qreal x, qreal y)
{
    // Move the Bomb
    m_x = x;
    m_y = y;
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

void Bomb::setThrown(int nDirection)
{
    if(!m_mortarTimer)
    {
        m_mortarTimer = new QTimer;
        m_mortarTimer->setSingleShot(true);
        if(m_detonationCountdownTimer)
        {
            m_detonationCountdownTimer->stop();
        }
        connect(m_mortarTimer, SIGNAL(timeout()), this, SLOT(updateMortarState()));
    }
    qreal fSpeed = 2 * Cell::SIZE / (1380 / 40/*Game::FPS*/);
    switch(nDirection)
    {
        case Element::NORTH:
            setXSpeed(0);
            setYSpeed(-fSpeed);
            break;
        case Element::EAST:
            setXSpeed(fSpeed);
            setYSpeed(0);
            break;
        case Element::SOUTH:
            setXSpeed(0);
            setYSpeed(fSpeed);
            break;
        case Element::WEST:
            setXSpeed(-fSpeed);
            setYSpeed(0);
            break;
    }
    
    m_mortarState = 1;
    updateMortarState();
}

Cell Bomb::getNextCell()
{
    Cell nextCell;
    // Get the current cell coordinates from the character coordinates
    int curCellRow = m_arena->getRowFromY(m_y);
    int curCellCol = m_arena->getColFromX(m_x);

    // Get the next cell function of the character direction
    if (m_xSpeed > 0) {
        nextCell = m_arena->getCell(curCellRow, curCellCol + 1);
    } else if (m_xSpeed < 0) {
        nextCell = m_arena->getCell(curCellRow, curCellCol - 1);
    } else if (m_ySpeed > 0) {
        nextCell = m_arena->getCell(curCellRow + 1, curCellCol);
    } else if (m_ySpeed < 0) {
        nextCell = m_arena->getCell(curCellRow - 1, curCellCol);
    }

    return nextCell;
}

bool Bomb::onCenter()
{
    // Get the current cell center coordinates
    qreal centerX = (m_arena->getColFromX(m_x) + 0.5) * Cell::SIZE;
    qreal centerY = (m_arena->getRowFromY(m_y) + 0.5) * Cell::SIZE;
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
    setX((m_arena->getColFromX(m_x) + 0.5) * Cell::SIZE);
    setY((m_arena->getRowFromY(m_y) + 0.5) * Cell::SIZE);
}

int Bomb::bombPower()
{
    return m_bombPower;
}

void Bomb::setBombPower(int bombPower)
{
    m_bombPower = bombPower;
}

void Bomb::pause()
{
    if(m_detonationCountdownTimer)
    {
        m_detonationCountdownTimer->stop();
    }
}

void Bomb::resume()
{
    if(m_detonationCountdownTimer)
    {
        m_detonationCountdownTimer->start();
    }
}

void Bomb::detonate()
{
    if(!m_detonated)
    {
        m_detonated = true;
        delete m_detonationCountdownTimer;
        m_detonationCountdownTimer = 0;
        emit bombDetonated(this);
    }
}

bool Bomb::isDetonated()
{
    return m_detonated;
}

int Bomb::explosionID()
{
    return m_explosionID;
}

void Bomb::initDetonation(int nBombID, int nDetonationTimeout)
{
    if(m_bombID == m_explosionID)
    {
        m_explosionID = nBombID;
    }
    
    if((!m_detonationCountdownTimer))
    {
        return;
    }
    
    if(m_detonationCountdownTimer->interval() > nDetonationTimeout)
    {
        m_detonationCountdownTimer->setInterval(nDetonationTimeout);
    }
    
    if(!(m_detonationCountdownTimer->isActive()))
    {
        m_detonationCountdownTimer->start();
    }
}

void Bomb::slot_detonationCompleted()
{
    //TODO: call from game
    m_arena->removeCellElement(m_arena->getRowFromY(m_y), m_arena->getColFromX(m_x), this);
}

void Bomb::updateMortarState()
{
    switch(m_mortarState)
    {
      case 0:
          {
              int nRow;
              int nCol;
              bool bFound = false;
              do
              {
                  nRow = m_arena->getNbRows() * (qrand()/1.0)/RAND_MAX;
                  nCol = m_arena->getNbColumns() * (qrand()/1.0)/RAND_MAX;
                  if(m_arena->getCell(nRow, nCol).getType() != Cell::WALL)
                  {
                      if(m_arena->getCell(nRow, nCol).getElement() == 0 || m_arena->getCell(nRow, nCol).getElement()->getType() != Element::BLOCK)
                      {
                          bFound = true;
                      }
                  }
              }
              while (!bFound);
              
              int curCellRow = m_arena->getRowFromY(m_y);
              int curCellCol = m_arena->getColFromX(m_x);
              
              setXSpeed((nCol - curCellCol) * Cell::SIZE / (1380 / 40/*Game::FPS*/));
              setYSpeed((nRow - curCellRow) * Cell::SIZE / (1380 / 40/*Game::FPS*/));
              
              m_mortarState++;
              emit mortar(m_mortarState);
              m_mortarTimer->start(50);
          }
          
          break;
      case 1:
          m_mortarState++;
          emit mortar(m_mortarState);
          m_mortarTimer->start(800);
          break;
      case 2:
          setXSpeed(0);
          setYSpeed(0);
          m_mortarState++;
          emit mortar(m_mortarState);
          m_mortarState = -1;
          if(m_detonationCountdownTimer)
          {
              initDetonation(m_bombID, 20);
          }
          break;
    }
}
