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

#ifndef ELEMENT_H
#define ELEMENT_H

#include <QObject>

class Arena;
class Player;

/**
 * @brief This class describes the common characteristics and behaviour of any game Element (character or item).
 */
class Element : public QObject {

Q_OBJECT

public:

    /** The Element possible types */
    enum Type
    {
        BLOCK,
        PLAYER,
        BOMB,
        BONUS
    };
    
    enum Direction
    {
        NORTH,
        EAST,
        SOUTH,
        WEST
    };
    
protected:

    /** The Element type */
    Type m_type;

    /** The Element initial x-coordinate */
    qreal m_xInit;

    /** The Element initial y-coordinate */ 
    qreal m_yInit;

    /** The Element current x-coordinate */
    qreal m_x;

    /** The Element current y-coordinate */
    qreal m_y;

    /** The Arena the Element is on */
    Arena* m_arena;

    /** The Id of the Element */
    QString m_imageId;

public:

    /**
      * Creates a new Element instance.
      * @param p_x the initial x-coordinate
      * @param p_y the initial y-coordinate
      * @param p_arena the Arena the Element is on
      */
    Element(qreal p_x, qreal p_y, Arena* p_arena);

    /**
      * Deletes the Element instance.
      */
    ~Element();

    /**
      * Computes an action on a collision with the Player.
      * @param p_player the instance of Player which collides with the Element
      */
    virtual void doActionOnCollision(Player* p_player);

    /**
      * Sets arena for the element.
      * @param p_arena arena
      */
    void setArena(Arena* p_arena);
    
    /**
      * Sets the path initial position.
      * @param p_position initial position
      */
    void setInitialCoordinates (qreal p_x, qreal p_y);
    
    /**
      * Gets the path to the Element image.
      * @return the path to the Element image
      */
    QString getImageId() const;
    
    /**
      * Gets the type of the Element.
      * @return the Element type
      */
    Element::Type getType() const;

    /**
      * Sets the Element image.
      * @param p_imageId the image to set
      */
    void setImageId(const QString & p_imageId);

    /**
      * Gets the Element x-coordinate.
      * @return the x-coordinate
      */
    qreal getX() const;

    /**
      * Gets the Element y-coordinate.
      * @return the y-coordinate
      */
    qreal getY() const;

    /**
      * Sets the Element x-coordinate to the given value
      * @param p_x the x-coordinate to set
      */
    void setX(qreal p_x);

    /**
      * Sets the Element y-coordinate to the given value
      * @param p_y the y-coordinate to set
      */
    void setY(qreal p_y);

    /**
    * Initializes Element x-coordinate and y-coordinate with
    * initial values
    */
    void initCoordinate();

signals:

    /**
      * Emitted on Element move.
      * @param p_x the new x-coordinate
      * @param p_y the new y-coordinate
      */
    void moved(qreal p_x, qreal p_y);
};

#endif

