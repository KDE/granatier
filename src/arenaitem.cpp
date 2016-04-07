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
#include "granatierglobals.h"

#include <QGraphicsView>
#include <KGameRenderer>

ArenaItem::ArenaItem(qreal p_x, qreal p_y, KGameRenderer* renderer, const QString& spriteKey) : KGameRenderedItem(renderer, spriteKey)
{
    // Init the view coordinates
    setPos(p_x, p_y);
}

ArenaItem::~ArenaItem()
{
}

void ArenaItem::updateGraphics(qreal svgScaleFactor)
{
    if(scene()->views().isEmpty())
    {
        return;
    }

    QPoint topLeft = scene()->views().first()->mapFromScene(0, 0);
    QPoint bottomRight = scene()->views().first()->mapFromScene(Granatier::CellSize, Granatier::CellSize);

    int width = bottomRight.x() - topLeft.x();
    int height = bottomRight.y() - topLeft.y();

    setRenderSize(QSize(width, height));
    setScale(svgScaleFactor);
}
