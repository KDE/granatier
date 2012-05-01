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

#include "cell.h"

const qreal Cell::SIZE = 40.0;

Cell::Cell() : m_type(Cell::WALL)
{
    m_elements.clear();
}

Cell::~Cell()
{
}

bool Cell::isWalkable(Element* p_element) const
{
    
    if(m_type == Cell::WALL)
    {
        return false;
    }
    else if(!m_elements.isEmpty())
    {
        foreach(Element* element, m_elements)
        {
            if(element->getType() == Element::BLOCK ||
                (element->getType() == Element::BOMB && p_element != element) ||
                (element->getType() == Element::PLAYER && p_element->getType() != Element::PLAYER))
            {
                return false;
            }
        }
    }
    return true;
}

Cell::Type Cell::getType() const
{
    return m_type;
}

void Cell::setType(Cell::Type p_type)
{
    m_type = p_type;
}

QList <Element*> Cell::getElements() const
{
    return m_elements;
}

QList <Element*> Cell::getElements(Element::Type type) const
{
    QList<Element*> elements;
    foreach(Element* element, m_elements)
    {
        if(element->getType() == type)
        {
            elements.append(element);
        }
    }
    return elements;
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
