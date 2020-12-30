/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    ~CharacterItem() override;

    /**
      * Overrides the default shape function to make it a small circle
      * This function is used to determinate collision between items
      * @return QPainterPath the new shape of the Character
      */
    QPainterPath shape() const override;

public Q_SLOTS:

    /**
      * Updates the CharacterItem coordinates.
      * @param p_x the new x-coordinate
      * @param p_y the new y-coordinate
      */
    void update(qreal p_x, qreal p_y) override;

    /**
      * Sets the character dead.
      */
    virtual void setDead();
};

#endif

