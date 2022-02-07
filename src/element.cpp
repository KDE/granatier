/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "element.h"

Element::Element(qreal p_x, qreal p_y, Arena* p_arena) : m_xInit(p_x), m_yInit(p_y),  m_arena(p_arena)
{
    initCoordinate();
}

Element::~Element()
= default;

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
    Q_EMIT moved(m_x, m_y);
}

void Element::setY(qreal p_y)
{
    m_y = p_y;
    Q_EMIT moved(m_x, m_y);
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
