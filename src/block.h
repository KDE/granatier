/*
 * Copyright 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
 * Copyright 2007-2008 Gaël Courcelle <gael.courcelle@gmail.com>
 * Copyright 2007-2008 Alexia Allanic <alexia_allanic@yahoo.fr>
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
    void startDestruction(int nExplosionID);
        
private:
    Bonus* m_bonus;

signals:
    void startDestructionAnimation();
};

#endif

