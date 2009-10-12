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

#include <KSvgRenderer>

CharacterItem::CharacterItem(Character* p_model) : ElementItem (p_model)
{
    m_renderer = new KSvgRenderer();
    setSharedRenderer(m_renderer);
    connect(p_model, SIGNAL(dead()), this, SLOT(setDead()));
}

CharacterItem::~CharacterItem()
{
    delete m_renderer;
}

QPainterPath CharacterItem::shape() const
{
    QPainterPath path;
    // Temporary variable to keep the boundingRect available
    QRectF rect = boundingRect();

    // Calculation of the shape
    QRectF shapeRect = QRectF( rect.x()+rect.width()/4, rect.y()+rect.height()/4, rect.width()/2, rect.height()/2 );
    path.addEllipse(shapeRect);
    return path;
}

void CharacterItem::update(qreal p_x, qreal p_y)
{
    // Compute the top-right coordinates of the item
    qreal x = p_x - boundingRect().width() / 2;
    qreal y = p_y - boundingRect().height() / 2;
    // Updates the view coordinates
    setPos(x, y);
}

void CharacterItem::setDead()
{
}

