/*
 * Copyright 2009 Mathias Kraus <k.hias@gmx.de>
 * Copyright 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
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


#include "player.h"
#include "bonus.h"
#include "arena.h"

#include <QKeyEvent>
#include <kdebug.h>

#ifdef GRANATIER_USE_GLUON
#include <KDE/KALEngine>
#include <KDE/KALBuffer>
#include <KDE/KALSound>
#endif

#include <cmath>

Player::Player(qreal p_x, qreal p_y, const QString& p_graphicsPath, const QString& p_playerName, Arena* p_arena) : Character(p_x, p_y, p_arena)
{
    m_type = Element::PLAYER;
    m_graphicsPath = p_graphicsPath;
    m_playerName = p_playerName;
    
    m_points = 0;
    
    resurrect();
    
    m_key.moveLeft = Qt::Key_Left;
    m_key.moveRight = Qt::Key_Right;
    m_key.moveUp = Qt::Key_Up;
    m_key.moveDown = Qt::Key_Down;
    m_key.dropBomb = Qt::Key_Return;
    
    #ifdef GRANATIER_USE_GLUON
    m_soundDie = NULL;
    m_soundWilhelmScream = NULL;
    #endif
}

Player::~Player()
{
    #ifdef GRANATIER_USE_GLUON
    if(m_soundDie)
    {
        delete m_soundDie;
    }
    if(m_soundWilhelmScream)
    {
        delete m_soundWilhelmScream;
    }
    #endif
}

void Player::setShortcuts(const Shortcuts keys)
{
    m_key = keys;
}

QString Player::getGraphicsPath() const
{
    return m_graphicsPath;
}

QString Player::getPlayerName() const
{
    return m_playerName;
}

void Player::init()
{
    goRight();
    updateDirection();
    stopMoving();
    // Stop animation
    emit stopped();
}

void Player::goUp()
{
    m_askedXSpeed = 0;
    m_askedYSpeed = -m_speed;
}

void Player::goDown()
{
    m_askedXSpeed = 0;
    m_askedYSpeed = m_speed;
}

void Player::goRight()
{
    m_askedXSpeed = m_speed;
    m_askedYSpeed = 0;
}

void Player::goLeft()
{
    m_askedXSpeed = -m_speed;
    m_askedYSpeed = 0;
}

void Player::updateDirection()
{
    setXSpeed(m_askedXSpeed);
    setYSpeed(m_askedYSpeed);
    m_askedXSpeed = 0;
    m_askedYSpeed = 0;
    // Signal to the player item that the direction changed
    emit directionChanged();
}

void Player::updateMove()
{
    //check if there is a hurdle in the way
    if(m_askedXSpeed != 0 || m_xSpeed != 0 || m_askedYSpeed != 0 || m_ySpeed != 0)
    {
        int xDirection = 0;                 //x-direction: -1: move left; 0: not moving; 1: move right
        int yDirection = 0;                 //y-direction: -1: move up; 0: not moving; 1: move down
        int straightDirection = 0;          //straight direction: -1: backward; 1:foreward; while foreward is right for moving in x-direction and down for y-direction
        qreal deltaStraightMove = 0;        //the move in straight direction
        qreal deltaPerpendicularMove = 0;   //the move in perpendicular direction; e.g. the player is not in cell center and will collide with a wall in the cell above, so the player has to be moved to the cell center
        qreal deltaAskedMove;               //how far to move; positive for right/down move and negative for left/up
        qreal deltaStraightCellCorner;      //move in x-direction: the x-pos from the top left cell corner; move in y-direction: the y-pos from the top left cell corner
        qreal deltaPerpendicularCellCorner; //move in x-direction: the y-pos from the top left cell corner; move in y-direction: the x-pos from the top left cell corner
        qreal deltaStraightCellCenter;      //distance to the cell center in moving direction; positive if left/up from cell center, negative if right/down
        qreal deltaPerpendicularCellCenter; //distance to the cell center perpendicular to moving direction; positive if up/left from cell center, negative if down/right
        bool bMoveWithinNextCellCenter = false;     //move is completed without exceeding the cell center
        int cellCol;
        int cellRow;
        
        // Get the current cell coordinates from the character coordinates
        int curCellRow = m_arena->getRowFromY(m_y);
        int curCellCol = m_arena->getColFromX(m_x);
        
        //set variables for right/left move
        if(m_askedXSpeed != 0 || m_xSpeed != 0)
        {
            //how far to move
            deltaAskedMove = (m_askedXSpeed != 0 ? m_askedXSpeed : m_xSpeed);
            
            //direction to move
            xDirection = sign(deltaAskedMove);
            straightDirection = xDirection;
            
            deltaStraightCellCorner = m_x - curCellCol * Cell::SIZE;
            deltaPerpendicularCellCorner = m_y - curCellRow * Cell::SIZE;
        }
        else        //set variables for down/up move
        {
            //how far to move
            deltaAskedMove = (m_askedYSpeed != 0 ? m_askedYSpeed : m_ySpeed);
            
            //direction to move
            yDirection = sign(deltaAskedMove);
            straightDirection = yDirection;
            
            deltaStraightCellCorner = m_y - curCellRow * Cell::SIZE;
            deltaPerpendicularCellCorner = m_x - curCellCol * Cell::SIZE;
        }
        
        //how far to current cell center
        deltaStraightCellCenter = Cell::SIZE/2 - deltaStraightCellCorner;
        deltaPerpendicularCellCenter = Cell::SIZE/2 - deltaPerpendicularCellCorner;
        
        //check if the move exceeds a cell center
        if(straightDirection*deltaStraightCellCenter >= 0)
        {
            if(fabs(deltaAskedMove) <= fabs(deltaStraightCellCenter))
            {
                bMoveWithinNextCellCenter = true;
            }
        }
        else if(fabs(deltaAskedMove) + fabs(deltaStraightCellCenter) <= Cell::SIZE)
        {
            bMoveWithinNextCellCenter = true;
        }
        
        //the move is within two cell centers
        if(bMoveWithinNextCellCenter)
        {
            deltaStraightMove += deltaAskedMove;
            //move to perpendicular center if needed
            if(deltaPerpendicularCellCenter != 0 && (straightDirection * deltaStraightCellCenter) < 0)  //not in perpendicular center and entering a new cell
            {
                if(fabs(deltaPerpendicularCellCenter) > Cell::SIZE/2 - fabs(deltaStraightMove - deltaStraightCellCenter))   //check if it already can collide with a hurdle
                {
                    cellRow = curCellRow + yDirection - fabs(xDirection)*signZeroPositive(deltaPerpendicularCellCenter);
                    cellCol = curCellCol + xDirection - fabs(yDirection)*signZeroPositive(deltaPerpendicularCellCenter);
                    if(!m_arena->getCell(cellRow, cellCol).isWalkable())
                    {
                        deltaPerpendicularMove = deltaPerpendicularCellCenter + signZeroPositive(deltaPerpendicularCellCenter) * (fabs(deltaStraightMove - deltaStraightCellCenter) - Cell::SIZE/2);
                        if(fabs(deltaPerpendicularMove) > fabs(deltaPerpendicularCellCenter))       //check if moved over perpendicular center
                        {
                            deltaPerpendicularMove = deltaPerpendicularCellCenter;
                        }
                    }
                }
            }
        }
        else    //the move exceeds a cell center
        {
            //at first move to the cell center
            deltaStraightMove += deltaStraightCellCenter;
            deltaAskedMove -= deltaStraightCellCenter;
            if(straightDirection * deltaStraightCellCenter < 0)     //the cell center to move is in the next cell
            {
                deltaStraightMove += straightDirection * Cell::SIZE;
                deltaAskedMove -= straightDirection * Cell::SIZE;

                //move to perpendicular center if needed
                if(deltaPerpendicularCellCenter != 0)
                {
                    cellRow = curCellRow + yDirection - fabs(xDirection)*signZeroPositive(deltaPerpendicularCellCenter);
                    cellCol = curCellCol + xDirection - fabs(yDirection)*signZeroPositive(deltaPerpendicularCellCenter);
                    if(!m_arena->getCell(cellRow, cellCol).isWalkable())
                    {
                        deltaPerpendicularMove = deltaPerpendicularCellCenter;
                    }
                }
                
                //update the current cell and row
                curCellCol += xDirection;
                curCellRow += yDirection;
            }
            while(fabs(deltaAskedMove) > 0)     //complete the move
            {
                if(m_arena->getCell(curCellRow + yDirection, curCellCol + xDirection).isWalkable())     //check if next cell is walkable
                {
                    if(fabs(deltaAskedMove) > Cell::SIZE)   //move to next cell center if the remaining move exceeds a cell center
                    {
                        deltaStraightMove += straightDirection * Cell::SIZE;
                        deltaAskedMove -= straightDirection * Cell::SIZE;
                        //move to perpendicular center if needed
                        if(deltaPerpendicularCellCenter != 0)
                        {
                            cellRow = curCellRow + yDirection - fabs(xDirection)*signZeroPositive(deltaPerpendicularCellCenter);
                            cellCol = curCellCol + xDirection - fabs(yDirection)*signZeroPositive(deltaPerpendicularCellCenter);
                            if(!m_arena->getCell(cellRow, cellCol).isWalkable())
                            {
                                deltaPerpendicularMove = deltaPerpendicularCellCenter;
                            }
                        }
                    }
                    else
                    {
                        deltaStraightMove += deltaAskedMove;
                        //move to perpendicular center if needed
                        if(deltaPerpendicularMove != deltaPerpendicularCellCenter && fabs(deltaPerpendicularCellCenter) > (Cell::SIZE/2 - fabs(deltaStraightMove - deltaStraightCellCenter)))   //check if it is in or already moved to perpendicular center and if it already can collide with a hurdle ***TODO: it seems to be wrong to use deltaStraightMove here, because ist could be greater than Cell::SIZE
                        {
                            cellRow = curCellRow + yDirection - fabs(xDirection)*signZeroPositive(deltaPerpendicularCellCenter);
                            cellCol = curCellCol + xDirection - fabs(yDirection)*signZeroPositive(deltaPerpendicularCellCenter);
                            if(!m_arena->getCell(cellRow, cellCol).isWalkable())
                            {
                                deltaPerpendicularMove = signZeroPositive(deltaPerpendicularCellCenter) * fabs(deltaAskedMove);
                                if(fabs(deltaPerpendicularMove) > fabs(deltaPerpendicularCellCenter))
                                {
                                    deltaPerpendicularMove = deltaPerpendicularCellCenter;      //check if moved over perpendicular center
                                }
                            }
                        }
                        deltaAskedMove = 0;
                    }
                    //update the current cell and row
                    curCellCol += xDirection;
                    curCellRow += yDirection;
                }
                else    //there is a hurdle in the next cell, so don't stop moving
                {
                    deltaAskedMove = 0;
                }
            }
        }
        
        // Update the direction
        if(m_askedXSpeed != 0 || m_askedYSpeed != 0)
        {
            updateDirection();
        }
        
        // Move the player
        if(xDirection != 0)
        {
            move(m_x + deltaStraightMove, m_y + deltaPerpendicularMove);
        }
        else
        {
            move(m_x + deltaPerpendicularMove, m_y + deltaStraightMove);
        }
    }
}

void Player::move(qreal x, qreal y)
{
    // Move the Character
    m_x = x;
    m_y = y;
    emit moved(m_x, m_y);
}

void Player::addBonus(Bonus* p_bonus)
{
    int bonusType = p_bonus->getBonusType();
    if(bonusType == Bonus::SPEED)
    {
        m_speed++;
        if(m_speed > m_maxSpeed)
        {
            m_speed = m_maxSpeed;
        }
    }
    else if(bonusType == Bonus::RANGE)
    {
        m_bombRange++;
        if(m_bombRange > 10)
        {
            m_bombRange = 10;
        }
    }
    else if(bonusType == Bonus::BOMB)
    {
        m_maxBombArmory++;
        if(m_maxBombArmory > 10)
        {
            m_maxBombArmory = 10;
        }
        m_bombArmory++;
        if(m_bombArmory > m_maxBombArmory)
        {
            m_bombArmory = m_maxBombArmory;
        }
    }
}

#ifdef GRANATIER_USE_GLUON
void Player::setSoundDie(KALBuffer* buffer)
{
    if(m_soundDie)
    {
        delete m_soundDie;
        m_soundDie = NULL;
    }
    
    if(buffer)
    {
        m_soundDie = new KALSound(buffer, KALEngine::instance());
    }
}

void Player::setSoundWilhelmScream(KALBuffer* buffer)
{
    if(m_soundWilhelmScream)
    {
        delete m_soundWilhelmScream;
        m_soundWilhelmScream = NULL;
    }
    
    if(buffer)
    {
        m_soundWilhelmScream = new KALSound(buffer, KALEngine::instance());
    }
}
#endif

void Player::die()
{
    #ifdef GRANATIER_USE_GLUON
    if(m_imageId == "player1" && m_soundWilhelmScream != NULL && m_soundWilhelmScream->elapsedTime() == 0)
    {
        m_soundWilhelmScream->play();
    }
    else if(m_imageId != "player1" && m_soundDie != NULL && m_soundDie->elapsedTime() == 0)
    {
        m_soundDie->play();
    }
    #endif
    
    if(!m_death)
    {
        m_death = true;
        emit dying(this);
    }
}

bool Player::isAlive()
{
    return !m_death;
}

void Player::resurrect()
{
    m_death = false;
    m_maxSpeed = 10;
    m_speed = 2;
    m_bombRange = 1;
    m_maxBombArmory = 1;
    m_bombArmory = m_maxBombArmory;
}

int Player::points()
{
    return m_points;
}

void Player::addPoint()
{
    m_points++;
}

void Player::emitGameUpdated()
{
    emit gameUpdated();
}

qreal Player::getAskedXSpeed() const
{
    return m_askedXSpeed;
}

qreal Player::getAskedYSpeed() const
{
    return m_askedYSpeed;
}

Cell Player::getAskedNextCell()
{
    // Get the current cell coordinates from the character coordinates
    int curCellRow = m_arena->getRowFromY(m_y);
    int curCellCol = m_arena->getColFromX(m_x);
    Cell nextCell;

    // Get the next cell function of the character asked direction
    if (m_askedXSpeed > 0) {
        nextCell = m_arena->getCell(curCellRow, curCellCol + 1);
    } else if (m_askedXSpeed < 0) {
        nextCell = m_arena->getCell(curCellRow, curCellCol - 1);
    } else if (m_askedYSpeed > 0) {
        nextCell = m_arena->getCell(curCellRow + 1, curCellCol);
    } else if (m_askedYSpeed < 0) {
        nextCell = m_arena->getCell(curCellRow - 1, curCellCol);
    }

    return nextCell;
}

int Player::getBombRange() const
{
    return m_bombRange;
}

void Player::decrementBombArmory()
{
    m_bombArmory--;
    if(m_bombArmory < 0)
    {
        m_bombArmory = 0;
    }
}

void Player::slot_refillBombArmory()
{
    m_bombArmory++;
    if(m_bombArmory > m_maxBombArmory)
    {
        m_bombArmory = m_maxBombArmory;
    }
}

void Player::stopMoving()
{
    setXSpeed(0);
    setYSpeed(0);
    m_askedXSpeed = 0;
    m_askedYSpeed = 0;
    emit stopped();
}

void Player::keyPressed(QKeyEvent* keyEvent)
{
    if(m_death)
    {
        return;
    }
    
    int key = keyEvent->key();

    if(key == m_key.moveLeft || key == m_key.moveRight || key == m_key.moveUp || key == m_key.moveDown || key == m_key.dropBomb)
    {
        keyEvent->accept();
        if(keyEvent->isAutoRepeat())
        {
            return;
        }
    }
    else
    {
        return;
    }

    if(key == m_key.moveLeft)
    {
        goLeft();
        updateDirection();
    }
    else if(key == m_key.moveRight)
    {
        goRight();
        updateDirection();
    }
    else if(key == m_key.moveUp)
    {
        goUp();
        updateDirection();
    }
    else if(key == m_key.moveDown)
    {
        goDown();
        updateDirection();
    }
    else if(key == m_key.dropBomb && m_bombArmory > 0)
    {
        emit bombDropped(this, m_x, m_y);
    }
    
}

void Player::keyReleased(QKeyEvent* keyEvent)
{
    int key = keyEvent->key();

    if(key == m_key.moveLeft || key == m_key.moveRight || key == m_key.moveUp || key == m_key.moveDown || key == m_key.dropBomb)
    {
        keyEvent->accept();
        if(keyEvent->isAutoRepeat())
        {
            return;
        }
    }
    else
    {
        return;
    }
    
    if(key == m_key.moveLeft && m_xSpeed < 0)
    {
        setXSpeed(0);
    }
    else if(key == m_key.moveRight && m_xSpeed > 0)
    {
        setXSpeed(0);
    }
    else if(key == m_key.moveUp && m_ySpeed < 0)
    {
        setYSpeed(0);
    }
    else if(key == m_key.moveDown && m_ySpeed > 0)
    {
        setYSpeed(0);
    }
    else if(key == m_key.dropBomb)
    {
        //emit bomb(this);
    }
    
    if(m_xSpeed == 0 && m_ySpeed == 0 && m_askedXSpeed == 0 && m_askedYSpeed == 0) stopMoving();
}

int Player::signZeroPositive(const qreal value)
{
    return (value >= 0 ? 1 : -1);
}

int Player::sign(const qreal value)
{
    if(value == 0)
    {
        return 0;
    }
    return (value > 0 ? 1 : -1);
}
