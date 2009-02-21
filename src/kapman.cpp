/*
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


#include "kapman.h"

#include <kdebug.h>
#include <KGameDifficulty>

#include <cmath>

const qreal Kapman::MAX_SPEED_RATIO = 1.5;

Kapman::Kapman(qreal p_x, qreal p_y, Maze* p_maze) : Character(p_x, p_y, p_maze) {
	m_type = Element::KAPMAN;
	m_maxSpeed = m_normalSpeed * MAX_SPEED_RATIO;
    
    m_key.moveLeft = Qt::Key_Left;
    m_key.moveRight = Qt::Key_Right;
    m_key.moveUp = Qt::Key_Up;
    m_key.moveDown = Qt::Key_Down;
    m_key.dropBomb = Qt::Key_Return;
}

Kapman::~Kapman() {

}

void Kapman::init() {
	goRight();
	updateDirection();
    stopMoving();
	// Stop animation
	emit(stopped());
}

void Kapman::goUp() {
	m_askedXSpeed = 0;
	m_askedYSpeed = -m_speed;
}

void Kapman::goDown() {
	m_askedXSpeed = 0;
	m_askedYSpeed = m_speed;
}

void Kapman::goRight() {
	m_askedXSpeed = m_speed;
	m_askedYSpeed = 0;
}

void Kapman::goLeft() {
	m_askedXSpeed = -m_speed;
	m_askedYSpeed = 0;
}

void Kapman::updateDirection() {
	setXSpeed(m_askedXSpeed);
	setYSpeed(m_askedYSpeed);
	m_askedXSpeed = 0;
	m_askedYSpeed = 0;
	// Signal to the kapman item that the direction changed
	emit(directionChanged());
}

void Kapman::updateMove()
{
    //check if there is a hurdle in the way
    if(m_askedXSpeed != 0 || m_xSpeed != 0 || m_askedYSpeed != 0 || m_ySpeed != 0)
    {
        int xDirection = 0;
        int yDirection = 0;
        int straightDirection = 0;
        qreal deltaStraightMove = 0;
        qreal deltaPerpendicularMove = 0;
        qreal deltaAskedMove;
        qreal deltaStraightCellCorner;      //if move in X, then X-pos
        qreal deltaPerpendicularCellCorner; //if move in X, then Y-pos
        qreal deltaStraightCellCenter;
        qreal deltaPerpendicularCellCenter;
        Cell nextCell;
        int cellCol;
        int cellRow;
        bool bMoveWithinNextCellCenter = false;
        
        // Get the current cell coordinates from the character coordinates
        int curCellRow = m_maze->getRowFromY(m_y);
        int curCellCol = m_maze->getColFromX(m_x);
        
        //**************************************************************
        //right/left move
        //**************************************************************
        if(m_askedXSpeed != 0 || m_xSpeed != 0)
        {
            //how far to move
            deltaAskedMove = (m_askedXSpeed != 0 ? m_askedXSpeed : m_xSpeed);
            
            //direction to move
            xDirection = sign(deltaAskedMove);
            straightDirection = xDirection;
            
            deltaStraightCellCorner = m_x - ((int) (m_x / Cell::SIZE)) * Cell::SIZE;
            deltaPerpendicularCellCorner = m_y - ((int) (m_y / Cell::SIZE)) * Cell::SIZE;
        }
        //**************************************************************
        //down/up move
        //**************************************************************
        else
        {
            //how far to move
            deltaAskedMove = (m_askedYSpeed != 0 ? m_askedYSpeed : m_ySpeed);
            
            //direction to move
            yDirection = sign(deltaAskedMove);
            straightDirection = yDirection;
            
            deltaStraightCellCorner = m_y - ((int) (m_y / Cell::SIZE)) * Cell::SIZE;
            deltaPerpendicularCellCorner = m_x - ((int) (m_x / Cell::SIZE)) * Cell::SIZE;
        }
        
        //how far to current cell center
        deltaStraightCellCenter = Cell::SIZE/2 - deltaStraightCellCorner;
        deltaPerpendicularCellCenter = Cell::SIZE/2 - deltaPerpendicularCellCorner;
        
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
        
        if(bMoveWithinNextCellCenter)
        {
            deltaStraightMove += deltaAskedMove;
            //move to y center if needed
            if(deltaPerpendicularCellCorner != Cell::SIZE/2 && (straightDirection * deltaStraightCellCenter) < 0)
            {
                if(fabs(deltaPerpendicularCellCenter) > Cell::SIZE/2 - fabs(deltaStraightMove - deltaStraightCellCenter))
                {
                    cellRow = curCellRow + yDirection - fabs(xDirection)*signZeroPositive(deltaPerpendicularCellCenter);
                    cellCol = curCellCol + xDirection - fabs(yDirection)*signZeroPositive(deltaPerpendicularCellCenter);
                    if(m_maze->getCell(cellRow, cellCol).getType() == Cell::WALL)
                    {
                        deltaPerpendicularMove = deltaPerpendicularCellCenter + signZeroPositive(deltaPerpendicularCellCenter) * (fabs(deltaStraightMove - deltaStraightCellCenter) - Cell::SIZE/2);
                        if(fabs(deltaPerpendicularMove) > fabs(deltaPerpendicularCellCenter))
                        {
                            deltaPerpendicularMove = deltaPerpendicularCellCenter;
                        }
                    }
                }
            }
        }
        else
        {
            deltaStraightMove += deltaStraightCellCenter;
            deltaAskedMove -= deltaStraightCellCenter;
            if(straightDirection * deltaStraightCellCenter < 0)
            {
                deltaStraightMove += straightDirection * Cell::SIZE;
                deltaAskedMove -= straightDirection * Cell::SIZE;

                //move to y center if needed
                cellRow = curCellRow + yDirection - fabs(xDirection)*signZeroPositive(deltaPerpendicularCellCenter);
                cellCol = curCellCol + xDirection - fabs(yDirection)*signZeroPositive(deltaPerpendicularCellCenter);
                if(deltaPerpendicularCellCorner != Cell::SIZE/2 && m_maze->getCell(cellRow, cellCol).getType() == Cell::WALL)
                {
                    deltaPerpendicularMove = deltaPerpendicularCellCenter;
                }
                curCellCol += xDirection;
                curCellRow += yDirection;
            }
            while(fabs(deltaAskedMove) > 0)
            {
                nextCell = m_maze->getCell(curCellRow + yDirection, curCellCol + xDirection);
                if(nextCell.getType() == Cell::GROUND)
                {
                    if(fabs(deltaAskedMove) > Cell::SIZE)
                    {
                        deltaStraightMove += straightDirection * Cell::SIZE;
                        deltaAskedMove -= straightDirection * Cell::SIZE;
                        //move to y center if needed
                        cellRow = curCellRow + yDirection - fabs(xDirection)*signZeroPositive(deltaPerpendicularCellCenter);
                        cellCol = curCellCol + xDirection - fabs(yDirection)*signZeroPositive(deltaPerpendicularCellCenter);
                        if(deltaPerpendicularCellCorner != Cell::SIZE/2 && m_maze->getCell(cellRow, cellCol).getType() == Cell::WALL)
                        {
                            deltaPerpendicularMove = deltaPerpendicularCellCenter;
                        }
                    }
                    else
                    {
                        deltaStraightMove += deltaAskedMove;
                        //move to y center if needed
                        if(deltaPerpendicularMove == 0 && deltaPerpendicularCellCorner != Cell::SIZE/2 && fabs(deltaPerpendicularCellCenter) > (Cell::SIZE/2 - fabs(deltaStraightMove - deltaStraightCellCenter)))
                        {
                            cellRow = curCellRow + yDirection - fabs(xDirection)*signZeroPositive(deltaPerpendicularCellCenter);
                            cellCol = curCellCol + xDirection - fabs(yDirection)*signZeroPositive(deltaPerpendicularCellCenter);
                            if(m_maze->getCell(cellRow, cellCol).getType() == Cell::WALL)
                            {
                                deltaPerpendicularMove = signZeroPositive(deltaPerpendicularCellCenter) * fabs(deltaAskedMove);
                                if(fabs(deltaPerpendicularMove) > fabs(deltaPerpendicularCellCenter))
                                {
                                    deltaPerpendicularMove = deltaPerpendicularCellCenter;
                                }
                            }
                        }
                        deltaAskedMove = 0;
                    }
                    curCellCol += xDirection;
                    curCellRow += yDirection;
                }
                else
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
        // Move the kapman
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

void Kapman::move(qreal x, qreal y) {
    // Move the Character
    m_x = x;
    m_y = y;
    emit(moved(m_x, m_y));
}

void Kapman::winPoints(Element* p_element) {
	// Emits a signal to the game
	emit(sWinPoints(p_element));
}

void Kapman::die() {
	emit(eaten());
}

void Kapman::emitGameUpdated() {
	emit(gameUpdated());
}

qreal Kapman::getAskedXSpeed() const {
	return m_askedXSpeed;
}

qreal Kapman::getAskedYSpeed() const {
	return m_askedYSpeed;
}

Cell Kapman::getAskedNextCell() {
	// Get the current cell coordinates from the character coordinates
	int curCellRow = m_maze->getRowFromY(m_y);
	int curCellCol = m_maze->getColFromX(m_x);
	Cell nextCell;

	// Get the next cell function of the character asked direction
	if (m_askedXSpeed > 0) {
		nextCell = m_maze->getCell(curCellRow, curCellCol + 1);
	} else if (m_askedXSpeed < 0) {
		nextCell = m_maze->getCell(curCellRow, curCellCol - 1);
	} else if (m_askedYSpeed > 0) {
		nextCell = m_maze->getCell(curCellRow + 1, curCellCol);
	} else if (m_askedYSpeed < 0) {
		nextCell = m_maze->getCell(curCellRow - 1, curCellCol);
	}

	return nextCell;
}

void Kapman::stopMoving() {
	setXSpeed(0);
	setYSpeed(0);
	m_askedXSpeed = 0;
	m_askedYSpeed = 0;
	emit(stopped());
}

void Kapman::initSpeedInc() {
	// Kapman speed increase when level up
	if(KGameDifficulty::level() == KGameDifficulty::Easy) {
		m_speedIncrease = (Character::LOW_SPEED_INC / 2);
	}
	if(KGameDifficulty::level() == KGameDifficulty::Medium) {
		m_speedIncrease = (Character::MEDIUM_SPEED_INC / 2);
	}
	if(KGameDifficulty::level() == KGameDifficulty::Hard) {
		m_speedIncrease = (Character::HIGH_SPEED_INC / 2);
	}
}

void Kapman::keyPressed(QKeyEvent* keyEvent)
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

    if(key == m_key.moveLeft)
    {
        goLeft();
    }
    else if(key == m_key.moveRight)
    {
        goRight();
    }
    else if(key == m_key.moveUp)
    {
        goUp();
    }
    else if(key == m_key.moveDown)
    {
        goDown();
    }
    else if(key == m_key.dropBomb)
    {
        emit bombDropped(m_x, m_y);
    }
    
}

void Kapman::keyReleased(QKeyEvent* keyEvent)
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

int Kapman::signZeroPositive(const qreal value)
{
    return (value >= 0 ? 1 : -1);
}

int Kapman::sign(const qreal value)
{
    if(value == 0)
    {
        return 0;
    }
    return (value > 0 ? 1 : -1);
}
