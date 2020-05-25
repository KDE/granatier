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
#include "bomb.h"
#include "arena.h"
#include "config/playersettings.h"
#include "settings.h"

#include <QKeyEvent>
#include <QTimer>

#include <cmath>

constexpr int onIceSpeedIncrease = 2;
constexpr int badBonusTimerTimeout = 100;
constexpr int badBonusCountdown = 10000;

Player::Player(qreal p_x, qreal p_y, const QString& p_playerID, const PlayerSettings* p_playerSettings, Arena* p_arena)
 : Character(p_x, p_y, p_arena)
 , m_throwBomb(false)
 , m_kickBomb(false)
{
    m_type = Granatier::Element::PLAYER;
    m_desktopFilePath = p_playerSettings->playerDesktopFilePath(p_playerID);
    m_graphicsFile = p_playerSettings->playerGraphicsFile(p_playerID);
    m_playerName = p_playerSettings->playerName(p_playerID);

    m_points = 0;

    m_direction = Granatier::Direction::EAST;

    m_badBonusCountdownTimer = new QTimer;
    m_badBonusCountdownTimer->setInterval(badBonusTimerTimeout);
    m_badBonusMillisecondsToElapse = 0;
    connect(m_badBonusCountdownTimer, &QTimer::timeout, this, &Player::slot_badBonusTimerTimeout);

    resurrect();

    m_key.moveLeft = p_playerSettings->keyLeft(p_playerID);
    m_key.moveRight = p_playerSettings->keyRight(p_playerID);
    m_key.moveUp = p_playerSettings->keyUp(p_playerID);
    m_key.moveDown = p_playerSettings->keyDown(p_playerID);
    m_key.dropBomb = p_playerSettings->keyPutBomb(p_playerID);
}

Player::~Player()
{
    delete m_badBonusCountdownTimer;
}

void Player::setShortcuts(const Shortcuts &keys)
{
    m_key = keys;
}

QString Player::getGraphicsFile() const
{
    return m_graphicsFile;
}

QString Player::getDesktopFilePath() const
{
  return m_desktopFilePath;
}

QString Player::getPlayerName() const
{
    return m_playerName;
}

void Player::init()
{
    updateDirection();
    stopMoving();
    int row = m_arena->getRowFromY(m_y);
    int column = m_arena->getColFromX(m_x);
    m_arena->setCellElement(row, column, this);
}

void Player::pause()
{
    m_badBonusCountdownTimer->stop();
}

void Player::resume()
{
    if(m_badBonusMillisecondsToElapse > 0)
    {
        m_badBonusCountdownTimer->start();
    }
}

void Player::goUp()
{
    m_askedXSpeed = 0;

    qreal nSpeed = m_speed;
    if(m_onIce)
    {
        nSpeed = m_speed + onIceSpeedIncrease;
    }
    m_askedYSpeed = -nSpeed;

    m_direction = Granatier::Direction::NORTH;
}

void Player::goDown()
{
    m_askedXSpeed = 0;

    qreal nSpeed = m_speed;
    if(m_onIce)
    {
        nSpeed = m_speed + onIceSpeedIncrease;
    }
    m_askedYSpeed = nSpeed;

    m_direction = Granatier::Direction::SOUTH;
}

void Player::goRight()
{
    qreal nSpeed = m_speed;
    if(m_onIce)
    {
        nSpeed = m_speed + onIceSpeedIncrease;
    }
    m_askedXSpeed = nSpeed;

    m_askedYSpeed = 0;

    m_direction = Granatier::Direction::EAST;
}

void Player::goLeft()
{
    qreal nSpeed = m_speed;
    if(m_onIce)
    {
        nSpeed = m_speed + onIceSpeedIncrease;
    }
    m_askedXSpeed = -nSpeed;

    m_askedYSpeed = 0;

    m_direction = Granatier::Direction::WEST;
}

