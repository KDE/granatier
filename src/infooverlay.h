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

#include <QMap>
#include <QList>

class QGraphicsScene;
class QGraphicsTextItem;
class QGraphicsSvgItem;
class QGraphicsRectItem;
class QTimer;
class KSvgRenderer;
class Game;
class Player;

/**
 * @brief This class the game info if paused or a round is over.
 */
class InfoOverlay
{
private:
    QGraphicsScene* m_graphicsScene;
    Game* m_game;
    KSvgRenderer* m_renderer;
    
    QMap <Player*, QList<QGraphicsSvgItem*> > m_mapScore;
    QMap <Player*, QGraphicsTextItem*> m_mapPlayerNames;
    QGraphicsTextItem* m_continueLabel;
    QGraphicsTextItem* m_newGameLabel;
    QGraphicsTextItem* m_gameOverLabel;
    
    QGraphicsTextItem* m_getReadyLabel;
    QGraphicsTextItem* m_startGameLabel;
    
    QGraphicsTextItem* m_pauseLabel;
    QGraphicsTextItem* m_continueAfterPauseLabel;
    
    QGraphicsRectItem* m_dimmOverlay;

public:

    /**
      * Creates a new InfoOverlay instance.
      * @param p_model the Character model
      */
    InfoOverlay (Game* p_game, KSvgRenderer* p_renderer, QGraphicsScene* p_scene);

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
};

#endif

