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

#include "arenaitem.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <KGameRenderer>

ArenaItem::ArenaItem(qreal p_x, qreal p_y, KGameRenderer* renderer, const QString& spriteKey) : KGameRenderedItem(renderer, spriteKey)
{
    // Init the view coordinates
    setPos(p_x, p_y);
    // Configure the cache in order to increase the performance
    //setCacheMode(DeviceCoordinateCache);
    //setMaximumCacheSize(QSize(500, 500));
}

ArenaItem::~ArenaItem()
{
}

void ArenaItem::updateGraphics(qreal svgScaleFactor)
{
    QSize svgSize = renderer()->boundsOnSprite(spriteKey()).size().toSize();
    
    QPoint topLeft(0, 0);
    topLeft = scene()->views().at(0)->mapFromScene(topLeft);
    
    QPoint bottomRight(svgSize.width(), svgSize.height()); 
    bottomRight = scene()->views().at(0)->mapFromScene(bottomRight);
    
    svgSize.setHeight(bottomRight.y() - topLeft.y());
    svgSize.setWidth(bottomRight.x() - topLeft.x());
    
    //TODO: squeeze into a hard pixel grid
    //m_arenaItem[i][j]->setRenderSize(QSize(Cell::SIZE / m_SvgScaleFactor, Cell::SIZE / m_SvgScaleFactor));
    setRenderSize(svgSize);
    setScale(svgScaleFactor);
}
