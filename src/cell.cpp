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
#include "element.h"

const qreal Cell::SIZE = 40.0;

Cell::Cell() : m_type(Cell::WALL), m_element(NULL), m_cost(0), m_parent(NULL)
{
}

Cell::~Cell()
{
}

bool Cell::isWalkable(Element* p_element) const
{
    if(p_element != NULL && p_element == m_element)
    {
        return true;
    }
    if(m_type == Cell::WALL)
    {
        return false;
    }
    else if(m_element != NULL && (m_element->getType() == Element::BLOCK || m_element->getType() == Element::BOMB))
    {
        return false;
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

Element* Cell::getElement() const
{
    return m_element;
}

void Cell::setElement(Element* p_element)
{
    m_element = p_element;
}

void Cell::removeElement(Element* p_element)
{
    if(p_element == m_element)
    {
        m_element = NULL;
    }
}

int Cell::getCost() const
{
    return m_cost;
}

void Cell::setCost(const int p_cost)
{
    m_cost = p_cost;
}

Cell* Cell::getParent() const
{
    return m_parent;
}

void Cell::setParent(Cell* p_parent) 
{
    m_parent = p_parent;
}
