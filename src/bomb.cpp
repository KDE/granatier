/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
    SPDX-FileCopyrightText: 2007-2008 Pierre-Benoît Besse <besse.pb@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "bomb.h"
#include "arena.h"
#include "granatier_random.h"

#include <QTimer>

#include <cstdlib>

const int nMortarRampEnd = static_cast<int>(Granatier::FPS * 50 / 1000.0);
const int nMortarPeak = static_cast<int>((Granatier::FPS * 800 / 1000.0) / 2 + nMortarRampEnd);
const int nMortarGround = static_cast<int>((Granatier::FPS * 800 / 1000.0) + nMortarRampEnd);

Bomb::Bomb(qreal fX, qreal fY, Arena* p_arena, int nBombID, int nDetonationCountdown) : Element(fX, fY, p_arena), m_xSpeed(0), m_ySpeed(0)
{
    m_type = Granatier::Element::BOMB;

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
    connect(m_detonationCountdownTimer, &QTimer::timeout, this, &Bomb::detonate);

    m_mortarTimer = nullptr;
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
    if(m_mortarState < 0 && m_xSpeed == 0 && m_ySpeed == 0)
    {
        switch (m_arena->getCell(currentRow, currentCol).getType())
        {
            case Granatier::Cell::ARROWUP:
                setYSpeed(-5);
                break;
            case Granatier::Cell::ARROWRIGHT:
                setXSpeed(5);
                break;
            case Granatier::Cell::ARROWDOWN:
                setYSpeed(5);
                break;
            case Granatier::Cell::ARROWLEFT:
                setXSpeed(-5);
                break;
            case Granatier::Cell::BOMBMORTAR:
                m_mortarTimer = new QTimer;
                m_mortarTimer->setSingleShot(true);
                m_mortarTimer->setInterval(1500);
                m_mortarTimer->start();
                m_detonationCountdownTimer->stop();
                m_mortarState = 0;
                updateMortarState();
                break;
            case Granatier::Cell::HOLE:
                if(!m_falling)
                {
                    m_falling = true;
                    m_type = Granatier::Element::NONE;
                    m_detonationCountdownTimer->stop();
                    Q_EMIT falling();
                    Q_EMIT releaseBombArmory();
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
        int xDeltaCenter = static_cast<int>(Granatier::CellSize/2 - (m_x - currentCol * Granatier::CellSize));
        bool bMoveAwayFromCenter = false;
        bool bIsHurdleCurrentCell = false;
        bool bIsHurdleNextCell = false;

        if (m_xSpeed > 0)
        {
            xDirection = 1;
        }
        else if (m_xSpeed < 0)
        {
            xDirection = -1;
        }
        int yDirection = 0;
        int yDeltaCenter = static_cast<int>(Granatier::CellSize/2 - (m_y - currentRow * Granatier::CellSize));
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
            nextRow = m_arena->getRowFromY(m_y + yDirection * Granatier::CellSize/2); //this is needed because the bombs won't move if they are coming from the top, hit an up arrow and there is a hurdle below the arrow
            nextCol = m_arena->getColFromX(m_x + xDirection * Granatier::CellSize/2);
        }
        else
        {
            nextRow = m_arena->getRowFromY(m_y + m_ySpeed + yDirection * Granatier::CellSize/2);
            nextCol = m_arena->getColFromX(m_x + m_xSpeed + xDirection * Granatier::CellSize/2);
        }

        bIsHurdleCurrentCell = !(m_arena->getCell(currentRow, currentCol).isWalkable(this));
        bIsHurdleNextCell = !(m_arena->getCell(nextRow, nextCol).isWalkable(this));

        if(xDirection * xDeltaCenter <= 0 || yDirection * yDeltaCenter <= 0)
        {
            bMoveAwayFromCenter = true;
        }

        //at first, check if move over cell center or currently on cell center
        if((bOnCenter || (xDirection * xDeltaCenter < 0 && xDirection * (m_xSpeed + xDeltaCenter) >= 0) || (yDirection * yDeltaCenter < 0 && yDirection * (m_ySpeed + yDeltaCenter) >= 0)) && m_mortarState < 0)
        {
            bool bIsMortar = false;
            bool bIsNewDirection = false;

            switch (m_arena->getCell(currentRow, currentCol).getType())
            {
                case Granatier::Cell::ARROWUP:
                    if(yDirection != -1)
                    {
                        bIsNewDirection = true;
                        bMoveAwayFromCenter = true;
                    }
                    break;
                case Granatier::Cell::ARROWRIGHT:
                    if(xDirection != 1)
                    {
                        bIsNewDirection = true;
                        bMoveAwayFromCenter = true;
                    }
                    break;
                case Granatier::Cell::ARROWDOWN:
                    if(yDirection != 1)
                    {
                        bIsNewDirection = true;
                        bMoveAwayFromCenter = true;
                    }
                    break;
                case Granatier::Cell::ARROWLEFT:
                    if(xDirection != -1)
                    {
                        bIsNewDirection = true;
                        bMoveAwayFromCenter = true;
                    }
                    break;
                case Granatier::Cell::BOMBMORTAR:
                    bIsMortar = true;
                    break;
                case Granatier::Cell::HOLE:
                    m_stopOnCenter = true;
                    break;
                default:
                    break;
            }

            //if two bombs move towards them, stop them and move them to the next cell center
            if(((bIsHurdleCurrentCell && !bMoveAwayFromCenter) || bIsHurdleNextCell) && !m_stopOnCenter)
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
                move((currentCol+0.5) * Granatier::CellSize, (currentRow+0.5) * Granatier::CellSize);
                setXSpeed(0);
                setYSpeed(0);
                m_stopOnCenter = false;
            }
            else
            {
                if((newRow != currentRow || newCol != currentCol) && m_mortarState < 0)
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
            if(((bIsHurdleCurrentCell && !bMoveAwayFromCenter) || bIsHurdleNextCell) && !m_stopOnCenter && m_mortarState < 0)
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
                if((newRow != currentRow || newCol != currentCol) && m_mortarState < 0)
                {
                    m_arena->removeCellElement(currentRow, currentCol, this);
                    m_arena->setCellElement(newRow, newCol, this);
                }
                move(m_x + m_xSpeed, m_y + m_ySpeed);
            }
        }
    }

    if(m_mortarState >= 0 && !(m_mortarTimer->isActive()))
    {
        updateMortarState();
    }
}

