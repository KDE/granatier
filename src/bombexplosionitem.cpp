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

#include <QGraphicsScene>
#include <QGraphicsView>
#include <KGameRenderer>

BombExplosionItem::BombExplosionItem(Bomb* p_model, Direction direction, int bombPower, KGameRenderer* renderer, qreal svgScaleFactor) : KGameRenderedItem(renderer, "")
{
    m_direction = direction;
    m_explosionID = p_model->explosionID();
    
    m_svgScaleFactor = svgScaleFactor;
    
    QString strElemetId;
    switch(m_direction)
    {
        case NORTH:
            setSpriteKey("bomb_blast_north_0");
            break;
        case EAST:
            setSpriteKey("bomb_blast_east_0");
            break;
        case SOUTH:
            setSpriteKey("bomb_blast_south_0");
            break;
        case WEST:
            setSpriteKey("bomb_blast_west_0");
            break;
    }
    
    QSize svgSize = renderer->boundsOnSprite(spriteKey()).size().toSize() / m_svgScaleFactor;
    setRenderSize(svgSize);
    setScale(m_svgScaleFactor);
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
    
    switch(m_direction)
    {
        case NORTH:
            x = p_x - renderer()->boundsOnSprite(spriteKey()).width() / 2;
            y = p_y - Cell::SIZE/2;
            setPos(x, y);
            break;
        case EAST:
            x = p_x - Cell::SIZE;
            y = p_y - renderer()->boundsOnSprite(spriteKey()).height() / 2;
            setPos(x, y);
            break;
        case SOUTH:
            x = p_x - renderer()->boundsOnSprite(spriteKey()).width() / 2;
            y = p_y - Cell::SIZE;
            setPos(x, y);
            break;
        case WEST:
            x = p_x - Cell::SIZE/2;
            y = p_y - renderer()->boundsOnSprite(spriteKey()).height() / 2;
            setPos(x, y);
            break;
    }
}

void BombExplosionItem::updateAnimationn(int nFrame)
{
    QString strElemetId;
    switch(m_direction)
    {
        case NORTH:
            setSpriteKey(QString("bomb_blast_north_%1").arg(nFrame));
            break;
        case EAST:
            setSpriteKey(QString("bomb_blast_east_%1").arg(nFrame));
            break;
        case SOUTH:
            setSpriteKey(QString("bomb_blast_south_%1").arg(nFrame));
            break;
        case WEST:
            setSpriteKey(QString("bomb_blast_west_%1").arg(nFrame));
            break;
    }
}

void BombExplosionItem::updateGraphics(qreal svgScaleFactor)
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
