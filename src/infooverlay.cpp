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

#include "infooverlay.h"
#include "game.h"
#include "player.h"
#include "settings.h"

#include <QTimer>
#include <QGraphicsScene>
#include <QGraphicsSvgItem>
#include <QGraphicsRectItem>
#include <KSvgRenderer>
#include <KStandardDirs>
#include <KLocale>

InfoOverlay::InfoOverlay (Game* p_game, KSvgRenderer* p_renderer, QGraphicsScene* p_scene)
{
    m_game = p_game;
    m_renderer = p_renderer;
    m_graphicsScene = p_scene;
    
    int nWinPoints = m_game->getWinPoints();
    QList <Player*> playerList = m_game->getPlayers();
    int nMaxPlayerNameLength = 0;
    int nTop = 0;
    int nLeft = 0;
    
    for(int i = 0; i < playerList.count(); i++)
    {
        QGraphicsTextItem playerName (playerList[i]->getPlayerName());
        playerName.setFont(QFont("Helvetica", 15, QFont::Bold, false));
        if(nMaxPlayerNameLength < playerName.boundingRect().width())
        {
            nMaxPlayerNameLength = playerName.boundingRect().width();
        }
        if(i == playerList.count() - 1)
        {
            QGraphicsSvgItem score;
            score.setElementId("score_star_enabled");
            score.setSharedRenderer(m_renderer);
            nLeft = m_graphicsScene->sceneRect().x() + m_graphicsScene->width()/2 - (nMaxPlayerNameLength + 10 + nWinPoints * (score.boundingRect().width()+2))/2;
            nTop = m_graphicsScene->sceneRect().y() + m_graphicsScene->height()/2 - (playerList.count()+2)/2 * playerName.boundingRect().height();
        }
    }
    
    for(int i = 0; i < playerList.count(); i++)
    {
        QList <QGraphicsSvgItem*> svgItemList;
        QGraphicsTextItem* playerName = new QGraphicsTextItem(playerList[i]->getPlayerName());
        playerName->setFont(QFont("Helvetica", 15, QFont::Bold, false));
        playerName->setDefaultTextColor(QColor("#FFFF00"));
        playerName->setZValue(1001);
        playerName->setPos(nLeft, nTop + i * (playerName->boundingRect().height()));
        
        for(int j = 0; j < nWinPoints; j++)
        {
            QGraphicsSvgItem* score = new QGraphicsSvgItem;
            score->setElementId("score_star_disabled");
            score->setSharedRenderer(m_renderer);
            score->setZValue(1001);
            score->setPos(nLeft + nMaxPlayerNameLength + 10 + j * (score->boundingRect().width()+2),
                          playerName->pos().y() + playerName->boundingRect().height()/2 - score->boundingRect().height()/2);
            svgItemList.append(score);
        }
        m_mapScore.insert(playerList[i], svgItemList);
        m_mapPlayerNames.insert(playerList[i], playerName);
    }
    
    QString strContinue (i18n("Press Space to continue"));
    
    m_continueLabel = new QGraphicsTextItem(strContinue);
    m_continueLabel->setFont(QFont("Helvetica", 15, QFont::Bold, false));
    m_continueLabel->setDefaultTextColor(QColor("#FFFF00"));
    m_continueLabel->setZValue(1001);
    m_continueLabel->setPos(m_graphicsScene->sceneRect().x() + (m_graphicsScene->width() - m_continueLabel->boundingRect().width())/2,
                            nTop + (playerList.count()+1) * (m_continueLabel->boundingRect().height()));
    
    m_newGameLabel = new QGraphicsTextItem(i18n("Press Space to start a new Game"));
    m_newGameLabel->setFont(QFont("Helvetica", 15, QFont::Bold, false));
    m_newGameLabel->setDefaultTextColor(QColor("#FFFF00"));
    m_newGameLabel->setZValue(1001);
    m_newGameLabel->setPos(m_graphicsScene->sceneRect().x() + (m_graphicsScene->width() - m_newGameLabel->boundingRect().width())/2,
                           nTop + (playerList.count()+1) * (m_newGameLabel->boundingRect().height()));
    
    m_gameOverLabel = new QGraphicsTextItem;;
    m_gameOverLabel->setFont(QFont("Helvetica", 20, QFont::Bold, false));
    m_gameOverLabel->setDefaultTextColor(QColor("#FFFF00"));
    m_gameOverLabel->setZValue(1001);
    m_gameOverLabel->setPos(m_graphicsScene->sceneRect().x() + (m_graphicsScene->width() - m_gameOverLabel->boundingRect().width())/2,
                            nTop - m_gameOverLabel->boundingRect().height());
   
    m_getReadyLabel = new QGraphicsTextItem(i18n("GET READY !!!"));
    m_getReadyLabel->setFont(QFont("Helvetica", 25, QFont::Bold, false));
    m_getReadyLabel->setDefaultTextColor(QColor("#FFFF00"));
    m_getReadyLabel->setZValue(1001);
    
    m_startGameLabel = new QGraphicsTextItem(i18n("Press Space to start"));
    m_startGameLabel->setFont(QFont("Helvetica", 15, QFont::Bold, false));
    m_startGameLabel->setDefaultTextColor(QColor("#FFFF00"));
    m_startGameLabel->setZValue(1001);
    
    m_getReadyLabel->setPos(m_graphicsScene->sceneRect().x() + (m_graphicsScene->width() - m_getReadyLabel->boundingRect().width()) / 2,
                            m_graphicsScene->sceneRect().y() + (m_graphicsScene->height() - m_getReadyLabel->boundingRect().height() -  m_startGameLabel->boundingRect().height()) / 2);
    m_startGameLabel->setPos(m_graphicsScene->sceneRect().x() + (m_graphicsScene->width() - m_startGameLabel->boundingRect().width()) / 2,
                             m_graphicsScene->sceneRect().y() + (m_graphicsScene->height() + m_getReadyLabel->boundingRect().height() - m_startGameLabel->boundingRect().height()) / 2);
    
    m_pauseLabel = new QGraphicsTextItem(i18n("PAUSED"));
    m_pauseLabel->setFont(QFont("Helvetica", 35, QFont::Bold, false));
    m_pauseLabel->setDefaultTextColor(QColor("#FFFF00"));
    m_pauseLabel->setZValue(1001);
    
    m_continueAfterPauseLabel = new QGraphicsTextItem(strContinue);
    m_continueAfterPauseLabel->setFont(QFont("Helvetica", 15, QFont::Bold, false));
    m_continueAfterPauseLabel->setDefaultTextColor(QColor("#FFFF00"));
    m_continueAfterPauseLabel->setZValue(1001);
    
    m_pauseLabel->setPos(m_graphicsScene->sceneRect().x() + (m_graphicsScene->width() - m_pauseLabel->boundingRect().width()) / 2,
                         m_graphicsScene->sceneRect().y() + (m_graphicsScene->height() - m_pauseLabel->boundingRect().height() - m_continueAfterPauseLabel->boundingRect().height()) / 2);
    m_continueAfterPauseLabel->setPos(m_graphicsScene->sceneRect().x() + (m_graphicsScene->width() - m_continueAfterPauseLabel->boundingRect().width()) / 2,
                                      m_graphicsScene->sceneRect().y() + (m_graphicsScene->height() + m_pauseLabel->boundingRect().height() - m_continueAfterPauseLabel->boundingRect().height()) / 2);
    
    m_dimmOverlay = new QGraphicsRectItem();
    m_dimmOverlay->setBrush(QBrush(QColor(0,0,0,175)));
    if(Settings::self()->showAllTiles() == 1)
    {
        m_dimmOverlay->setBrush(QBrush(QColor(0,0,0,0)));
    }
    m_dimmOverlay->setZValue(1000);
    m_dimmOverlay->setRect(m_graphicsScene->sceneRect().x(), m_graphicsScene->sceneRect().y(),
                           m_graphicsScene->width(), m_graphicsScene->height());
}

