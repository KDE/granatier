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

#include "bombexplosionitem.h"
#include "bomb.h"
#include "bombitem.h"
#include "cell.h"

BombExplosionItem::BombExplosionItem(Bomb* p_model, Direction direction, int bombPower) : QGraphicsSvgItem()
{
    m_direction = direction;
    m_bombPower = bombPower;
    m_explosionID = p_model->explosionID();
    
    switch(m_direction)
    {
        case NORTH:
            setElementId("bomb_blast_north_0");
            break;
        case EAST:
            setElementId("bomb_blast_east_0");
            break;
        case SOUTH:
            setElementId("bomb_blast_south_0");
            break;
        case WEST:
            setElementId("bomb_blast_west_0");
            break;
    }
    
    setVisible(true);
}

BombExplosionItem::~BombExplosionItem()
{
}

QPainterPath BombExplosionItem::shape() const
{
    QPainterPath path;
    // Temporary variable to keep the boundingRect available
    QRectF rect = boundingRect();

    // Calculation of the shape
    QRectF shapeRect = QRectF(rect.x(), rect.y(), rect.width(), rect.height());
    path.addEllipse(shapeRect);
    return path;
}

int BombExplosionItem::explosionID()
{
    return m_explosionID;
}

void BombExplosionItem::setPosition(qreal p_x, qreal p_y)
{
    qreal x;
    qreal y;
    
    QTransform transform;
    switch(m_direction)
    {
        case NORTH:
            x = p_x - boundingRect().width() / 2;
            y = p_y - (Cell::SIZE);
            setPos(x, y);
            transform.translate(boundingRect().width() / 2.0, 0);
            transform.translate(-boundingRect().width() / 2.0, 20);
            break;
        case EAST:
            x = p_x - (Cell::SIZE/2);
            y = p_y - boundingRect().height() / 2;
            setPos(x, y);
            transform.translate(0, boundingRect().height() / 2.0);
            transform.translate(-20, -boundingRect().height() / 2.0);
            break;
        case SOUTH:
            x = p_x - boundingRect().width() / 2;
            y = p_y - (Cell::SIZE/2);
            setPos(x, y);
            transform.translate(boundingRect().width() / 2.0, 0);
            transform.translate(-boundingRect().width() / 2.0, -20);
            break;
        case WEST:
            x = p_x - (Cell::SIZE);
            y = p_y - boundingRect().height() / 2;
            setPos(x, y);
            transform.translate(0, boundingRect().height() / 2.0);
            transform.translate(20, -boundingRect().height() / 2.0);
            break;
    }
    setTransform(transform);
}

void BombExplosionItem::updateAnimationn(int nFrame)
{
    QString strElemetId;
    switch(m_direction)
    {
        case NORTH:
            strElemetId = QString("bomb_blast_north_%1").arg(nFrame);
            break;
        case EAST:
            strElemetId = QString("bomb_blast_east_%1").arg(nFrame);
            break;
        case SOUTH:
            strElemetId = QString("bomb_blast_south_%1").arg(nFrame);
            break;
        case WEST:
            strElemetId = QString("bomb_blast_west_%1").arg(nFrame);
            break;
    }
    setElementId(strElemetId);
}
