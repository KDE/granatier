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

#include "bonus.h"

#include <QObject>
#include <QMap>
#include <QList>

class QGraphicsScene;
class QGraphicsTextItem;
class QGraphicsSvgItem;
class QGraphicsRectItem;
class QSvgRenderer;
class Game;
class Player;
class QRectF;

/**
 * @brief This class represents the game sidebar with game information about the player.
 */
class InfoSidebar : public QObject
{

    Q_OBJECT
  
private:
    QGraphicsScene* m_graphicsScene;
    Game* m_game;
    QSvgRenderer* m_renderer;
    
    QMap <Player*, QGraphicsSvgItem*> m_mapPlayerSvgs;
    QMap <Player*, QGraphicsTextItem*> m_mapPlayerNames;
    QMap <Player*, QGraphicsSvgItem*> m_mapBonusThrowSvgs;
    QMap <Player*, QGraphicsRectItem*> m_mapBonusThrowDimm;
    QMap <Player*, QGraphicsSvgItem*> m_mapBonusKickSvgs;
    QMap <Player*, QGraphicsRectItem*> m_mapBonusKickDimm;
    QMap <Player*, QGraphicsSvgItem*> m_mapBonusShieldSvgs;
    QMap <Player*, QGraphicsRectItem*> m_mapBonusShieldDimm;
    QMap <Player*, QGraphicsSvgItem*> m_mapBadBonusSvgs;
    QMap <Player*, QGraphicsRectItem*> m_mapBadBonusDimm;
    
    QGraphicsRectItem* m_background;

public:

    /**
      * Creates a new InfoSidebar instance.
      * @param p_game the game instance
      * @param p_renderer the renderer instance
      * @param p_scene the gamescene instance
      */
    InfoSidebar (Game* p_game, QSvgRenderer* p_renderer, QGraphicsScene* p_scene);

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

private slots:
    /**
      * sets the bonus info
      * @param player the player which info changed
      * @param bonusType the bonus that was taken
      * @param percentageElapsed the bad bonus time that has elapsed
      */
    void bonusInfoChanged(Player* player, Bonus::BonusType bonusType, int percentageElapsed);
};

#endif