InfoOverlay::~InfoOverlay()
{
    //remove the items from the scene
    hideItems();
    
    // Find the score items and remove them
    QMap<Player*, QList<QGraphicsSvgItem*> >::iterator iteratorScore = m_mapScore.begin();
    while (iteratorScore != m_mapScore.end())
    {
        while(!iteratorScore.value().isEmpty())
        {
            delete iteratorScore.value().takeFirst();
        }
        iteratorScore = m_mapScore.erase(iteratorScore);
    }
    // Find the player name labels and remove them
    QMap<Player*, QGraphicsTextItem*>::iterator iteratorNames = m_mapPlayerNames.begin();
    while (iteratorNames != m_mapPlayerNames.end())
    {
        if(iteratorNames.value())
        {
            delete iteratorNames.value();
        }
        iteratorNames = m_mapPlayerNames.erase(iteratorNames);
    }
    
    if(m_continueLabel)
    {
        delete m_continueLabel;
    }
    
    if(m_newGameLabel)
    {
        delete m_newGameLabel;
    }
    
    if(m_gameOverLabel)
    {
        delete m_gameOverLabel;
    }
    
    if(m_getReadyLabel)
    {
        delete m_getReadyLabel;
    }
    
    if(m_startGameLabel)
    {
        delete m_startGameLabel;
    }
    
    if(m_pauseLabel)
    {
        delete m_pauseLabel;
    }
    
    if (m_dimmOverlay)
    {
        delete m_dimmOverlay;
    }
    
    if(m_continueAfterPauseLabel)
    {
        delete m_continueAfterPauseLabel;
    }
    if(m_renderer)
    {
        delete m_renderer;
    }
}

