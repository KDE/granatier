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

#ifndef CHARACTERITEM_H
#define CHARACTERITEM_H

#include "elementitem.h"

class Character;
class KGameRenderer;

/**
 * @brief This class is the graphical representation of a Character.
 */
class CharacterItem : public ElementItem {

Q_OBJECT

protected:
    /** Shared renderer for the player frames */
    KGameRenderer* m_renderer;

public:

    /**
      * Creates a new CharacterItem instance.
      * @param p_model the Character model
      * @param renderer the KGameRenderer
      */
    CharacterItem(Character* p_model, KGameRenderer* renderer);

    /**
      * Deletes the CharacterItem instance.
      */
    ~CharacterItem();

    /**
      * Overrides the default shape function to make it a small circle
      * This function is used to determinate collision between items
      * @return QPainterPath the new shape of the Character
      */
    QPainterPath shape() const;
    
public slots:

    /**
      * Updates the CharacterItem coordinates.
      * @param p_x the new x-coordinate
      * @param p_y the new y-coordinate
      */
    virtual void update(qreal p_x, qreal p_y);

    /**
      * Sets the character dead.
      */
    virtual void setDead();
};

#endif

