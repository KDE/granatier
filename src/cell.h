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

#ifndef CELL_H
#define CELL_H

#include "element.h"

#include <QtGlobal>
#include <QList>

/**
 * @brief This class represents a Cell of the Arena.
 */
class Cell
{
public:

    /** The Cell side size */
    static const qreal SIZE;

    /** The Cell possible types */
    enum Type
    {
        WALL,
        GROUND,
        BLOCK,
        ICE,
        BOMBMORTAR,
        HOLE,
        ARROWUP,
        ARROWLEFT,
        ARROWDOWN,
        ARROWRIGHT
    };

private:

    /** The Cell type */
    Type m_type;

    /** A list of references of all Elements that are on the Cell */
    QList<Element*> m_elements;

public:

    /**
      * Creates a new Cell instance.
      */
    Cell();

    /**
      * Deletes the Cell instance.
      */
    ~Cell();

    /**
      * Returns if it is possible to move into the cell or not, because of a wall, bomb, etc.
      * @return true if it is possible to move into the cell
      */
    bool isWalkable(Element* p_element = NULL) const;

    /**
      * Gets the Cell type.
      * @return the Cell type
      */
    Type getType() const;

    /**
      * Sets the Cell type.
      * @param p_type the new type to set
      */
    void setType(Type p_type);

    /**
     * Gets all the Elements that are on the Cell.
     * @return the Elements that are on the Cell
     */
    QList <Element*> getElements() const;
    
    /**
     * Gets the Elements of Element::Type type that are on the Cell.
     * @return the Elements of Element::Type type that are on the Cell
     */
    QList <Element*> getElements(Element::Type type) const;

    /**
      * Sets the Element that is on the Cell.
      * @param p_element the Element to set on the Cell
      */
    void setElement(Element* p_element);

    /**
      * Removes the Element that is on the Cell.
      * @param p_element the Element to remove from the Cell
      */
    void removeElement(Element* p_element);
};

#endif

