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

#include <math.h>

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
    //TODO: refactor with xMove and yMove flags and speed and askedSpeed with
    //       0:  not moving
    //       1:  move right/down
    //      -1:  move left/up
    //this should it make easier to merge the moving into one step instead of four
    
    //check if there is a hurdle in the way
    if(m_askedXSpeed != 0 || m_xSpeed != 0 || m_askedYSpeed != 0 || m_ySpeed != 0)
    {
        qreal deltaXMove = 0;
        qreal deltaYMove = 0;
        qreal deltaAskedMove = 0;
        qreal deltaXCellCorner = 0;
        qreal deltaYCellCorner = 0;
        qreal deltaXCellCenter = 0;
        qreal deltaXCellCenter_new = 0;
        qreal deltaYCellCenter = 0;
        qreal deltaYCellCenter_new = 0;
        int curCellRow = 0;
        int curCellCol = 0;
        Cell nextCell;
        
        //**************************************************************
        //mover right
        //**************************************************************
        if(m_askedXSpeed > 0 || m_xSpeed > 0)
        {
            deltaXMove = 0;
            deltaYMove = 0;
            deltaAskedMove = 0;
            deltaXCellCorner = 0;
            deltaYCellCorner = 0;
            deltaXCellCenter = 0;
            deltaXCellCenter_new = 0;
            deltaYCellCenter = 0;
            deltaYCellCenter_new = 0;
            
            // Get the current cell coordinates from the character coordinates
            curCellRow = m_maze->getRowFromY(m_y);
            curCellCol = m_maze->getColFromX(m_x);
            
            deltaXCellCorner = m_x - ((int) (m_x / Cell::SIZE)) * Cell::SIZE;
            deltaYCellCorner = m_y - ((int) (m_y / Cell::SIZE)) * Cell::SIZE;
            
            //how far to move
            deltaAskedMove = (m_askedXSpeed > 0 ? m_askedXSpeed : m_xSpeed);
            
            //how far to next cell center
            if(deltaXCellCorner == Cell::SIZE/2)
            {
                deltaXCellCenter = 0;
            }
            else if(deltaXCellCorner < Cell::SIZE/2)
            {
                deltaXCellCenter = Cell::SIZE/2 - deltaXCellCorner;
            }
            else
            {
                deltaXCellCenter = Cell::SIZE*3/2 - deltaXCellCorner;
            }
            
            deltaXCellCenter_new = Cell::SIZE/2 - deltaXCellCorner;
            deltaYCellCenter = Cell::SIZE/2 - deltaYCellCorner;
            if(deltaAskedMove <= deltaXCellCenter)
            {
                deltaXMove += deltaAskedMove;
                //move to y center if needed
                if(deltaYCellCorner != Cell::SIZE/2 && (signZeroPositive(deltaAskedMove) * deltaXCellCenter_new) < 0)
                {
                    if(fabs(deltaYCellCenter) > Cell::SIZE/2 - fabs(deltaXMove - deltaXCellCenter_new))
                    {
                        if(m_maze->getCell(curCellRow - signZeroPositive(deltaYCellCenter)*1, curCellCol + signZeroPositive(deltaAskedMove)*1).getType() == Cell::WALL)
                        {
                            deltaYMove = deltaYCellCenter + signZeroPositive(deltaYCellCenter) * (fabs(deltaXMove - deltaXCellCenter_new) - Cell::SIZE/2);
                            if(fabs(deltaYMove) > fabs(deltaYCellCenter))
                            {
                                deltaYMove = deltaYCellCenter;
                            }
                        }
                    }
                }
            }
            else
            {
                deltaXMove += deltaXCellCenter;
                deltaAskedMove -= deltaXCellCenter;
                if(deltaXCellCorner > Cell::SIZE/2)
                {
                    //move to y center if needed
                    if(deltaYCellCorner != Cell::SIZE/2 && m_maze->getCell(curCellRow - signZeroPositive(deltaYCellCenter)*1, curCellCol + signZeroPositive(deltaAskedMove)*1).getType() == Cell::WALL)
                    {
                        deltaYMove = deltaYCellCenter;
                    }
                    curCellCol++;
                }
                while(deltaAskedMove > 0)
                {
                    nextCell = m_maze->getCell(curCellRow, curCellCol + 1);
                    if(nextCell.getType() == Cell::GROUND)
                    {
                        if(deltaAskedMove > Cell::SIZE)
                        {
                            deltaXMove += Cell::SIZE;
                            deltaAskedMove -= Cell::SIZE;
                            //move to y center if needed
                            if(deltaYCellCorner != Cell::SIZE/2 && m_maze->getCell(curCellRow - signZeroPositive(deltaYCellCenter)*1, curCellCol + signZeroPositive(deltaAskedMove)*1).getType() == Cell::WALL)
                            {
                                deltaYMove = deltaYCellCenter;
                            }
                        }
                        else
                        {
                            deltaXMove += deltaAskedMove;
                            //move to y center if needed
                            if(deltaYMove == 0 && deltaYCellCorner != Cell::SIZE/2 && fabs(deltaYCellCenter) > (Cell::SIZE/2 - fabs(deltaXMove - deltaXCellCenter_new)))
                            {
                                if(m_maze->getCell(curCellRow - signZeroPositive(deltaYCellCenter)*1, curCellCol + signZeroPositive(deltaAskedMove)*1).getType() == Cell::WALL)
                                {
                                    deltaYMove = signZeroPositive(deltaYCellCenter) * fabs(deltaAskedMove);
                                    if(fabs(deltaYMove) > fabs(deltaYCellCenter))
                                    {
                                        deltaYMove = deltaYCellCenter;
                                    }
                                }
                            }
                            deltaAskedMove = 0;
                        }
                        curCellCol++;
                    }
                    else
                    {
                        deltaAskedMove = 0;
                    }
                }
            }
        }
        //**************************************************************
        //mover left
        //**************************************************************
        else if(m_askedXSpeed < 0 || m_xSpeed < 0)
        {
            deltaXMove = 0;
            deltaYMove = 0;
            deltaAskedMove = 0;
            deltaXCellCorner = 0;
            deltaYCellCorner = 0;
            deltaXCellCenter = 0;
            deltaXCellCenter_new = 0;
            deltaYCellCenter = 0;
            deltaYCellCenter_new = 0;
            
            // Get the current cell coordinates from the character coordinates
            curCellRow = m_maze->getRowFromY(m_y);
            curCellCol = m_maze->getColFromX(m_x);
            
            deltaXCellCorner = m_x - ((int) (m_x / Cell::SIZE)) * Cell::SIZE;
            deltaYCellCorner = m_y - ((int) (m_y / Cell::SIZE)) * Cell::SIZE;
            
            //how far to move
            deltaAskedMove = (m_askedXSpeed < 0 ? m_askedXSpeed : m_xSpeed);
            
            //how far to next cell center
            if(deltaXCellCorner == Cell::SIZE/2)
            {
                deltaXCellCenter = 0;
            }
            else if(deltaXCellCorner < Cell::SIZE/2)
            {
                deltaXCellCenter = 0 - Cell::SIZE/2 - deltaXCellCorner;
            }
            else
            {
                deltaXCellCenter = Cell::SIZE/2 - deltaXCellCorner;
            }
            
            deltaXCellCenter_new = Cell::SIZE/2 - deltaXCellCorner;
            deltaYCellCenter = Cell::SIZE/2 - deltaYCellCorner;
            if(deltaAskedMove >= deltaXCellCenter)
            {
                deltaXMove += deltaAskedMove;
                //move to y center if needed
                if(deltaYCellCorner != Cell::SIZE/2 && (signZeroPositive(deltaAskedMove) * deltaXCellCenter_new) < 0)
                {
                    if(fabs(deltaYCellCenter) > Cell::SIZE/2 - fabs(deltaXMove - deltaXCellCenter_new))
                    {
                        if(m_maze->getCell(curCellRow - signZeroPositive(deltaYCellCenter)*1, curCellCol + signZeroPositive(deltaAskedMove)*1).getType() == Cell::WALL)
                        {
                            deltaYMove = deltaYCellCenter + signZeroPositive(deltaYCellCenter) * (fabs(deltaXMove - deltaXCellCenter_new) - Cell::SIZE/2);
                            if(fabs(deltaYMove) > fabs(deltaYCellCenter))
                            {
                                deltaYMove = deltaYCellCenter;
                            }
                        }
                    }
                }
            }
            else
            {
                deltaXMove += deltaXCellCenter;
                deltaAskedMove -= deltaXCellCenter;
                if(deltaXCellCorner < Cell::SIZE/2)
                {
                    //move to y center if needed
                    if(deltaYCellCorner != Cell::SIZE/2 && m_maze->getCell(curCellRow - signZeroPositive(deltaYCellCenter)*1, curCellCol + signZeroPositive(deltaAskedMove)*1).getType() == Cell::WALL)
                    {
                        deltaYMove = deltaYCellCenter;
                    }
                    curCellCol--;
                }
                while(deltaAskedMove < 0)
                {
                    nextCell = m_maze->getCell(curCellRow, curCellCol - 1);
                    if(nextCell.getType() == Cell::GROUND)
                    {
                        if(fabs(deltaAskedMove) > Cell::SIZE)
                        {
                            deltaXMove -= Cell::SIZE;
                            deltaAskedMove += Cell::SIZE;
                            //move to y center if needed
                            if(deltaYCellCorner != Cell::SIZE/2 && m_maze->getCell(curCellRow - signZeroPositive(deltaYCellCenter)*1, curCellCol + signZeroPositive(deltaAskedMove)*1).getType() == Cell::WALL)
                            {
                                deltaYMove = deltaYCellCenter;
                            }
                        }
                        else
                        {
                            deltaXMove += deltaAskedMove;
                            //move to y center if needed
                            if(deltaYMove == 0 && deltaYCellCorner != Cell::SIZE/2 && fabs(deltaYCellCenter) > (Cell::SIZE/2 - fabs(deltaXMove - deltaXCellCenter_new)))
                            {
                                if(m_maze->getCell(curCellRow - signZeroPositive(deltaYCellCenter)*1, curCellCol + signZeroPositive(deltaAskedMove)*1).getType() == Cell::WALL)
                                {
                                    deltaYMove = signZeroPositive(deltaYCellCenter) * fabs(deltaAskedMove);
                                    if(fabs(deltaYMove) > fabs(deltaYCellCenter))
                                    {
                                        deltaYMove = deltaYCellCenter;
                                    }
                                }
                            }
                            deltaAskedMove = 0;
                        }
                        curCellCol--;
                    }
                    else
                    {
                        deltaAskedMove = 0;
                    }
                }
            }
        }
        //**************************************************************
        //mover down
        //**************************************************************
        else if(m_askedYSpeed > 0 || m_ySpeed > 0)
        {
            deltaXMove = 0;
            deltaYMove = 0;
            deltaAskedMove = 0;
            deltaXCellCorner = 0;
            deltaYCellCorner = 0;
            deltaXCellCenter = 0;
            deltaXCellCenter_new = 0;
            deltaYCellCenter = 0;
            deltaYCellCenter_new = 0;
            
            // Get the current cell coordinates from the character coordinates
            curCellRow = m_maze->getRowFromY(m_y);
            curCellCol = m_maze->getColFromX(m_x);
            
            deltaXCellCorner = m_x - ((int) (m_x / Cell::SIZE)) * Cell::SIZE;
            deltaYCellCorner = m_y - ((int) (m_y / Cell::SIZE)) * Cell::SIZE;
            
            //how far to move
            deltaAskedMove = (m_askedYSpeed > 0 ? m_askedYSpeed : m_ySpeed);
            
            //how far to next cell center
            if(deltaYCellCorner == Cell::SIZE/2)
            {
                deltaYCellCenter = 0;
            }
            else if(deltaYCellCorner < Cell::SIZE/2)
            {
                deltaYCellCenter = Cell::SIZE/2 - deltaYCellCorner;
            }
            else
            {
                deltaYCellCenter = Cell::SIZE*3/2 - deltaYCellCorner;
            }
            
            deltaXCellCenter = Cell::SIZE/2 - deltaXCellCorner;
            deltaYCellCenter_new = Cell::SIZE/2 - deltaYCellCorner;
            if(deltaAskedMove <= deltaYCellCenter)
            {
                deltaYMove += deltaAskedMove;
                //move to x center if needed
                if(deltaXCellCorner != Cell::SIZE/2 && (signZeroPositive(deltaAskedMove) * deltaYCellCenter_new) < 0)
                {
                    if(fabs(deltaXCellCenter) > Cell::SIZE/2 - fabs(deltaYMove - deltaYCellCenter_new))
                    {
                        if(m_maze->getCell(curCellRow + signZeroPositive(deltaAskedMove)*1, curCellCol  - signZeroPositive(deltaXCellCenter)*1).getType() == Cell::WALL)
                        {
                            deltaXMove = deltaXCellCenter + signZeroPositive(deltaXCellCenter) * (fabs(deltaYMove - deltaYCellCenter_new) - Cell::SIZE/2);
                            if(fabs(deltaXMove) > fabs(deltaXCellCenter))
                            {
                                deltaXMove = deltaXCellCenter;
                            }
                        }
                    }
                }
            }
            else
            {
                deltaYMove += deltaYCellCenter;
                deltaAskedMove -= deltaYCellCenter;
                if(deltaYCellCorner > Cell::SIZE/2)
                {
                    //move to x center if needed
                    if(deltaXCellCorner != Cell::SIZE/2 && m_maze->getCell(curCellRow + signZeroPositive(deltaAskedMove)*1, curCellCol  - signZeroPositive(deltaXCellCenter)*1).getType() == Cell::WALL)
                    {
                        deltaXMove = deltaXCellCenter;
                    }
                    curCellRow++;
                }
                while(deltaAskedMove > 0)
                {
                    nextCell = m_maze->getCell(curCellRow + 1, curCellCol);
                    if(nextCell.getType() == Cell::GROUND)
                    {
                        if(deltaAskedMove > Cell::SIZE)
                        {
                            deltaYMove += Cell::SIZE;
                            deltaAskedMove -= Cell::SIZE;
                            //move to x center if needed
                            if(deltaXCellCorner != Cell::SIZE/2 && m_maze->getCell(curCellRow + signZeroPositive(deltaAskedMove)*1, curCellCol  - signZeroPositive(deltaXCellCenter)*1).getType() == Cell::WALL)
                            {
                                deltaXMove = deltaXCellCenter;
                            }
                        }
                        else
                        {
                            deltaYMove += deltaAskedMove;
                            //move to x center if needed
                            if(deltaXMove == 0 && deltaXCellCorner != Cell::SIZE/2 && fabs(deltaXCellCenter) > (Cell::SIZE/2 - fabs(deltaYMove - deltaYCellCenter_new)))
                            {
                                if(m_maze->getCell(curCellRow + signZeroPositive(deltaAskedMove)*1, curCellCol  - signZeroPositive(deltaXCellCenter)*1).getType() == Cell::WALL)
                                {
                                    deltaXMove = signZeroPositive(deltaXCellCenter) * fabs(deltaAskedMove);
                                    if(fabs(deltaXMove) > fabs(deltaXCellCenter))
                                    {
                                        deltaXMove = deltaXCellCenter;
                                    }
                                }
                            }
                            deltaAskedMove = 0;
                        }
                        curCellRow++;
                    }
                    else
                    {
                        deltaAskedMove = 0;
                    }
                }
            }
        }
        //**************************************************************
        //mover up
        //**************************************************************
        else
        {
            deltaXMove = 0;
            deltaYMove = 0;
            deltaAskedMove = 0;
            deltaXCellCorner = 0;
            deltaYCellCorner = 0;
            deltaXCellCenter = 0;
            deltaXCellCenter_new = 0;
            deltaYCellCenter = 0;
            deltaYCellCenter_new = 0;
            
            // Get the current cell coordinates from the character coordinates
            curCellRow = m_maze->getRowFromY(m_y);
            curCellCol = m_maze->getColFromX(m_x);
            
            deltaXCellCorner = m_x - ((int) (m_x / Cell::SIZE)) * Cell::SIZE;
            deltaYCellCorner = m_y - ((int) (m_y / Cell::SIZE)) * Cell::SIZE;
            
            //how far to move
            deltaAskedMove = (m_askedYSpeed < 0 ? m_askedYSpeed : m_ySpeed);
            
            //how far to next cell center
            if(deltaYCellCorner == Cell::SIZE/2)
            {
                deltaYCellCenter = 0;
            }
            else if(deltaYCellCorner < Cell::SIZE/2)
            {
                deltaYCellCenter = 0 - Cell::SIZE/2 - deltaYCellCorner;
            }
            else
            {
                deltaYCellCenter = Cell::SIZE/2 - deltaYCellCorner;
            }
            
            deltaXCellCenter = Cell::SIZE/2 - deltaXCellCorner;
            deltaYCellCenter_new = Cell::SIZE/2 - deltaYCellCorner;
            if(deltaAskedMove >= deltaYCellCenter)
            {
                deltaYMove += deltaAskedMove;
                //move to x center if needed
                if(deltaXCellCorner != Cell::SIZE/2 && (signZeroPositive(deltaAskedMove) * deltaYCellCenter_new) < 0)
                {
                    if(fabs(deltaXCellCenter) > Cell::SIZE/2 - fabs(deltaYMove - deltaYCellCenter_new))
                    {
                        if(m_maze->getCell(curCellRow + signZeroPositive(deltaAskedMove)*1, curCellCol  - signZeroPositive(deltaXCellCenter)*1).getType() == Cell::WALL)
                        {
                            deltaXMove = deltaXCellCenter + signZeroPositive(deltaXCellCenter) * (fabs(deltaYMove - deltaYCellCenter_new) - Cell::SIZE/2);
                            if(fabs(deltaXMove) > fabs(deltaXCellCenter))
                            {
                                deltaXMove = deltaXCellCenter;
                            }
                        }
                    }
                }
            }
            else
            {
                deltaYMove += deltaYCellCenter;
                deltaAskedMove -= deltaYCellCenter;
                if(deltaYCellCorner < Cell::SIZE/2)
                {
                    //move to x center if needed
                    if(deltaXCellCorner != Cell::SIZE/2 && m_maze->getCell(curCellRow + signZeroPositive(deltaAskedMove)*1, curCellCol  - signZeroPositive(deltaXCellCenter)*1).getType() == Cell::WALL)
                    {
                        deltaXMove = deltaXCellCenter;
                    }
                    curCellRow--;
                }
                while(deltaAskedMove < 0)
                {
                    nextCell = m_maze->getCell(curCellRow - 1, curCellCol);
                    if(nextCell.getType() == Cell::GROUND)
                    {
                        if(fabs(deltaAskedMove) > Cell::SIZE)
                        {
                            deltaYMove -= Cell::SIZE;
                            deltaAskedMove += Cell::SIZE;
                            //move to x center if needed
                            if(deltaXCellCorner != Cell::SIZE/2 && m_maze->getCell(curCellRow + signZeroPositive(deltaAskedMove)*1, curCellCol  - signZeroPositive(deltaXCellCenter)*1).getType() == Cell::WALL)
                            {
                                deltaXMove = deltaXCellCenter;
                            }
                        }
                        else
                        {
                            deltaYMove += deltaAskedMove;
                            //move to x center if needed
                            if(deltaXMove == 0 && deltaXCellCorner != Cell::SIZE/2 && fabs(deltaXCellCenter) > (Cell::SIZE/2 - fabs(deltaYMove - deltaYCellCenter_new)))
                            {
                                if(m_maze->getCell(curCellRow + signZeroPositive(deltaAskedMove)*1, curCellCol  - signZeroPositive(deltaXCellCenter)*1).getType() == Cell::WALL)
                                {
                                    deltaXMove = signZeroPositive(deltaXCellCenter) * fabs(deltaAskedMove);
                                    if(fabs(deltaXMove) > fabs(deltaXCellCenter))
                                    {
                                        deltaXMove = deltaXCellCenter;
                                    }
                                }
                            }
                            deltaAskedMove = 0;
                        }
                        curCellRow--;
                    }
                    else
                    {
                        deltaAskedMove = 0;
                    }
                }
            }
        }
        // Update the direction
        if(m_askedXSpeed != 0 || m_askedYSpeed != 0)
        {
            updateDirection();
        }
        // Move the kapman
        move(m_x + deltaXMove, m_y + deltaYMove);
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

int Kapman::signZeroPositive(qreal value)
{
    return (value >= 0 ? 1 : -1);
}
