/*
    SPDX-FileCopyrightText: 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
    SPDX-FileCopyrightText: 2007-2008 Gaël Courcelle <gael.courcelle@gmail.com>
    SPDX-FileCopyrightText: 2007-2008 Alexia Allanic <alexia_allanic@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BLOCK_H
#define BLOCK_H

#include "element.h"

class Bonus;

/**
 * @brief This class represents a block with the possibility of a bonus inside
 */
class Block : public Element
{

    Q_OBJECT

public:

    /**
      * Creates a new Block instance.
      */
    Block(qreal p_x, qreal p_y, Arena* p_arena, const QString& p_imageId);

    /**
      * Deletes the Block instance.
      */
    ~Block();

    /**
      * Sets the Bonus which is contained by the Block
      * @param bonus the hidden Bonus
      */
    void setBonus(Bonus* bonus);

    /**
      * Returns the Bonus which is contained by the Block
      * @return the hidden Bonus
      */
    Bonus* getBonus();

    /**
      * destroys the block
      */
    void startDestruction();

private:
    Bonus* m_bonus;

Q_SIGNALS:
    void startDestructionAnimation();
};

#endif

