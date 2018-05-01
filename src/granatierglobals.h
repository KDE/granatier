/*
 *  <one line to give the program's name and a brief idea of what it does.>
 *  Copyright (C) 2011  Mathias Kraus <k.hias@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef GRANATIERGLOBALS_H
#define GRANATIERGLOBALS_H

#include <QtGlobal>

namespace Granatier
{
    /** The Frames Per Second for the game */
    constexpr int FPS = 60;

    /** The Cell size */
    constexpr qreal CellSize = 60.0;

    /** All possible graphic types */
    namespace GraphicItem
    {
        enum Type
        {
            NONE,
            HOLE,
            GROUND,
            WALL,
            BLOCK,
            ICE,
            BOMBMORTAR,
            ARROWUP,
            ARROWDOWN,
            ARROWLEFT,
            ARROWRIGHT,
            PLAYER,
            BOMB,
            BONUS,
            BONUS_ITEM_SPEED,
            BONUS_ITEM_BOMB,
            BONUS_ITEM_POWER,
            BONUS_ITEM_SHIELD,
            BONUS_ITEM_THROW,
            BONUS_ITEM_KICK,
            BONUS_ITEM_SLOW,
            BONUS_ITEM_HYPERACTIVE,
            BONUS_ITEM_MIRROR,
            BONUS_ITEM_SCATTY,
            BONUS_ITEM_RESTRAIN,
            BONUS_ITEM_RESURRECT,
            SCORE
        };
    }

    /** The Cell possible types */
    namespace Cell
    {
        enum Type
        {
            HOLE            = GraphicItem::HOLE,
            GROUND          = GraphicItem::GROUND,
            WALL            = GraphicItem::WALL,
            BLOCK           = GraphicItem::BLOCK,
            ICE             = GraphicItem::ICE,
            BOMBMORTAR      = GraphicItem::BOMBMORTAR,
            ARROWUP         = GraphicItem::ARROWUP,
            ARROWDOWN       = GraphicItem::ARROWDOWN,
            ARROWLEFT       = GraphicItem::ARROWLEFT,
            ARROWRIGHT      = GraphicItem::ARROWRIGHT
        };
    }

    /** The Element possible types */
    namespace Element
    {
        enum Type
        {
            NONE            = GraphicItem::NONE,
            BLOCK           = GraphicItem::BLOCK,
            PLAYER          = GraphicItem::PLAYER,
            BOMB            = GraphicItem::BOMB,
            BONUS           = GraphicItem::BONUS,
            SCORE           = GraphicItem::SCORE
        };
    }

    /** The Bonus possible types */
    namespace Bonus
    {
        enum Type
        {
            NONE            = GraphicItem::NONE,
            SPEED           = GraphicItem::BONUS_ITEM_SPEED,
            BOMB            = GraphicItem::BONUS_ITEM_BOMB,
            POWER           = GraphicItem::BONUS_ITEM_POWER,
            SHIELD          = GraphicItem::BONUS_ITEM_SHIELD,
            THROW           = GraphicItem::BONUS_ITEM_THROW,
            KICK            = GraphicItem::BONUS_ITEM_KICK,
            SLOW            = GraphicItem::BONUS_ITEM_SLOW,
            HYPERACTIVE     = GraphicItem::BONUS_ITEM_HYPERACTIVE,
            MIRROR          = GraphicItem::BONUS_ITEM_MIRROR,
            SCATTY          = GraphicItem::BONUS_ITEM_SCATTY,
            RESTRAIN        = GraphicItem::BONUS_ITEM_RESTRAIN,
            RESURRECT       = GraphicItem::BONUS_ITEM_RESURRECT,
        };
    }

    /** The possible directions */
    namespace Direction
    {
        enum Type
        {
            NORTH,
            SOUTH,
            EAST,
            WEST
        };
    }

    /** The different types of information about the game */
    namespace Info
    {
        enum Type
        {
            NoInfo = 0,
            TimeInfo = 1,     // Remaining time
            ArenaInfo = 2,    // The name of the current arena
            PlayerInfo = 4,   // Current level
            AllInfo = TimeInfo | ArenaInfo | PlayerInfo
        };
    }
}

#endif // GRANATIERGLOBALS_H
