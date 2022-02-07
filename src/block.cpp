/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
    SPDX-FileCopyrightText: 2007-2008 Gaël Courcelle <gael.courcelle@gmail.com>
    SPDX-FileCopyrightText: 2007-2008 Alexia Allanic <alexia_allanic@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "block.h"

Block::Block(qreal p_x, qreal p_y, Arena* p_arena, const QString& p_imageId) : Element(p_x, p_y, p_arena)
{
    m_imageId = p_imageId;
    m_type = Granatier::Element::BLOCK;
    m_bonus = nullptr;
}

Block::~Block()
= default;

void Block::setBonus(Bonus* bonus)
{
    m_bonus = bonus;
}

Bonus* Block::getBonus()
{
    return m_bonus;
}

void Block::startDestruction()
{
    Q_EMIT startDestructionAnimation();
}
