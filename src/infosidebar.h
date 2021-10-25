/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INFOSIDEBAR_H
#define INFOSIDEBAR_H

#include "granatierglobals.h"

#include <QObject>
#include <QMap>

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
    ~InfoSidebar() override;

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

