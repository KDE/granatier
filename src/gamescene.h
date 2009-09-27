/*
 * Copyright 2009 Mathias Kraus <k.hias@gmx.de>
 * Copyright 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
 * Copyright 2007-2008 Alexandre Galinier <alex.galinier@hotmail.com>
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

#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QGraphicsScene>

#include <game.h>

#include <QList>
#include <QHash>

class ElementItem;
class ArenaItem;
class PlayerItem;
class Bomb;
class BombItem;
class BombExplosionItem;
class KPixmapCache;
class KSvgRenderer;

/**
 * @brief This class contains all the Game elements to be drawn on the screen by the GameView instance.
 */
class GameScene : public QGraphicsScene {

Q_OBJECT
    
private:

    /** The Game instance */
    Game* m_game;

    /** The ArenaItems to be drawn */
    ArenaItem*** m_arenaItem;

    /** The PlayerItem of each Player to be drawn */
    QList<PlayerItem*> m_playerItems;
    
    /** The current Points of each Player to be drawn */
    QList<QGraphicsTextItem*> m_playerPointsLabels;
    
    /** The BombItem of each Bomb to be drawn */
    QHash<BombItem*, QList<BombExplosionItem*> > m_bombItems;
    
    /** The ElementItem to be drawn (each Block) */
    ElementItem*** m_blockItems;
    
    /** The Bonus ElementItem */
    ElementItem*** m_bonusItems;

    /** The overlay to darken the game during pause*/
    QGraphicsRectItem* m_dimmOverlay;
    
    /** The labels to be displayed during the game */
    QGraphicsTextItem* m_introLabel;
    QGraphicsTextItem* m_introLabel2;
    QGraphicsTextItem* m_introLabel3;
    QGraphicsTextItem* m_pauseLabel;
    QGraphicsTextItem* m_remainingTime;

    /** The pixmap cache */
    KPixmapCache* m_cache; //TODO: check if the cache is used

    /** The SVG renderer */
    KSvgRenderer* m_rendererSelectedTheme;
    KSvgRenderer* m_rendererDefaultTheme;
    KSvgRenderer* m_rendererArenaItems;
    KSvgRenderer* m_rendererBonusItems;
    KSvgRenderer* m_rendererBombItems;
    
public:

    /**
      * Creates a new GameScene instance.
      * @param p_game the Game instance whose elements must be contained in the GameScene in order to be drawn
      */
    GameScene(Game* p_game);
    
    /**
      * Deletes the Game instance.
      */
    ~GameScene();
    
    /**
      * @return the Game instance
      */
    Game* getGame() const;

    /**
      * Loads the game theme.
      */
    void loadTheme();
    
    /**
     * Initializes class
     */
    void init();
    
    /**
     * Cleans class
     */
    void cleanUp();
    
    /**
     * Shows the labels with the points.
     * @param p_winPoints the points needed to win the game
     */
    void showScore(int p_winPoints);
    
private slots:
    
    /**
    * Updates the elements to be drawn when the Game starts.
    */
    void start();
    
    /**
    * Updates the elements to be drawn considering the Game state (paused or running).
    * @param p_pause if true the Game has been paused, if false the Game has been resumed
    * @param p_fromUser if true the Game has been paused due to an action from the user
    */
    void setPaused(const bool p_pause, const bool p_fromUser);
    
    /**
    * Remove the Bonus from the GameScene.
    */
    void removeBonusItem(ElementItem* bonusItem);

    /**
    * Upadates the Game information labels.
    * @param p_info the type of the information to be updated
    */
    void updateInfo(const Game::InformationTypes p_info);
    
    /**
    * Creates a BombItem
    */
    void createBombItem(Bomb* bomb);
    
    /**
    * Removes a BombItem
    */
    void removeBombItem(BombItem* bombItem);
    
    /**
    * Creates the explosion items
    */
    void bombDetonated(Bomb* bomb);
};

#endif

