/*
    SPDX-FileCopyrightText: 2011 Mathias Kraus <k.hias@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
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
