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

#include "infosidebar.h"
#include "game.h"
#include "player.h"
#include "settings.h"
#include "cell.h"

#include <QGraphicsScene>
#include <QGraphicsSvgItem>
#include <QGraphicsRectItem>
#include <KSvgRenderer>
#include <KLocale>
#include <KStandardDirs>

InfoSidebar::InfoSidebar (Game* p_game, KSvgRenderer* p_renderer, QGraphicsScene* p_scene)
{
    m_game = p_game;
    m_renderer = p_renderer;
    m_graphicsScene = p_scene;
    
    QList <Player*> playerList = m_game->getPlayers();
    int nMaxPlayerNameLength = 0;
    int nTop = 0;
    int nLeft = 0;
    int nWidth = 0;
    int nHeight = 0;
    
    //calculate max player name length and top-left position
    for(int i = 0; i < playerList.count(); i++)
    {
        QGraphicsTextItem playerName (playerList[i]->getPlayerName());
        playerName.setFont(QFont("Helvetica", 10, QFont::Bold, false));
        if(nMaxPlayerNameLength < playerName.boundingRect().width())
        {
            nMaxPlayerNameLength = playerName.boundingRect().width();
        }
        if(i == playerList.count() - 1)
        {
            int nAllItemsWidth = 5 * (Cell::SIZE / 2 + 2);
            if(nAllItemsWidth > nMaxPlayerNameLength + Cell::SIZE / 2 + 2)
            {
                nWidth = nAllItemsWidth;
            }
            else
            {
                nWidth = nMaxPlayerNameLength + Cell::SIZE / 2 + 2;
            }
            nLeft = -(nWidth + 20);
            nHeight = playerName.boundingRect().height() + Cell::SIZE / 2;
            nTop = m_graphicsScene->sceneRect().y() + m_graphicsScene->height()/2 - playerList.count()/2 * (nHeight + 4);
        }
    }
    
    //create the labels
    for(int i = 0; i < playerList.count(); i++)
    {
        //create the player icons
        QGraphicsSvgItem* svgItem = new QGraphicsSvgItem;
        KSvgRenderer* playerRenderer = new KSvgRenderer;
        playerRenderer->load(KStandardDirs::locate("appdata", QString("players/%1").arg(playerList[i]->getGraphicsPath())));
        svgItem->setSharedRenderer(playerRenderer);
        if(playerRenderer->elementExists("player_0"))
        {
            svgItem->setElementId("player_0");
            svgItem->setZValue(1);
            svgItem->setScale(0.5);
            svgItem->setPos(nLeft, nTop + i * (nHeight + 4));
            m_graphicsScene->addItem(svgItem);
            m_mapPlayerSvgs.insert(playerList[i], svgItem);
        }
        
        //create the player names
        QGraphicsTextItem* playerName = new QGraphicsTextItem(playerList[i]->getPlayerName());
        playerName->setFont(QFont("Helvetica", 10, QFont::Bold, false));
        playerName->setDefaultTextColor(QColor("#FFFF00"));
        playerName->setZValue(1);
        playerName->setPos(nLeft + Cell::SIZE / 2 + 2, nTop + i * (playerName->boundingRect().height() + Cell::SIZE / 2 + 4));
        m_graphicsScene->addItem(playerName);
        m_mapPlayerNames.insert(playerList[i], playerName);
        
        //create the bonus shield icons
        svgItem = new QGraphicsSvgItem;
        svgItem->setSharedRenderer(m_renderer);
        if(m_renderer->elementExists("bonus_shield"))
        {
            svgItem->setElementId("bonus_shield");
            svgItem->setZValue(1);
            svgItem->setScale(0.5);
            svgItem->setPos(nLeft, nTop + Cell::SIZE / 2 + 1 + i * (nHeight + 4));
            m_graphicsScene->addItem(svgItem);
            m_mapBonusShieldSvgs.insert(playerList[i], svgItem);
        }
        
        //create the bonus throw icons
        svgItem = new QGraphicsSvgItem;
        svgItem->setSharedRenderer(m_renderer);
        if(m_renderer->elementExists("bonus_throw"))
        {
            svgItem->setElementId("bonus_throw");
            svgItem->setZValue(1);
            svgItem->setScale(0.5);
            svgItem->setPos(nLeft + Cell::SIZE / 2 + 2, nTop + Cell::SIZE / 2 + 1 + i * (nHeight + 4));
            m_graphicsScene->addItem(svgItem);
            m_mapBonusThrowSvgs.insert(playerList[i], svgItem);
        }
        
        //create the bonus kick icons
        svgItem = new QGraphicsSvgItem;
        svgItem->setSharedRenderer(m_renderer);
        if(m_renderer->elementExists("bonus_kick"))
        {
            svgItem->setElementId("bonus_kick");
            svgItem->setZValue(1);
            svgItem->setScale(0.5);
            svgItem->setPos(nLeft + 2 * (Cell::SIZE / 2 + 2), nTop + Cell::SIZE / 2 + 1 + i * (nHeight + 4));
            m_graphicsScene->addItem(svgItem);
            m_mapBonusKickSvgs.insert(playerList[i], svgItem);
        }
        
        //create the bad bonus icons
        svgItem = new QGraphicsSvgItem;
        svgItem->setSharedRenderer(m_renderer);
        if(m_renderer->elementExists("bonus_bad_restrain"))
        {
            svgItem->setElementId("bonus_bad_restrain");
            svgItem->setZValue(1);
            svgItem->setScale(0.5);
            svgItem->setPos(nLeft + 3 * (Cell::SIZE / 2 + 2), nTop + Cell::SIZE / 2 + 1 + i * (nHeight + 4));
            m_graphicsScene->addItem(svgItem);
            m_mapBonusThrowSvgs.insert(playerList[i], svgItem);
        }
    }
    
    m_background = new QGraphicsRectItem();
    m_background->setBrush(QBrush(QColor(0,0,0,175)));
    m_background->setZValue(0);
    m_background->setRect(nLeft - 10 , nTop - 10, nWidth + 20, playerList.count() * (nHeight + 4) + 16);
    m_graphicsScene->addItem(m_background);
}

InfoSidebar::~InfoSidebar()
{
    //remove the items from the scene
    if(m_graphicsScene->items().contains(m_background))
    {
        m_graphicsScene->removeItem(m_background);
    }
    delete m_background;
    
    // Find the player name labels and remove them
    QMap<Player*, QGraphicsTextItem*>::iterator iteratorNames = m_mapPlayerNames.begin();
    while (iteratorNames != m_mapPlayerNames.end())
    {
        if(iteratorNames.value())
        {
            if(m_graphicsScene->items().contains(iteratorNames.value()))
            {
                m_graphicsScene->removeItem(iteratorNames.value());
            }
            delete iteratorNames.value();
        }
        iteratorNames = m_mapPlayerNames.erase(iteratorNames);
    }
    
    delete m_renderer;
}
