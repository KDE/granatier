/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "characteritem.h"
#include "character.h"

#include <KGameRenderer>

CharacterItem::CharacterItem(Character* p_model, KGameRenderer* renderer) : ElementItem (p_model, renderer)
{
    m_renderer = renderer;
    connect(p_model, &Character::dead, this, &CharacterItem::setDead);
}

CharacterItem::~CharacterItem()
{
}

QPainterPath CharacterItem::shape() const
{
    QPainterPath path;
    // Temporary variable to keep the boundingRect available
    QRectF rect = boundingRect();

    // Calculation of the shape
    QRectF shapeRect = QRectF( rect.x()+rect.width()/8, rect.y()+rect.height()/8, rect.width()*3.0/4.0, rect.height()*3.0/4.0 );
    path.addEllipse(shapeRect);
    return path;
}

void CharacterItem::update(qreal p_x, qreal p_y)
{
    // Compute the top-right coordinates of the item
    qreal x = p_x - m_itemSizeSet.width() / 2;
    qreal y = p_y - m_itemSizeSet.height() / 2;
    // Updates the view coordinates
    setPos(x, y);
}

void CharacterItem::setDead()
{
}

