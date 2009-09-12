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

#include <QTimer>
#include <KDebug>

BombExplosionItem::BombExplosionItem(Bomb* p_model, Direction direction, int i) : QGraphicsSvgItem()
{
    QTransform transform;
    m_direction = direction;
    switch(m_direction)
    {
        case NORTH:
            setElementId("bomb_exploded_north");
            transform.translate(0, 20);
            break;
        case EAST:
            setElementId("bomb_exploded_east");
            transform.translate(-20, 0);
            break;
        case SOUTH:
            setElementId("bomb_exploded_south");
            transform.translate(0, -20);
            break;
        case WEST:
            setElementId("bomb_exploded_west");
            transform.translate(20, 0);
            break;
    }
    setTransform(transform);
    
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
    QRectF shapeRect = QRectF( rect.x()+rect.width()/4, rect.y()+rect.height()/4, rect.width()/2, rect.height()/2 );
    path.addEllipse(shapeRect);
    return path;
}

void BombExplosionItem::update(qreal p_x, qreal p_y)
{
    // Compute the top-right coordinates of the item
    qreal x = p_x - boundingRect().width() / 2;
    qreal y = p_y - boundingRect().height() / 2;
    // Updates the view coordinates
    setPos(x, y);
}
