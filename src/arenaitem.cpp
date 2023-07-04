/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
    SPDX-FileCopyrightText: 2007-2008 Pierre-Benoît Besse <besse.pb@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
= default;

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

#include "moc_arenaitem.cpp"
