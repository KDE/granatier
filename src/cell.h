/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CELL_H
#define CELL_H

#include "element.h"

#include <QList>

/**
 * @brief This class represents a Cell of the Arena.
 */
class Cell
{
private:

    /** The Cell type */
    Granatier::Cell::Type m_type;

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
    bool isWalkable(Element* p_element) const;

    /**
      * Gets the Cell type.
      * @return the Cell type
      */
    Granatier::Cell::Type getType() const;

    /**
      * Sets the Cell type.
      * @param p_type the new type to set
      */
    void setType(Granatier::Cell::Type p_type);

    /**
     * Gets all the Elements that are on the Cell.
     * @return the Elements that are on the Cell
     */
    QList <Element*> getElements() const;

    /**
     * Gets the Elements of Element::Type type that are on the Cell.
     * @return the Elements of Element::Type type that are on the Cell
     */
    QList <Element*> getElements(Granatier::Element::Type type) const;

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

