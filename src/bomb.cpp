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
    m_thrown = false;
    m_stopOnCenter = false;
    m_falling = false;
    
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
    //check if the bomb is on an arrow, mortar or hole
    if(m_mortarState == -1 && m_xSpeed == 0 && m_ySpeed == 0)
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
                m_type = Element::NONE;
                m_arena->removeCellElement(currentRow, currentCol, this);
                emit mortar(m_mortarState);
                break;
            case Cell::HOLE:
                if(!m_falling)
                {
                    m_falling = true;
                    m_type = Element::NONE;
                    m_detonationCountdownTimer->stop();
                    emit falling();
                    emit releaseBombArmory();
                }
                break;
            default:
                break;
        }
    }
    
    if(m_xSpeed != 0 || m_ySpeed != 0)
    {
        bool bOnCenter = false;
        int xDirection = 0;
        int xDeltaCenter = Cell::SIZE/2 - (m_x - currentCol * Cell::SIZE);
        if (m_xSpeed > 0)
        {
            xDirection = 1;
        }
        else if (m_xSpeed < 0)
        {
            xDirection = -1;
        }
        int yDirection = 0;
        int yDeltaCenter = Cell::SIZE/2 - (m_y - currentRow * Cell::SIZE);
        if (m_ySpeed > 0)
        {
            yDirection = 1;
        }
        else if (m_ySpeed < 0)
        {
            yDirection = -1;
        }
        
        if((xDirection != 0 && xDeltaCenter == 0) || (yDirection != 0 && yDeltaCenter == 0))
        {
            bOnCenter = true;
        }
        
        int newRow = m_arena->getRowFromY(m_y + m_ySpeed);
        int newCol = m_arena->getColFromX(m_x + m_xSpeed);
        int nextRow;
        int nextCol;
        if(xDirection > 0 || yDirection > 0)
        {
            nextRow = m_arena->getRowFromY(m_y + yDirection * Cell::SIZE/2); //this is needed because the bombs won't move if they are coming from the top, hit an up arrow and there is a hurdle below the arrow
            nextCol = m_arena->getColFromX(m_x + xDirection * Cell::SIZE/2);
        }
        else
        {
            nextRow = m_arena->getRowFromY(m_y + m_ySpeed + yDirection * Cell::SIZE/2);
            nextCol = m_arena->getColFromX(m_x + m_xSpeed + xDirection * Cell::SIZE/2);
        }
        
        //at first, check if move over cell center or currently on cell center
        if((bOnCenter || (xDirection * xDeltaCenter < 0 && xDirection * (m_xSpeed + xDeltaCenter) >= 0) || (yDirection * yDeltaCenter < 0 && yDirection * (m_ySpeed + yDeltaCenter) >= 0)) && m_mortarState == -1)
        {
            bool bIsMortar = false;
            bool bIsNewDirection = false;
            bool bIsHurdleCurrentCell = false;
            bool bIsHurdleNextCell = false;
            
            switch (m_arena->getCell(currentRow, currentCol).getType())
            {
                case Cell::ARROWUP:
                    if(yDirection != -1)
                    {
                        bIsNewDirection = true;
                    }
                    break;
                case Cell::ARROWRIGHT:
                    if(xDirection != 1)
                    {
                        bIsNewDirection = true;
                    }
                    break;
                case Cell::ARROWDOWN:
                    if(yDirection != 1)
                    {
                        bIsNewDirection = true;
                    }
                    break;
                case Cell::ARROWLEFT:
                    if(xDirection != -1)
                    {
                        bIsNewDirection = true;
                    }
                    break;
                case Cell::BOMBMORTAR:
                    bIsMortar = true;
                    break;
                case Cell::HOLE:
                    m_stopOnCenter = true;
                    break;
                default:
                    break;
            }
            
            
            bIsHurdleCurrentCell = !(m_arena->getCell(currentRow, currentCol).isWalkable(this));
            bIsHurdleNextCell = !(m_arena->getCell(nextRow, nextCol).isWalkable(this));
            
            //if two bombs move towards them, stop them and move them to the next cell center
            if((bIsHurdleCurrentCell || bIsHurdleNextCell) && !m_stopOnCenter)
            {
                if(bOnCenter)
                {
                    setXSpeed(0);
                    setYSpeed(0);
                }
                else
                {
                    m_stopOnCenter = true;
                    setXSpeed(-m_xSpeed);
                    setYSpeed(-m_ySpeed);
                }
            }
            
            //stop at cell center if direction change or bomb mortar in current cell
            else if(bIsMortar || bIsNewDirection || m_stopOnCenter)
            {
                move((currentCol+0.5) * Cell::SIZE, (currentRow+0.5) * Cell::SIZE);
                setXSpeed(0);
                setYSpeed(0);
                m_stopOnCenter = false;
            }
            else
            {
                if((newRow != currentRow || newCol != currentCol) && m_mortarState == -1)
                {
                    m_arena->removeCellElement(currentRow, currentCol, this);
                    m_arena->setCellElement(newRow, newCol, this);
                }
                move(m_x + m_xSpeed, m_y + m_ySpeed);
            }
        }
        else
        {
            //if two bombs move towards them, stop them and move them to the next cell center
            if((!(m_arena->getCell(nextRow, nextCol).isWalkable(this)) || !(m_arena->getCell(currentRow, currentCol).isWalkable(this))) && m_mortarState == -1 && !m_stopOnCenter)
            {
                if(bOnCenter)
                {
                    setXSpeed(0);
                    setYSpeed(0);
                }
                else
                {
                    m_stopOnCenter = true;
                    setXSpeed(-m_xSpeed);
                    setYSpeed(-m_ySpeed);
                }
            }
            else
            {
                if((newRow != currentRow || newCol != currentCol) && m_mortarState == -1)
                {
                    m_arena->removeCellElement(currentRow, currentCol, this);
                    m_arena->setCellElement(newRow, newCol, this);
                }
                move(m_x + m_xSpeed, m_y + m_ySpeed);
            }
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
    qreal fSpeed = 2 * Cell::SIZE / 32.0;//800ms with 40FPS are 32 frames
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
    
    m_thrown = true;
    m_mortarState = 1;
    updateMortarState();
}

void Bomb::setKicked(int nDirection)
{
    switch(nDirection)
    {
        case Element::NORTH:
            setXSpeed(0);
            setYSpeed(-5);
            break;
        case Element::EAST:
            setXSpeed(5);
            setYSpeed(0);
            break;
        case Element::SOUTH:
            setXSpeed(0);
            setYSpeed(5);
            break;
        case Element::WEST:
            setXSpeed(-5);
            setYSpeed(0);
            break;
    }
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
        emit releaseBombArmory();
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
              int curCellRow = m_arena->getRowFromY(m_y);
              int curCellCol = m_arena->getColFromX(m_x);
              
              m_thrown = false;
              m_type = Element::NONE;
              m_arena->removeCellElement(curCellRow, curCellCol, this);
              
              m_mortarState++;
              emit mortar(m_mortarState);
              m_mortarTimer->start(50);
          }
          
          break;
      case 1:
          {
              int curCellRow = m_arena->getRowFromY(m_y);
              int curCellCol = m_arena->getColFromX(m_x);
              
              if(!m_thrown)
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
                          bFound = true;
                      }
                  }
                  while (!bFound);
                  
                  setXSpeed((nCol - curCellCol) * Cell::SIZE / 32.0);//800ms with 40FPS are 32 frames
                  setYSpeed((nRow - curCellRow) * Cell::SIZE / 32.0);//800ms with 40FPS are 32 frames
              }
              
              m_type = Element::NONE;
              m_arena->removeCellElement(curCellRow, curCellCol, this);
              
              m_mortarState++;
              emit mortar(m_mortarState);
              m_mortarTimer->start(800);
          }
          break;
      case 2:
          {
              int curCellRow = m_arena->getRowFromY(m_y);
              int curCellCol = m_arena->getColFromX(m_x);
              m_type = Element::BOMB;
              if(!(m_arena->getCell(curCellRow, curCellCol).getElement()))
              {
                  m_arena->setCellElement(curCellRow, curCellCol, this);
              }
              
              m_mortarState++;
              emit mortar(m_mortarState);
              m_mortarState = -1;
              if(m_detonationCountdownTimer)
              {
                  setXSpeed(0);
                  setYSpeed(0);
                  if(!m_thrown)
                  {
                      initDetonation(m_bombID, 40);
                  }
                  else
                  {
                      m_detonationCountdownTimer->setInterval(2000);
                      m_detonationCountdownTimer->start();
                  }
              }
          }
          break;
    }
}
