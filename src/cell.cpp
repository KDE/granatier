/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "cell.h"

Cell::Cell() : m_type(Granatier::Cell::WALL)
{
    m_elements.clear();
}

Cell::~Cell()
{
}

bool Cell::isWalkable(Element* p_element) const
{

    if(m_type == Granatier::Cell::WALL)
    {
        return false;
    }
    else if(!m_elements.isEmpty())
    {
        for(const auto& element: m_elements)
        {
            if(element->getType() == Granatier::Element::BLOCK ||
                (element->getType() == Granatier::Element::BOMB && (p_element == nullptr || p_element != element)) ||
                (element->getType() == Granatier::Element::PLAYER && (p_element == nullptr || p_element->getType() != Granatier::Element::PLAYER)))
            {
                return false;
            }
        }
    }
    return true;
}

Granatier::Cell::Type Cell::getType() const
{
    return m_type;
}

void Cell::setType(Granatier::Cell::Type p_type)
{
    m_type = p_type;
}

QList <Element*> Cell::getElements() const
{
    return m_elements;
}

QList <Element*> Cell::getElements(Granatier::Element::Type type) const
{
    QList<Element*> elementsFound;
    for(const auto& element: m_elements)
    {
        if(element->getType() == type)
        {
            elementsFound.append(element);
        }
    }
    return elementsFound;
}

void Cell::setElement(Element* p_element)
{
    if(!m_elements.contains(p_element))
    {
        m_elements.append(p_element);
    }
}

void Cell::removeElement(Element* p_element)
{
    int index = m_elements.indexOf(p_element);
    if(index >= 0)
    {
        m_elements.removeAt(index);
    }
}