void InfoOverlay::showGetReady ()
{
    hideItems();
    
    if (!m_graphicsScene->items().contains(m_dimmOverlay))
    {
        m_graphicsScene->addItem(m_dimmOverlay);
    }
    
    if (!m_graphicsScene->items().contains(m_getReadyLabel))
    {
        m_graphicsScene->addItem(m_getReadyLabel);
    }
    
    if (!m_graphicsScene->items().contains(m_startGameLabel))
    {
        m_graphicsScene->addItem(m_startGameLabel);
    }
}

void InfoOverlay::showPause ()
{
    hideItems();
    
    if (!m_graphicsScene->items().contains(m_dimmOverlay))
    {
        m_graphicsScene->addItem(m_dimmOverlay);
    }
    
    if (!m_graphicsScene->items().contains(m_pauseLabel))
    {
        m_graphicsScene->addItem(m_pauseLabel);
    }
    
    if (!m_graphicsScene->items().contains(m_continueAfterPauseLabel))
    {
        m_graphicsScene->addItem(m_continueAfterPauseLabel);
    }
}

void InfoOverlay::showScore ()
{
    int nWinPoints = m_game->getWinPoints();
    QList <Player*> players = m_game->getPlayers();
    
    hideItems();
    
    if (!m_graphicsScene->items().contains(m_dimmOverlay))
    {
        m_graphicsScene->addItem(m_dimmOverlay);
    }
    
    for(int i = 0; i < players.length(); i++)
    {
        if (!m_graphicsScene->items().contains(m_mapPlayerNames.value(players[i])))
        {
            // display the player name
            m_graphicsScene->addItem(m_mapPlayerNames.value(players[i]));
        }
        
        for(int j = 0; j < nWinPoints; j++)
        {
            QGraphicsSvgItem* svgItem = m_mapScore.value(players[i]).at(j);
            if (players[i]->points() > j)
            {
                svgItem->setElementId("score_star_enabled");
            }
            // if the score was not displayed yet
            if (!m_graphicsScene->items().contains(svgItem))
            {
                // display the score
                m_graphicsScene->addItem(svgItem);
            }
        }
    }
    
    if(m_game->getGameOver() == false)
    {
        if (!m_graphicsScene->items().contains(m_continueLabel))
        {
            m_graphicsScene->addItem(m_continueLabel);
        }
    }
    else
    {
        if (!m_graphicsScene->items().contains(m_newGameLabel))
        {
            m_graphicsScene->addItem(m_newGameLabel);
        }
        if (m_graphicsScene->items().contains(m_gameOverLabel))
        {
            m_graphicsScene->removeItem(m_gameOverLabel);
        }
        m_gameOverLabel->setPlainText(i18n("The winner is %1", m_game->getWinner()));
        m_gameOverLabel->setPos(m_graphicsScene->sceneRect().x() + (m_graphicsScene->width() - m_gameOverLabel->boundingRect().width())/2,
                                m_gameOverLabel->y());
        m_graphicsScene->addItem(m_gameOverLabel);
    }
}

void InfoOverlay::hideItems ()
{
    int nWinPoints = m_game->getWinPoints();
    QList <Player*> players = m_game->getPlayers();
    
    for(int i = 0; i < players.length(); i++)
    {
        if (m_graphicsScene->items().contains(m_mapPlayerNames.value(players[i])))
        {
            // display the player name
            m_graphicsScene->removeItem(m_mapPlayerNames.value(players[i]));
        }
        
        for(int j = 0; j < nWinPoints; j++)
        {
            QGraphicsSvgItem* svgItem = m_mapScore.value(players[i]).at(j);
            // if the score is displayed
            if (m_graphicsScene->items().contains(svgItem))
            {
                // remove the score
                m_graphicsScene->removeItem(svgItem);
            }
        }
    }
    
    if (m_graphicsScene->items().contains(m_continueLabel))
    {
        m_graphicsScene->removeItem(m_continueLabel);
    }
    
    if (m_graphicsScene->items().contains(m_newGameLabel))
    {
        m_graphicsScene->removeItem(m_newGameLabel);
    }
    
    if (m_graphicsScene->items().contains(m_gameOverLabel))
    {
        m_graphicsScene->removeItem(m_gameOverLabel);
    }
    
    if (m_graphicsScene->items().contains(m_getReadyLabel))
    {
        m_graphicsScene->removeItem(m_getReadyLabel);
    }
    
    if (m_graphicsScene->items().contains(m_startGameLabel))
    {
        m_graphicsScene->removeItem(m_startGameLabel);
    }
    
    if (m_graphicsScene->items().contains(m_pauseLabel))
    {
        m_graphicsScene->removeItem(m_pauseLabel);
    }
    
    if (m_graphicsScene->items().contains(m_continueAfterPauseLabel))
    {
        m_graphicsScene->removeItem(m_continueAfterPauseLabel);
    }
    
    if (m_graphicsScene->items().contains(m_dimmOverlay))
    {
        m_graphicsScene->removeItem(m_dimmOverlay);
    }
}