void Bomb::move(qreal x, qreal y)
{
    // Move the Bomb
    m_x = x;
    m_y = y;
    Q_EMIT moved(m_x, m_y);
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
    }
    qreal fSpeed = 2 * Granatier::CellSize / (Granatier::FPS * 800 / 1000.0 + 1);
    switch(nDirection)
    {
        case Granatier::Direction::NORTH:
            setXSpeed(0);
            setYSpeed(-fSpeed);
            break;
        case Granatier::Direction::EAST:
            setXSpeed(fSpeed);
            setYSpeed(0);
            break;
        case Granatier::Direction::SOUTH:
            setXSpeed(0);
            setYSpeed(fSpeed);
            break;
        case Granatier::Direction::WEST:
            setXSpeed(-fSpeed);
            setYSpeed(0);
            break;
    }

    int curCellRow = m_arena->getRowFromY(m_y);
    int curCellCol = m_arena->getColFromX(m_x);
    m_arena->removeCellElement(curCellRow, curCellCol, this);

    m_type = Granatier::Element::NONE;
    m_thrown = true;
    m_mortarState = nMortarRampEnd;
}

void Bomb::setKicked(int nDirection)
{
    switch(nDirection)
    {
        case Granatier::Direction::NORTH:
            setXSpeed(0);
            setYSpeed(-5);
            break;
        case Granatier::Direction::EAST:
            setXSpeed(5);
            setYSpeed(0);
            break;
        case Granatier::Direction::SOUTH:
            setXSpeed(0);
            setYSpeed(5);
            break;
        case Granatier::Direction::WEST:
            setXSpeed(-5);
            setYSpeed(0);
            break;
    }
}

bool Bomb::onCenter()
{
    // Get the current cell center coordinates
    qreal centerX = (m_arena->getColFromX(m_x) + 0.5) * Granatier::CellSize;
    qreal centerY = (m_arena->getRowFromY(m_y) + 0.5) * Granatier::CellSize;
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
    setX((m_arena->getColFromX(m_x) + 0.5) * Granatier::CellSize);
    setY((m_arena->getRowFromY(m_y) + 0.5) * Granatier::CellSize);
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
        m_detonationCountdownTimer = nullptr;
        Q_EMIT bombDetonated(this);
        Q_EMIT releaseBombArmory();
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
    Q_EMIT mortar(m_mortarState, nMortarRampEnd, nMortarPeak, nMortarGround);

    if(m_mortarState <= 0)
    {
        int curCellRow = m_arena->getRowFromY(m_y);
        int curCellCol = m_arena->getColFromX(m_x);

        setXSpeed(0);
        setYSpeed(0);

        m_thrown = false;
        m_type = Granatier::Element::NONE;
        m_arena->removeCellElement(curCellRow, curCellCol, this);
    }
    else if(m_mortarState == nMortarRampEnd)
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
                nRow = granatier::RNG::fromRange(0, m_arena->getNbRows()-1);
                nCol = granatier::RNG::fromRange(0, m_arena->getNbColumns()-1);
                if(m_arena->getCell(nRow, nCol).getType() != Granatier::Cell::WALL)
                {
                    bFound = true;
                }
            }
            while (!bFound);

            setXSpeed((nCol - curCellCol) * Granatier::CellSize / (Granatier::FPS * 800 / 1000.0));
            setYSpeed((nRow - curCellRow) * Granatier::CellSize / (Granatier::FPS * 800 / 1000.0));
        }

        m_type = Granatier::Element::NONE;
        m_arena->removeCellElement(curCellRow, curCellCol, this);
    }
    else if(m_mortarState == nMortarGround)
    {
        setXSpeed(0);
        setYSpeed(0);
    }
    else if (m_mortarState > nMortarGround)
    {
        int curCellRow = m_arena->getRowFromY(m_y);
        int curCellCol = m_arena->getColFromX(m_x);
        m_type = Granatier::Element::BOMB;
        m_arena->setCellElement(curCellRow, curCellCol, this);

        m_mortarState = -1;
        if(m_detonationCountdownTimer)
        {
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

    if(m_mortarState >= 0)
    {
        m_mortarState++;
    }
}

#include "moc_bomb.cpp"
