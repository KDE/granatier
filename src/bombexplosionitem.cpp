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

#include <QGraphicsView>
#include <KGameRenderer>

BombExplosionItem::BombExplosionItem(Bomb* p_model, Granatier::Direction::Type direction, int bombPower, KGameRenderer* renderer, qreal svgScaleFactor) : KGameRenderedItem(renderer, QStringLiteral(""))
{
    m_direction = direction;
    m_explosionID = p_model->explosionID();

    m_svgScaleFactor = svgScaleFactor;

    int width = 1;
    int height = 1;
    switch(m_direction)
    {
        case Granatier::Direction::NORTH:
            setSpriteKey(QStringLiteral("bomb_blast_north_0"));
            width = static_cast<int>(Granatier::CellSize * 0.5);
            height = static_cast<int>(Granatier::CellSize * 1.5);
            break;
        case Granatier::Direction::EAST:
            setSpriteKey(QStringLiteral("bomb_blast_east_0"));
            width = static_cast<int>(Granatier::CellSize * 1.5);
            height = static_cast<int>(Granatier::CellSize * 0.5);
            break;
        case Granatier::Direction::SOUTH:
            setSpriteKey(QStringLiteral("bomb_blast_south_0"));
            width = static_cast<int>(Granatier::CellSize * 0.5);
            height = static_cast<int>(Granatier::CellSize * 1.5);
            break;
        case Granatier::Direction::WEST:
            setSpriteKey(QStringLiteral("bomb_blast_west_0"));
            width = static_cast<int>(Granatier::CellSize * 1.5);
            height = static_cast<int>(Granatier::CellSize * 0.5);
            break;
    }

    if((static_cast<int>(Granatier::CellSize) - width) % 2 != 0)
    {
        width--;
    }
    if((static_cast<int>(Granatier::CellSize) - height) % 2 != 0)
    {
        height--;
    }
    m_itemSizeSet = QSize(width, height);

    QSize svgSize = m_itemSizeSet / m_svgScaleFactor;
    setRenderSize(svgSize);
    setScale(m_svgScaleFactor);

    setZValue(303 + bombPower);
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
        case Granatier::Direction::NORTH:
            x = p_x - m_itemSizeSet.width() / 2;
            y = p_y - Granatier::CellSize/2;
            setPos(x, y);
            break;
        case Granatier::Direction::EAST:
            x = p_x - Granatier::CellSize;
            y = p_y - m_itemSizeSet.height() / 2;
            setPos(x, y);
            break;
        case Granatier::Direction::SOUTH:
            x = p_x - m_itemSizeSet.width() / 2;
            y = p_y - Granatier::CellSize;
            setPos(x, y);
            break;
        case Granatier::Direction::WEST:
            x = p_x - Granatier::CellSize/2;
            y = p_y - m_itemSizeSet.height() / 2;
            setPos(x, y);
            break;
    }
}

void BombExplosionItem::updateAnimationn(int nFrame)
{
    switch(m_direction)
    {
        case Granatier::Direction::NORTH:
            setSpriteKey(QStringLiteral("bomb_blast_north_%1").arg(nFrame));
            break;
        case Granatier::Direction::EAST:
            setSpriteKey(QStringLiteral("bomb_blast_east_%1").arg(nFrame));
            break;
        case Granatier::Direction::SOUTH:
            setSpriteKey(QStringLiteral("bomb_blast_south_%1").arg(nFrame));
            break;
        case Granatier::Direction::WEST:
            setSpriteKey(QStringLiteral("bomb_blast_west_%1").arg(nFrame));
            break;
    }
}

void BombExplosionItem::updateGraphics(qreal svgScaleFactor)
{
    if(scene()->views().isEmpty())
    {
        return;
    }

    //calculate the size of the item on the view
    QPoint viewTopLeft = scene()->views().first()->mapFromScene(0, 0);
    QPoint viewBottomRight = scene()->views().first()->mapFromScene(m_itemSizeSet.width(), m_itemSizeSet.height());

    int viewWidth = viewBottomRight.x() - viewTopLeft.x();
    int viewHeight = viewBottomRight.y() - viewTopLeft.y();

    //for better alignment, if the item size is different from the cell size, make the difference between the cell size and item size always even
    if(m_itemSizeSet.width() != Granatier::CellSize || m_itemSizeSet.height() != Granatier::CellSize)
    {
        viewBottomRight = scene()->views().first()->mapFromScene(Granatier::CellSize, Granatier::CellSize);
        int viewCellWidth = viewBottomRight.x() - viewTopLeft.x();
        int viewCellHeight = viewBottomRight.y() - viewTopLeft.y();
        if((viewCellWidth - viewWidth) % 2 != 0)
        {
            viewWidth--;
        }
        if((viewCellHeight - viewHeight) % 2 != 0)
        {
            viewHeight--;
        }

        //calculate the real item size after change of the render size
        QPointF sceneTopLeft = scene()->views().first()->mapToScene(0, 0);
        QPointF sceneBottomRight = scene()->views().first()->mapToScene(viewWidth, viewHeight);

        int sceneWidth = static_cast<int>(sceneBottomRight.x() - sceneTopLeft.x());
        int sceneHeight = static_cast<int>(sceneBottomRight.y() - sceneTopLeft.y());
        m_itemSizeReal = QSize(sceneWidth, sceneHeight);
    }

    setRenderSize(QSize(viewWidth, viewHeight));
    setScale(svgScaleFactor);
    m_svgScaleFactor = svgScaleFactor;

    QTransform transform;
    transform.translate(m_itemSizeSet.width() / 2.0, m_itemSizeSet.height() / 2.0);
    setRenderSize(renderSize());
    transform.translate(-m_itemSizeReal.width() / 2.0, -m_itemSizeReal.height() / 2.0);
    setTransform(transform);
}
