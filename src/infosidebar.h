/*
 * Copyright 2009 Mathias Kraus <k.hias@gmx.de>
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

#ifndef INFOSIDEBAR_H
#define INFOSIDEBAR_H

#include "granatierglobals.h"

#include <QObject>
#include <QMap>
#include <QList>

class QGraphicsScene;
class QGraphicsTextItem;
class QGraphicsRectItem;
class Game;
class GameScene;
class Player;
class QRectF;
class KGameRenderedItem;

struct PlayerInfo
{
    QGraphicsTextItem* name;
    KGameRenderedItem* icon;
    KGameRenderedItem* bonusShield;
    QGraphicsRectItem* bonusShieldDimm;
    KGameRenderedItem* bonusThrow;
    QGraphicsRectItem* bonusThrowDimm;
    KGameRenderedItem* bonusKick;
    QGraphicsRectItem* bonusKickDimm;
    KGameRenderedItem* badBonus;
    QGraphicsRectItem* badBonusDimm;
};

/**
 * @brief This class represents the game sidebar with game information about the player.
 */
class InfoSidebar : public QObject
{

    Q_OBJECT

private:
    GameScene* m_gameScene;
    Game* m_game;

    QMap<Player*, PlayerInfo*> m_mapPlayerInfo;

    QGraphicsRectItem* m_background;

    qreal m_svgScaleFactor;

    QString m_badBonusSpriteKey;

public:

    /**
      * Creates a new InfoSidebar instance.
      * @param p_game the game instance
      * @param p_scene the gamescene instance
      */
    explicit InfoSidebar (Game* p_game, GameScene* p_scene);

    /**
      * Deletes the InfoSidebar instance.
      */
    virtual ~InfoSidebar();

    /**
      * Resets the player states
      */
    void reset();

    /**
      * Returns the background rect of the infoSidebar
      */
    QRectF rect();

    /**
      * Handles theme changes
      */
    void themeChanged();

private Q_SLOTS:
    /**
      * sets the bonus info
      * @param player the player which info changed
      * @param bonusType the bonus that was taken
      * @param percentageElapsed the bad bonus time that has elapsed
      */
    void bonusInfoChanged(Player* player, Granatier::Bonus::Type bonusType, int percentageElapsed);

public Q_SLOTS:
    /**
     * Updates the graphics after a resize
     * @param svgScaleFactor the scaling factor between svg and rendered pixmap
     */
    virtual void updateGraphics(qreal svgScaleFactor);
};

#endif

