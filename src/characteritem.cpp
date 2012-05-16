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

#include "characteritem.h"
#include "character.h"

#include <KGameRenderer>

CharacterItem::CharacterItem(Character* p_model, KGameRenderer* renderer) : ElementItem (p_model, renderer)
{
    m_renderer = renderer;
    connect(p_model, SIGNAL(dead()), this, SLOT(setDead()));
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