void Player::updateDirection()
{
    if(m_death)
    {
        return;
    }

    setXSpeed(m_askedXSpeed);
    setYSpeed(m_askedYSpeed);
    m_askedXSpeed = 0;
    m_askedYSpeed = 0;
    // Signal to the player item that the direction changed
    emit directionChanged();
}

void Player::updateMove()
{
    if(m_death)
    {
        return;
    }

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
        int moveStartRow = m_arena->getRowFromY(m_y);
        int moveStartCol = m_arena->getColFromX(m_x);
        int curCellRow = moveStartRow;
        int curCellCol = moveStartCol;

        //set variables for right/left move
        if(m_askedXSpeed != 0 || m_xSpeed != 0)
        {
            //how far to move
            deltaAskedMove = (m_askedXSpeed != 0 ? m_askedXSpeed : m_xSpeed);

            //direction to move
            xDirection = sign(deltaAskedMove);
            straightDirection = xDirection;

            deltaStraightCellCorner = m_x - curCellCol * Granatier::CellSize;
            deltaPerpendicularCellCorner = m_y - curCellRow * Granatier::CellSize;
        }
        else        //set variables for down/up move
        {
            //how far to move
            deltaAskedMove = (m_askedYSpeed != 0 ? m_askedYSpeed : m_ySpeed);

            //direction to move
            yDirection = sign(deltaAskedMove);
            straightDirection = yDirection;

            deltaStraightCellCorner = m_y - curCellRow * Granatier::CellSize;
            deltaPerpendicularCellCorner = m_x - curCellCol * Granatier::CellSize;
        }

        //how far to current cell center
        deltaStraightCellCenter = Granatier::CellSize/2 - deltaStraightCellCorner;
        deltaPerpendicularCellCenter = Granatier::CellSize/2 - deltaPerpendicularCellCorner;

        //check if the move exceeds a cell center
        if(straightDirection*deltaStraightCellCenter >= 0)
        {
            if(fabs(deltaAskedMove) <= fabs(deltaStraightCellCenter))
            {
                bMoveWithinNextCellCenter = true;
            }
        }
        else if(fabs(deltaAskedMove) + fabs(deltaStraightCellCenter) <= Granatier::CellSize)
        {
            bMoveWithinNextCellCenter = true;
        }

        //the move is within two cell centers
        if(bMoveWithinNextCellCenter)
        {
            bool isHurdle = false;
            if(!(m_badBonusCountdownTimer->isActive() && m_badBonusType == Granatier::Bonus::SCATTY))
            {
                bool kickBomb = false;
                QList<Element*> bombElements;
                //moving towards cell center; don't move if there is a bomb in the cell
                if(deltaStraightCellCenter * straightDirection > 0 && !m_arena->getCell(moveStartRow, moveStartCol).isWalkable(this) && !m_omitBombCurrentCell)
                {
                    isHurdle = true;
                    if(m_kickBomb)
                    {
                        kickBomb = true;
                        bombElements =  m_arena->getCell(moveStartRow, moveStartCol).getElements(Granatier::Element::BOMB);
                    }
                }
                //moving away of cell center; don't move if there is a bomb in the next cell; ignore a bomb in the current cell
                else if(deltaStraightCellCenter * straightDirection < 0 && !m_arena->getCell(moveStartRow + yDirection, moveStartCol + xDirection).isWalkable(this))
                {
                    isHurdle = true;
                    if(m_kickBomb)
                    {
                        kickBomb = true;
                        bombElements =  m_arena->getCell(moveStartRow + yDirection, moveStartCol + xDirection).getElements(Granatier::Element::BOMB);
                    }
                }
                if(kickBomb)
                {
                    for(auto& element: bombElements)
                    {
                        dynamic_cast <Bomb*> (element)->setKicked(m_direction);
                    }
                }
            }
            if(!isHurdle)
            {
                deltaStraightMove += deltaAskedMove;
                //move to perpendicular center if needed
                if(deltaPerpendicularCellCenter != 0 && (straightDirection * deltaStraightCellCenter) < 0)  //not in perpendicular center and entering a new cell
                {
                    if(fabs(deltaPerpendicularCellCenter) > Granatier::CellSize/2 - fabs(deltaStraightMove - deltaStraightCellCenter))   //check if it already can collide with a hurdle
                    {
                        cellRow = curCellRow + yDirection - qAbs(xDirection)*signZeroPositive(deltaPerpendicularCellCenter);
                        cellCol = curCellCol + xDirection - qAbs(yDirection)*signZeroPositive(deltaPerpendicularCellCenter);
                        if(!m_arena->getCell(cellRow, cellCol).isWalkable(this))
                        {
                            deltaPerpendicularMove = deltaPerpendicularCellCenter + signZeroPositive(deltaPerpendicularCellCenter) * (fabs(deltaStraightMove - deltaStraightCellCenter) - Granatier::CellSize/2);
                            if(fabs(deltaPerpendicularMove) > fabs(deltaPerpendicularCellCenter))       //check if moved over perpendicular center
                            {
                                deltaPerpendicularMove = deltaPerpendicularCellCenter;
                            }
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
                deltaStraightMove += straightDirection * Granatier::CellSize;
                deltaAskedMove -= straightDirection * Granatier::CellSize;

                //move to perpendicular center if needed
                if(deltaPerpendicularCellCenter != 0)
                {
                    cellRow = curCellRow + yDirection - qAbs(xDirection)*signZeroPositive(deltaPerpendicularCellCenter);
                    cellCol = curCellCol + xDirection - qAbs(yDirection)*signZeroPositive(deltaPerpendicularCellCenter);
                    if(!m_arena->getCell(cellRow, cellCol).isWalkable(this))
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
                if(m_arena->getCell(curCellRow + yDirection, curCellCol + xDirection).isWalkable(this))     //check if next cell is walkable
                {
                    if(fabs(deltaAskedMove) > Granatier::CellSize)   //move to next cell center if the remaining move exceeds a cell center
                    {
                        deltaStraightMove += straightDirection * Granatier::CellSize;
                        deltaAskedMove -= straightDirection * Granatier::CellSize;
                        //move to perpendicular center if needed
                        if(deltaPerpendicularCellCenter != 0)
                        {
                            cellRow = curCellRow + yDirection - qAbs(xDirection)*signZeroPositive(deltaPerpendicularCellCenter);
                            cellCol = curCellCol + xDirection - qAbs(yDirection)*signZeroPositive(deltaPerpendicularCellCenter);
                            if(!m_arena->getCell(cellRow, cellCol).isWalkable(this))
                            {
                                deltaPerpendicularMove = deltaPerpendicularCellCenter;
                            }
                        }
                    }
                    else
                    {
                        deltaStraightMove += deltaAskedMove;
                        //move to perpendicular center if needed
                        if(deltaPerpendicularMove != deltaPerpendicularCellCenter && fabs(deltaPerpendicularCellCenter) > (Granatier::CellSize/2 - fabs(deltaStraightMove - deltaStraightCellCenter)))   //check if it is in or already moved to perpendicular center and if it already can collide with a hurdle ***TODO: it seems to be wrong to use deltaStraightMove here, because ist could be greater than Granatier::CellSize
                        {
                            cellRow = curCellRow + yDirection - qAbs(xDirection)*signZeroPositive(deltaPerpendicularCellCenter);
                            cellCol = curCellCol + xDirection - qAbs(yDirection)*signZeroPositive(deltaPerpendicularCellCenter);
                            if(!m_arena->getCell(cellRow, cellCol).isWalkable(this))
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
                else    //there is a hurdle in the next cell, so stop moving
                {
                    deltaAskedMove = 0;
                    cellRow = curCellRow + yDirection;
                    cellCol = curCellCol + xDirection;
                    //check if bomb
                    if(m_kickBomb)
                    {
                        QList<Element*> bombElements =  m_arena->getCell(cellRow, cellCol).getElements(Granatier::Element::BOMB);
                        for(auto& element: bombElements)
                        {
                            dynamic_cast <Bomb*> (element)->setKicked(m_direction);
                        }
                    }
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

        //check if the player is on ice
        // Get the current cell coordinates from the character coordinates
        int newCellRow = m_arena->getRowFromY(m_y);
        int newCellCol = m_arena->getColFromX(m_x);
        if(!m_onIce)
        {
            if(m_arena->getCell(newCellRow, newCellCol).getType() == Granatier::Cell::ICE)
            {
                if(xDirection != 0)
                {
                    setXSpeed(m_xSpeed + xDirection * onIceSpeedIncrease);
                }
                else
                {
                    setYSpeed(m_ySpeed + yDirection * onIceSpeedIncrease);
                }
                m_onIce = true;
            }
        }
        else
        {
            if(m_arena->getCell(newCellRow, newCellCol).getType() != Granatier::Cell::ICE)
            {
                if(m_arena->getCell(newCellRow, newCellCol).getType() != Granatier::Cell::HOLE)
                {
                    if(xDirection != 0)
                    {
                        setXSpeed(m_xSpeed - xDirection * onIceSpeedIncrease);
                    }
                    else
                    {
                        setYSpeed(m_ySpeed - yDirection * onIceSpeedIncrease);
                    }
                }
                m_onIce = false;

                if(m_xSpeed == 0 && m_ySpeed == 0 && m_askedXSpeed == 0 && m_askedYSpeed == 0)
                {
                    stopMoving();
                }
            }
        }

        //check if the player move in a hole
        if(m_arena->getCell(newCellRow, newCellCol).getType() == Granatier::Cell::HOLE)
        {
            m_falling = true;
            //check if cell center passed
            if(xDirection != 0)
            {
                qreal cellCenter = newCellCol * Granatier::CellSize + 0.5 * Granatier::CellSize;
                qreal deltaCellCenter = cellCenter - (m_x + m_xSpeed);
                if (cellCenter - m_x == 0)
                {
                    setXSpeed(0);
                    emit falling();
                }
                else if (xDirection * deltaCellCenter < 0)
                {
                    setXSpeed(cellCenter - m_x);
                }
            }
            else if (yDirection != 0)
            {
                qreal cellCenter = newCellRow * Granatier::CellSize + 0.5 * Granatier::CellSize;
                qreal deltaCellCenter = cellCenter - (m_y + m_ySpeed);
                if (cellCenter - m_y == 0)
                {
                    setYSpeed(0);
                    emit falling();
                }
                else if (yDirection * deltaCellCenter < 0)
                {
                    setYSpeed(cellCenter - m_y);
                }
            }
        }

        if(moveStartCol != newCellCol || moveStartRow != newCellRow)
        {
            m_arena->removeCellElement(moveStartRow, moveStartCol, this);
            m_arena->setCellElement(newCellRow, newCellCol, this);
            m_omitBombCurrentCell = false;
        }
    }

    //check if bad bonus scatty and drop bombs
    if(m_badBonusCountdownTimer->isActive() && m_badBonusType == Granatier::Bonus::SCATTY  && m_bombArmory > 0)
    {
        //TODO: improve
        emit bombDropped(this, m_x, m_y, true, 0);
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
    Granatier::Bonus::Type bonusType = p_bonus->getBonusType();

    if(m_badBonusCountdownTimer->isActive())
    {
        m_badBonusCountdownTimer->stop();
        slot_removeBadBonus();
    }

    switch (bonusType)
    {
        case Granatier::Bonus::SPEED:
            m_speed += 1;
            if(m_speed > m_maxSpeed)
            {
                m_speed = m_maxSpeed;
            }
            setXSpeed(sign(m_xSpeed) * m_speed);
            setYSpeed(sign(m_ySpeed) * m_speed);
            break;
        case Granatier::Bonus::POWER:
            m_bombPower++;
            if(m_bombPower > 10)
            {
                m_bombPower = 10;
            }
            break;
        case Granatier::Bonus::BOMB:
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
            break;
        case Granatier::Bonus::SHIELD:
            if(m_listShield.isEmpty() || m_listShield.last() != 0)
            {
                m_listShield.append(0);
            }
            break;
        case Granatier::Bonus::THROW:
            m_throwBomb = true;
            break;
        case Granatier::Bonus::KICK:
            m_kickBomb = true;
            break;
        case Granatier::Bonus::HYPERACTIVE:
            {
                qreal askedXSpeedTemp = m_askedXSpeed;
                qreal askedYSpeedTemp = m_askedYSpeed;
                m_normalSpeed = m_speed;
                m_speed = m_maxSpeed * 3;
                m_askedXSpeed = sign(m_xSpeed) * m_speed;
                m_askedYSpeed = sign(m_ySpeed) * m_speed;
                updateDirection();
                m_askedXSpeed = askedXSpeedTemp;
                m_askedYSpeed = askedYSpeedTemp;

                m_badBonusType = Granatier::Bonus::HYPERACTIVE;
                m_badBonusMillisecondsToElapse = badBonusCountdown;
                m_badBonusCountdownTimer->start();
            }
            break;
        case Granatier::Bonus::SLOW:
            {
                qreal askedXSpeedTemp = m_askedXSpeed;
                qreal askedYSpeedTemp = m_askedYSpeed;
                m_normalSpeed = m_speed;
                m_speed = 1;
                m_askedXSpeed = sign(m_xSpeed) * m_speed;
                m_askedYSpeed = sign(m_ySpeed) * m_speed;
                updateDirection();
                m_askedXSpeed = askedXSpeedTemp;
                m_askedYSpeed = askedYSpeedTemp;

                m_badBonusType = Granatier::Bonus::SLOW;
                m_badBonusMillisecondsToElapse = badBonusCountdown;
                m_badBonusCountdownTimer->start();
            }
            break;
        case Granatier::Bonus::MIRROR:
            {
                qreal askedXSpeedTemp = m_askedXSpeed;
                qreal askedYSpeedTemp = m_askedYSpeed;
                m_askedXSpeed = -m_xSpeed;
                m_askedYSpeed = -m_ySpeed;
                switch(m_direction)
                {
                    case Granatier::Direction::EAST:
                        m_direction = Granatier::Direction::WEST;
                        break;
                    case Granatier::Direction::WEST:
                        m_direction = Granatier::Direction::EAST;
                        break;
                    case Granatier::Direction::NORTH:
                        m_direction = Granatier::Direction::SOUTH;
                        break;
                    case Granatier::Direction::SOUTH:
                        m_direction = Granatier::Direction::NORTH;
                        break;
                }
                updateDirection();
                m_askedXSpeed = -askedXSpeedTemp;
                m_askedYSpeed = -askedYSpeedTemp;

                QKeySequence tempKey = m_key.moveLeft;
                m_key.moveLeft = m_key.moveRight;
                m_key.moveRight = tempKey;
                tempKey = m_key.moveUp;
                m_key.moveUp = m_key.moveDown;
                m_key.moveDown = tempKey;

                m_moveMirrored = true;
                m_badBonusType = Granatier::Bonus::MIRROR;
                m_badBonusMillisecondsToElapse = badBonusCountdown;
                m_badBonusCountdownTimer->start();
            }
            break;
        case Granatier::Bonus::SCATTY:
            m_badBonusType = Granatier::Bonus::SCATTY;
            m_badBonusMillisecondsToElapse = badBonusCountdown;
            m_badBonusCountdownTimer->start();
            break;
        case Granatier::Bonus::RESTRAIN:
            m_normalBombArmory = m_bombArmory;
            m_bombArmory = 0;
            m_badBonusType = Granatier::Bonus::RESTRAIN;
            m_badBonusMillisecondsToElapse = badBonusCountdown;
            m_badBonusCountdownTimer->start();
            break;
        case Granatier::Bonus::RESURRECT:
            emit resurrectBonusTaken();
            break;
        default:
            break;
    }

    Q_EMIT bonusUpdated(this, bonusType, 0);
}

bool Player::shield(int nExplosionID)
{
    for(int i = 0; i < m_listShield.count(); i++)
    {
        if(m_listShield[i] == nExplosionID)
        {
            return true;
        }
        else if(m_listShield[i] == 0)
        {
            m_listShield[i] = nExplosionID;
            if(i == m_listShield.count()-1)
            {
                emit bonusUpdated(this, Granatier::Bonus::SHIELD, 100);
            }
            return true;
        }
    }
    return false;
}

bool Player::hasShield()
{
    if(m_listShield.count() > 0 && m_listShield.last() == 0)
    {
        return true;
    }
    return false;
}

bool Player::hasThrowBomb()
{
    return m_throwBomb;
}

bool Player::hasKickBomb()
{
    return m_kickBomb;
}

bool Player::hasBadBonus()
{
    if(m_badBonusCountdownTimer->isActive())
    {
        return true;
    }
    return false;
}

void Player::die()
{
    if(!m_death)
    {
        m_death = true;
        emit dying();
        m_xSpeed = 0;
        m_xSpeed = 0;

        if(m_badBonusCountdownTimer->isActive())
        {
            m_badBonusCountdownTimer->stop();
            slot_removeBadBonus();
        }
        int row = m_arena->getRowFromY(m_y);
        int column = m_arena->getColFromX(m_x);
        m_arena->removeCellElement(row, column, this);
    }
}

bool Player::isAlive() const
{
    return !m_death;
}

void Player::resurrect()
{
    if(m_badBonusMillisecondsToElapse > 0)
    {
        slot_removeBadBonus();
    }

    m_onIce = false;
    m_falling = false;
    m_death = false;
    m_maxSpeed = 10;
    m_speed = Settings::self()->initialSpeed();
    m_normalSpeed = m_speed;
    m_moveMirrored = false;
    m_bombPower = Settings::self()->initialBombPower();
    m_maxBombArmory = Settings::self()->initialBombArmory();
    m_bombArmory = m_maxBombArmory;
    if(m_listShield.count() != 0)
    {
        m_listShield.clear();
        emit bonusUpdated(this, Granatier::Bonus::SHIELD, 100);
    }
    if(m_throwBomb)
    {
        m_throwBomb = false;
        emit bonusUpdated(this, Granatier::Bonus::THROW, 100);
    }
    if(m_kickBomb)
    {
        m_kickBomb = false;
        emit bonusUpdated(this, Granatier::Bonus::KICK, 100);
    }
    m_omitBombCurrentCell = false;
    if(m_badBonusCountdownTimer->isActive())
    {
        m_badBonusCountdownTimer->stop();
        slot_removeBadBonus();
    }

    //check if the player is above a hole
    if(m_arena)
    {
        int cellRow = m_arena->getRowFromY(m_y);
        int cellCol = m_arena->getColFromX(m_x);

        m_arena->removeCellElement(cellRow, cellCol, this); //just to be really sure

        if(m_arena->getCell(cellRow, cellCol).getType() == Granatier::Cell::HOLE)
        {
            move(m_xInit, m_yInit);
            cellRow = m_arena->getRowFromY(m_yInit);
            cellCol = m_arena->getColFromX(m_xInit);
        }

        m_arena->setCellElement(cellRow, cellCol, this);
    }

    emit resurrected();
}

int Player::points() const
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

int Player::direction()
{
    return m_direction;
}

int Player::getBombPower() const
{
    return m_bombPower;
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
    int* bombArmory = &m_bombArmory;
    if(m_badBonusCountdownTimer->isActive() && m_badBonusType == Granatier::Bonus::RESTRAIN)
    {
        bombArmory = &m_normalBombArmory;
    }
    (*bombArmory)++;
    if((*bombArmory) > m_maxBombArmory)
    {
        (*bombArmory) = m_maxBombArmory;
    }
}

void Player::slot_badBonusTimerTimeout()
{
    m_badBonusMillisecondsToElapse -= badBonusTimerTimeout;
    if(m_badBonusMillisecondsToElapse <= 0)
    {
        slot_removeBadBonus();
    }
    else
    {
        bonusUpdated(this, m_badBonusType, (badBonusCountdown - m_badBonusMillisecondsToElapse) * 100 / badBonusCountdown);
    }
}

void Player::slot_removeBadBonus()
{
    m_badBonusCountdownTimer->stop();
    m_badBonusMillisecondsToElapse = 0;

    switch (m_badBonusType)
    {
        case Granatier::Bonus::HYPERACTIVE:
        case Granatier::Bonus::SLOW:
            {
                qreal askedXSpeedTemp = m_askedXSpeed;
                qreal askedYSpeedTemp = m_askedYSpeed;
                m_speed = m_normalSpeed;
                m_askedXSpeed = sign(m_xSpeed) * m_speed;
                m_askedYSpeed = sign(m_ySpeed) * m_speed;
                updateDirection();
                m_askedXSpeed = askedXSpeedTemp;
                m_askedYSpeed = askedYSpeedTemp;
            }
            break;
        case Granatier::Bonus::MIRROR:
            {
                qreal askedXSpeedTemp = m_askedXSpeed;
                qreal askedYSpeedTemp = m_askedYSpeed;
                m_askedXSpeed = -m_xSpeed;
                m_askedYSpeed = -m_ySpeed;
                switch(m_direction)
                {
                    case Granatier::Direction::EAST:
                        m_direction = Granatier::Direction::WEST;
                        break;
                    case Granatier::Direction::WEST:
                        m_direction = Granatier::Direction::EAST;
                        break;
                    case Granatier::Direction::NORTH:
                        m_direction = Granatier::Direction::SOUTH;
                        break;
                    case Granatier::Direction::SOUTH:
                        m_direction = Granatier::Direction::NORTH;
                        break;
                }
                updateDirection();
                m_askedXSpeed = -askedXSpeedTemp;
                m_askedYSpeed = -askedYSpeedTemp;

                QKeySequence tempKey = m_key.moveLeft;
                m_key.moveLeft = m_key.moveRight;
                m_key.moveRight = tempKey;
                tempKey = m_key.moveUp;
                m_key.moveUp = m_key.moveDown;
                m_key.moveDown = tempKey;

                m_moveMirrored = false;
            }
            break;
        case Granatier::Bonus::RESTRAIN:
            m_bombArmory = m_normalBombArmory;
            break;
        default:
            break;
    }

    Q_EMIT bonusUpdated(this, m_badBonusType, 100);
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
    if(m_death || m_falling)
    {
        return;
    }

    QKeySequence key = QKeySequence(keyEvent->key());

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
    else if(key == m_key.dropBomb)
    {
        if(m_bombArmory > 0)
        {
            emit bombDropped(this, m_x, m_y, true, 2);
            m_omitBombCurrentCell = true;
        }
        else
        {
            emit bombDropped(this, m_x, m_y, false, 2);
        }
    }

}

void Player::keyReleased(QKeyEvent* keyEvent)
{
    if(m_death || m_falling)
    {
        return;
    }

    QKeySequence key = QKeySequence(keyEvent->key());

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

    int nSpeed = 0;
    if(m_onIce)
    {
        nSpeed = onIceSpeedIncrease;
    }

    if(key == m_key.moveLeft && m_xSpeed < 0)
    {
        setXSpeed(-nSpeed);
    }
    else if(key == m_key.moveRight && m_xSpeed > 0)
    {
        setXSpeed(nSpeed);
    }
    else if(key == m_key.moveUp && m_ySpeed < 0)
    {
        setYSpeed(-nSpeed);
    }
    else if(key == m_key.moveDown && m_ySpeed > 0)
    {
        setYSpeed(nSpeed);
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
