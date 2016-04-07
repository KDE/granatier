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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "element.h"

Element::Element(qreal p_x, qreal p_y, Arena* p_arena) : m_xInit(p_x), m_yInit(p_y),  m_arena(p_arena)
{
    initCoordinate();
}

Element::~Element()
{
}

void Element::doActionOnCollision(Player*)
{
    // Do nothing by default : will be redefined within the subclasses
}

void Element::setArena(Arena* p_arena)
{
    m_arena = p_arena;
}

void Element::setInitialCoordinates (qreal p_x, qreal p_y)
{
    m_xInit = p_x;
    m_yInit = p_y;
}

qreal Element::getX() const
{
    return m_x;
}

qreal Element::getY() const
{
    return m_y;
}

void Element::setX(qreal p_x)
{
    m_x = p_x;
    emit(moved(m_x, m_y));
}

void Element::setY(qreal p_y)
{
    m_y = p_y;
    emit(moved(m_x, m_y));
}

Granatier::Element::Type Element::getType() const
{
    return m_type;
}

QString Element::getImageId() const
{
    return m_imageId;
}

void  Element::setImageId(const QString & p_imageId)
{
    m_imageId = p_imageId;
}

void Element::initCoordinate()
{
    setX(m_xInit);
    setY(m_yInit);
}
