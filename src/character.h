/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
    SPDX-FileCopyrightText: 2007-2008 Pierre-Benoît Besse <besse.pb@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CHARACTER_H
#define CHARACTER_H

#include "element.h"

#include <QKeySequence>


/**
 * @brief This class describes the common characteristics and behaviour of the game characters (Players).
 */
class Character : public Element {

Q_OBJECT

public:

    /** Keyboard shortcuts for moving and dropping a bomb */
    struct Shortcuts
    {
        QKeySequence moveLeft;
        QKeySequence moveRight;
        QKeySequence moveUp;
        QKeySequence moveDown;
        QKeySequence dropBomb;
    };
    Shortcuts m_key;

protected:

    /** The Character x-speed */
    qreal m_xSpeed;

    /** The Character y-speed */
    qreal m_ySpeed;

    /** The character speed */
    qreal m_speed;

    /** Reference to the speed of the character when in "normal" behaviour */
    qreal m_normalSpeed;

    /** The maximum character speed */
    qreal m_maxSpeed;

public:

    /**
      * Creates a new Character instance.
      * @param p_x the initial x-coordinate
      * @param p_y the initial y-coordinate
      * @param p_arena the Arena the Character is on
      */
    Character(qreal p_x, qreal p_y, Arena* p_arena);

    /**
      * Deletes the Character instance.
      */
    ~Character() override;

    /**
      * Makes the Character go up.
      */
    virtual void goUp() = 0;

    /**
      * Makes the Character go down.
      */
    virtual void goDown() = 0;

    /**
      * Makes the Character go to the right.
      */
    virtual void goRight() = 0;

    /**
      * Makes the Character go to the left.
      */
    virtual void goLeft() = 0;

    /**
      * Updates the Character move.
      */
    virtual void updateMove() = 0;

    /**
      * Moves the Character function of its current coordinates and speed.
      * If the character reaches a border, it circles around the arena and continue its way from the other side.
      */
    void move();

    /**
      * Manages the character death (essentially blinking).
      */
    void die();

    /**
      * Gets the Character x-speed value.
      * @return the x-speed value
      */
    qreal getXSpeed() const;

    /**
      * Gets the Character y-speed value.
      * @return the y-speed value
      */
    qreal getYSpeed() const;

    /**
      * Gets the Character speed.
      * @return the character speed
      */
    qreal getSpeed() const;

    /**
      * Gets the Character normal speed.
      * @return the character speed reference, when in "normal" behaviour
      */
    qreal getNormalSpeed() const;

    /**
      * Set the Character x-speed value.
      * @param p_xSpeed the x-speed to set
      */
      void setXSpeed(qreal p_xSpeed);

    /**
      * Set the Character y-speed value.
      * @param p_ySpeed the y-speed to set
      */
      void setYSpeed(qreal p_ySpeed);

    /**
      * Initializes the Character speed considering the difficulty level.
      */
    void initSpeed();

    /**
      * Checks the Character is in the line of sight of the given other Character.
      * @param p_character the other Character
      * @return true if the Character is in the same line than the given one
      */
    bool isInLineSight(Character* p_character) const;

Q_SIGNALS:

    /**
      * Emitted when the character is dead.
      */
    void dead();
};

#endif

