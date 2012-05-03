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
#include <QMap>
#include "element.h"

class BlockItem;
class BonusItem;
class ArenaItem;
class Player;
class PlayerItem;
class Bomb;
class BombItem;
class BombExplosionItem;
class InfoOverlay;
class InfoSidebar;
class QTimer;

class KGameRenderer;
class KGameRenderedItem;
class KgThemeProvider;

/**
 * @brief This class contains all the Game elements to be drawn on the screen by the GameView instance.
 */
class GameScene : public QGraphicsScene {

Q_OBJECT
    
private:

    /** The min size for the gamescene */
    QRectF m_minSize;
    
    /** The Game instance */
    Game* m_game;

    /** The ArenaItems to be drawn */
    ArenaItem*** m_arenaItem;

    /** The PlayerItem of each Player to be drawn */
    QList<PlayerItem*> m_playerItems;
    
    /** The BombItem of each Bomb to be drawn */
    QHash<BombItem*, QList<BombExplosionItem*> > m_bombItems;
    
    /** The ElementItem to be drawn (each Block) */
    BlockItem*** m_blockItems;
    
    /** The Bonus ElementItem */
    BonusItem*** m_bonusItems;

    /** The Arena background */
    KGameRenderedItem* m_arenaBackground;
    
    /** The overlay to show the score at the end of a round*/
    InfoOverlay* m_infoOverlay;
    
    /** The sidebar to show the players game info*/
    InfoSidebar* m_infoSidebar;
    
    /** The labels to displayed the arena name */
    QGraphicsTextItem* m_arenaNameLabel;
    
    /** The labels to displayed the remaining time */
    QGraphicsTextItem* m_remainingTimeLabel;

    qreal m_SvgScaleFactor;
    
    /** The KGameRenderer */
    KGameRenderer* m_rendererSelectedTheme;
    KGameRenderer* m_rendererDefaultTheme;
    KGameRenderer* m_rendererBackground;
    KGameRenderer* m_rendererArenaItems;
    KGameRenderer* m_rendererBonusItems;
    KGameRenderer* m_rendererBombItems;
    KGameRenderer* m_rendererScoreItems;
    KGameRenderer* m_rendererInfoSidebar;
    QMap <Player*, KGameRenderer*> m_mapRendererPlayerItems;
    KgThemeProvider* m_themeProvider;
    QTimer* m_backgroundResizeTimer;
    
public:

    /**
      * Creates a new GameScene instance.
      * @param p_game the Game instance whose elements must be contained in the GameScene in order to be drawn
      */
    GameScene(Game* p_game, KgThemeProvider* p_themeProvider);
    
    /**
      * Deletes the Game instance.
      */
    ~GameScene();
    
    /**
      * @return the Game instance
      */
    Game* getGame() const;
    
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
     */
    void showScore();
    
    /**
     * Updates the sprites after a resize.
     */
    void resizeSprites();
    
    /**
     * Returns the renderer for the requestet Element Type
     * @param type
     * @param strPlayerID
     */
    KGameRenderer* renderer(Element::Type type, Player* player = NULL);
    
    /** setup the KGameRenderer for the selected theme and if necessary the default theme */
    void setupThemeRenderer();
    
private slots:
    
    /**
    * Updates the elements to be drawn when the Game starts.
    */
    void start();
    
    /**
     * Updates the background to fit into the QGraphicsView after a resize.
     */
    void resizeBackground();
    
    /**
    * Updates the elements to be drawn considering the Game state (paused or running).
    * @param p_pause if true the Game has been paused, if false the Game has been resumed
    * @param p_fromUser if true the Game has been paused due to an action from the user
    */
    void setPaused(const bool p_pause, const bool p_fromUser);
    
    /**
    * Remove the Block from the GameScene.
    */
    void removeBlockItem(BlockItem* blockItem);
    
    /**
    * Remove the Bonus from the GameScene.
    */
    void removeBonusItem(BonusItem* bonusItem);

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
    
    /**
    * Updates the BombExplosionItems
    */
    void updateBombExplosionItemAnimation(BombItem* bombItem, int nFrame);
    
signals:
    
    /**
     * Signals that the graphics need an update
     */
    void resizeGraphics(qreal svgScaleFactor);
};

#endif

