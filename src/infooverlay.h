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

#ifndef INFOOVERLAY_H
#define INFOOVERLAY_H

#include <QObject>
#include <QMap>
#include <QList>

class QGraphicsTextItem;
class QGraphicsRectItem;
class Game;
class GameScene;
class Player;
class KGameRenderedItem;

/**
 * @brief This class the game info if paused or a round is over.
 */
class InfoOverlay : public QObject
{
    Q_OBJECT
    
private:
    GameScene* m_gameScene;
    Game* m_game;
    
    QMap <Player*, QList<KGameRenderedItem*> > m_mapScore;
    QMap <Player*, QGraphicsTextItem*> m_mapPlayerNames;
    QGraphicsTextItem* m_continueLabel;
    QGraphicsTextItem* m_newGameLabel;
    QGraphicsTextItem* m_gameOverLabel;
    
    QGraphicsTextItem* m_getReadyLabel;
    QGraphicsTextItem* m_startGameLabel;
    
    QGraphicsTextItem* m_pauseLabel;
    QGraphicsTextItem* m_continueAfterPauseLabel;
    
    QGraphicsRectItem* m_dimmOverlay;
    
    qreal m_svgScaleFactor;

public:

    /**
      * Creates a new InfoOverlay instance.
      * @param p_game the game instance
      * @param p_scene the gamescene instance
      */
    InfoOverlay (Game* p_game, GameScene* p_scene);

    /**
      * Deletes the InfoOverlay instance.
      */
    ~InfoOverlay();
    
    /**
      * Shows the get ready info.
      */
    void showGetReady();
    
    /**
      * Shows the pause info.
      */
    void showPause();
    
    /**
      * Shows the score info.
      */
    void showScore();
    
    /**
      * Hides the info items.
      */
    void hideItems();
    
    /**
      * Resize dimm overlay.
      * @param x the new left position
      * @param y the new top position
      * @param width the new width
      * @param height the new height
      */
    void resizeDimmOverlay(qreal x, qreal y, qreal width, qreal height);

    /**
     * Handles theme changes
     */
    void themeChanged();
    
public slots:
    /**
     * Updates the graphics after a resize
     * @param svgScaleFactor the scaling factor between svg and rendered pixmap
     */
    virtual void updateGraphics(qreal svgScaleFactor);
};

#endif

