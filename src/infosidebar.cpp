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
#include "gamescene.h"
#include "player.h"
#include "settings.h"

#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QTimer>
#include <KLocalizedString>
#include <KStandardDirs>
#include <KGameRenderer>
#include <KGameRenderedItem>

InfoSidebar::InfoSidebar (Game* p_game, GameScene* p_scene) : QObject()
{
    m_game = p_game;
    m_gameScene = p_scene;
    m_svgScaleFactor = 1;
    m_badBonusSpriteKey = "bonus_bad_restrain";
    
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
        playerName.setFont(QFont("Helvetica", Granatier::CellSize * 0.25, QFont::Bold, false));
        if(nMaxPlayerNameLength < playerName.boundingRect().width())
        {
            nMaxPlayerNameLength = playerName.boundingRect().width();
        }
        if(i == playerList.count() - 1)
        {
            int nAllItemsWidth = 4 * (Granatier::CellSize / 2 + 4);
            if(nAllItemsWidth > nMaxPlayerNameLength + Granatier::CellSize / 2 + 4)
            {
                nWidth = nAllItemsWidth;
            }
            else
            {
                nWidth = nMaxPlayerNameLength + Granatier::CellSize / 2 + 4;
            }
            nLeft = -(nWidth + 20);
            nHeight = playerName.boundingRect().height() + Granatier::CellSize / 2;
            nTop = m_gameScene->sceneRect().y() + m_gameScene->height()/2 - playerList.count()/2 * (nHeight + 4);
        }
    }
    
    //create the labels
    PlayerInfo* playerInfo;
    KGameRenderer* renderer;
    for(int i = 0; i < playerList.count(); i++)
    {
        QPen dimmRectPen;
        dimmRectPen.setWidth(0);
        dimmRectPen.setColor(QColor(0,0,0,200));
        QBrush dimmRectBrush(QColor(0,0,0,200));
        
        playerInfo = new PlayerInfo;
        playerInfo->name = 0;
        playerInfo->icon = 0;
        playerInfo->bonusShield = 0;
        playerInfo->bonusShieldDimm = 0;
        playerInfo->bonusThrow = 0;
        playerInfo->bonusThrowDimm = 0;
        playerInfo->bonusKick = 0;
        playerInfo->bonusKickDimm = 0;
        playerInfo->badBonus = 0;
        playerInfo->badBonusDimm = 0;
        
        //create the player icons
        renderer = m_gameScene->renderer(Granatier::Element::PLAYER, playerList[i]);
        if(renderer)
        {
            playerInfo->icon = new KGameRenderedItem(renderer, "player_0");
            playerInfo->icon->setZValue(1001);
            playerInfo->icon->setPos(nLeft, nTop + i * (nHeight + 4));
            m_gameScene->addItem(playerInfo->icon);
        }
        
        //create the player names
        playerInfo->name = new QGraphicsTextItem(playerList[i]->getPlayerName());
        playerInfo->name->setFont(QFont("Helvetica", Granatier::CellSize * 0.25, QFont::Bold, false));
        playerInfo->name->setDefaultTextColor(QColor("#FFFF00"));
        playerInfo->name->setZValue(1001);
        playerInfo->name->setPos(nLeft + Granatier::CellSize / 2 + 2, nTop + i * (nHeight+4) - 4);
        m_gameScene->addItem(playerInfo->name);
        
        renderer = m_gameScene->renderer(Granatier::Element::BONUS);
        if(renderer)
        {
            //create the bonus shield icons
            playerInfo->bonusShield = new KGameRenderedItem(renderer, "bonus_shield");
            playerInfo->bonusShield->setZValue(1001);
            playerInfo->bonusShield->setPos(nLeft, nTop + Granatier::CellSize / 2 + 1 + i * (nHeight + 4));
            m_gameScene->addItem(playerInfo->bonusShield);
            
            playerInfo->bonusShieldDimm = new QGraphicsRectItem();
            playerInfo->bonusShieldDimm->setBrush(dimmRectBrush);
            playerInfo->bonusShieldDimm->setPen(dimmRectPen);
            playerInfo->bonusShieldDimm->setZValue(1002);
            playerInfo->bonusShieldDimm->setPos(playerInfo->bonusShield->pos().x()-0.5, playerInfo->bonusShield->pos().y()-0.5);
            m_gameScene->addItem(playerInfo->bonusShieldDimm);
            
            //create the bonus throw icons
            playerInfo->bonusThrow = new KGameRenderedItem(renderer, "bonus_throw");
            playerInfo->bonusThrow->setZValue(1001);
            playerInfo->bonusThrow->setPos(nLeft + Granatier::CellSize / 2 + 4, nTop + Granatier::CellSize / 2 + 1 + i * (nHeight + 4));
            m_gameScene->addItem(playerInfo->bonusThrow);
            
            playerInfo->bonusThrowDimm = new QGraphicsRectItem();
            playerInfo->bonusThrowDimm->setBrush(dimmRectBrush);
            playerInfo->bonusThrowDimm->setPen(dimmRectPen);
            playerInfo->bonusThrowDimm->setZValue(1002);
            playerInfo->bonusThrowDimm->setPos(playerInfo->bonusThrow->pos().x()-0.5, playerInfo->bonusThrow->pos().y()-0.5);
            m_gameScene->addItem(playerInfo->bonusThrowDimm);
            
            //create the bonus kick icons
            playerInfo->bonusKick = new KGameRenderedItem(renderer, "bonus_kick");
            playerInfo->bonusKick->setZValue(1001);
            playerInfo->bonusKick->setPos(nLeft + 2 * (Granatier::CellSize / 2 + 4), nTop + Granatier::CellSize / 2 + 1 + i * (nHeight + 4));
            m_gameScene->addItem(playerInfo->bonusKick);
            
            playerInfo->bonusKickDimm = new QGraphicsRectItem();
            playerInfo->bonusKickDimm->setBrush(dimmRectBrush);
            playerInfo->bonusKickDimm->setPen(dimmRectPen);
            playerInfo->bonusKickDimm->setZValue(1002);
            playerInfo->bonusKickDimm->setPos(playerInfo->bonusKick->pos().x()-0.5, playerInfo->bonusKick->pos().y()-0.5);
            m_gameScene->addItem(playerInfo->bonusKickDimm);
            
            //create the bad bonus icons
            playerInfo->badBonus = new KGameRenderedItem(renderer, m_badBonusSpriteKey);
            playerInfo->badBonus->setZValue(1001);
            playerInfo->badBonus->setPos(nLeft + 3 * (Granatier::CellSize / 2 + 4), nTop + Granatier::CellSize / 2 + 1 + i * (nHeight + 4));
            m_gameScene->addItem(playerInfo->badBonus);
        
            playerInfo->badBonusDimm = new QGraphicsRectItem();
            playerInfo->badBonusDimm->setBrush(dimmRectBrush);
            playerInfo->badBonusDimm->setPen(dimmRectPen);
            playerInfo->badBonusDimm->setZValue(1002);
            playerInfo->badBonusDimm->setPos(playerInfo->badBonus->pos().x()-0.5, playerInfo->badBonus->pos().y()-0.5);
            m_gameScene->addItem(playerInfo->badBonusDimm);
        }
        
        m_mapPlayerInfo.insert(playerList[i], playerInfo);
        
        //connect player
        connect(playerList[i], SIGNAL(bonusUpdated(Player*,Granatier::Bonus::Type,int)), this, SLOT(bonusInfoChanged(Player*,Granatier::Bonus::Type,int)));
    }
    
    m_background = new QGraphicsRectItem();
    m_background->setBrush(QBrush(QColor(0,0,0,175)));
    m_background->setZValue(1000);
    m_background->setRect(nLeft - 10 , nTop - 10, nWidth + 16, playerList.count() * (nHeight + 4) + 16);
    m_gameScene->addItem(m_background);
}

InfoSidebar::~InfoSidebar()
{
    //Remove the player infosidebar
    QMap <Player*, PlayerInfo*>::iterator iteratorPlayerInfo = m_mapPlayerInfo.begin();
    while (iteratorPlayerInfo != m_mapPlayerInfo.end())
    {
        if(iteratorPlayerInfo.value())
        {
            //remove the icon
            if(m_gameScene->items().contains(iteratorPlayerInfo.value()->icon))
            {
                m_gameScene->removeItem(iteratorPlayerInfo.value()->icon);
            }
            delete iteratorPlayerInfo.value()->icon;
            //remove the name
            if(m_gameScene->items().contains(iteratorPlayerInfo.value()->name))
            {
                m_gameScene->removeItem(iteratorPlayerInfo.value()->name);
            }
            delete iteratorPlayerInfo.value()->name;
            //remove shield bonus
            if(m_gameScene->items().contains(iteratorPlayerInfo.value()->bonusShield))
            {
                m_gameScene->removeItem(iteratorPlayerInfo.value()->bonusShield);
            }
            delete iteratorPlayerInfo.value()->bonusShield;
            //remove shield bonus dimm
            if(m_gameScene->items().contains(iteratorPlayerInfo.value()->bonusShieldDimm))
            {
                m_gameScene->removeItem(iteratorPlayerInfo.value()->bonusShieldDimm);
            }
            delete iteratorPlayerInfo.value()->bonusShieldDimm;
            //remove throw bonus
            if(m_gameScene->items().contains(iteratorPlayerInfo.value()->bonusThrow))
            {
                m_gameScene->removeItem(iteratorPlayerInfo.value()->bonusThrow);
            }
            delete iteratorPlayerInfo.value()->bonusThrow;
            //remove throw bonus dimm
            if(m_gameScene->items().contains(iteratorPlayerInfo.value()->bonusThrowDimm))
            {
                m_gameScene->removeItem(iteratorPlayerInfo.value()->bonusThrowDimm);
            }
            delete iteratorPlayerInfo.value()->bonusThrowDimm;
            //remove kick bonus
            if(m_gameScene->items().contains(iteratorPlayerInfo.value()->bonusKick))
            {
                m_gameScene->removeItem(iteratorPlayerInfo.value()->bonusKick);
            }
            delete iteratorPlayerInfo.value()->bonusKick;
            //remove kick bonus dimm
            if(m_gameScene->items().contains(iteratorPlayerInfo.value()->bonusKickDimm))
            {
                m_gameScene->removeItem(iteratorPlayerInfo.value()->bonusKickDimm);
            }
            delete iteratorPlayerInfo.value()->bonusKickDimm;
            //remove bad bonus
            if(m_gameScene->items().contains(iteratorPlayerInfo.value()->badBonus))
            {
                m_gameScene->removeItem(iteratorPlayerInfo.value()->badBonus);
            }
            delete iteratorPlayerInfo.value()->badBonus;
            //remove bad bonus dimm
            if(m_gameScene->items().contains(iteratorPlayerInfo.value()->badBonusDimm))
            {
                m_gameScene->removeItem(iteratorPlayerInfo.value()->badBonusDimm);
            }
            delete iteratorPlayerInfo.value()->badBonusDimm;
            
            delete iteratorPlayerInfo.value();
        }
        iteratorPlayerInfo = m_mapPlayerInfo.erase(iteratorPlayerInfo);
    }
    
    //remove the background
    if(m_gameScene->items().contains(m_background))
    {
        m_gameScene->removeItem(m_background);
    }
    delete m_background;
    
    m_gameScene = 0;
    m_game = 0;
}

void InfoSidebar::reset()
{
    QList <Player*> playerList = m_game->getPlayers();
    for(int i = 0; i < playerList.count(); i++)
    {
        m_mapPlayerInfo.value(playerList[i])->bonusShieldDimm->setVisible(true);
        m_mapPlayerInfo.value(playerList[i])->bonusThrowDimm->setVisible(true);
        m_mapPlayerInfo.value(playerList[i])->bonusKickDimm->setVisible(true);
        m_mapPlayerInfo.value(playerList[i])->badBonusDimm->setVisible(true);
    }
}

QRectF InfoSidebar::rect()
{
    return m_background->rect();
}

void InfoSidebar::themeChanged()
{
    KGameRenderer* renderer = m_gameScene->renderer(Granatier::Element::BONUS);
    KGameRenderedItem* tempItem;
    
    //update player infosidebar
    QMap <Player*, PlayerInfo*>::iterator iteratorPlayerInfo = m_mapPlayerInfo.begin();
    while (iteratorPlayerInfo != m_mapPlayerInfo.end())
    {
        tempItem = iteratorPlayerInfo.value()->bonusShield;
        if(m_gameScene->items().contains(tempItem))
        {
            m_gameScene->removeItem(tempItem);
        }
        //create the bonus shield
        iteratorPlayerInfo.value()->bonusShield = new KGameRenderedItem(renderer, "bonus_shield");
        iteratorPlayerInfo.value()->bonusShield->setZValue(1001);
        iteratorPlayerInfo.value()->bonusShield->setPos(tempItem->pos());
        m_gameScene->addItem(iteratorPlayerInfo.value()->bonusShield);
        delete tempItem;
        
        tempItem = iteratorPlayerInfo.value()->bonusThrow;
        if(m_gameScene->items().contains(tempItem))
        {
            m_gameScene->removeItem(tempItem);
        }
        //create the bonus throw
        iteratorPlayerInfo.value()->bonusThrow = new KGameRenderedItem(renderer, "bonus_throw");
        iteratorPlayerInfo.value()->bonusThrow->setZValue(1001);
        iteratorPlayerInfo.value()->bonusThrow->setPos(tempItem->pos());
        m_gameScene->addItem(iteratorPlayerInfo.value()->bonusThrow);
        delete tempItem;
        
        tempItem = iteratorPlayerInfo.value()->bonusKick;
        if(m_gameScene->items().contains(tempItem))
        {
            m_gameScene->removeItem(tempItem);
        }
        //create the bonus kick
        iteratorPlayerInfo.value()->bonusKick = new KGameRenderedItem(renderer, "bonus_kick");
        iteratorPlayerInfo.value()->bonusKick->setZValue(1001);
        iteratorPlayerInfo.value()->bonusKick->setPos(tempItem->pos());
        m_gameScene->addItem(iteratorPlayerInfo.value()->bonusKick);
        delete tempItem;
        
        tempItem = iteratorPlayerInfo.value()->badBonus;
        if(m_gameScene->items().contains(tempItem))
        {
            m_gameScene->removeItem(tempItem);
        }
        //create the bad bonus
        iteratorPlayerInfo.value()->badBonus = new KGameRenderedItem(renderer, m_badBonusSpriteKey);
        iteratorPlayerInfo.value()->badBonus->setZValue(1001);
        iteratorPlayerInfo.value()->badBonus->setPos(tempItem->pos());
        m_gameScene->addItem(iteratorPlayerInfo.value()->badBonus);
        delete tempItem;
        
        iteratorPlayerInfo++;
    }
    
    updateGraphics(m_svgScaleFactor);
}

void InfoSidebar::bonusInfoChanged(Player* player, Granatier::Bonus::Type bonusType, int percentageElapsed)
{
    if(m_gameScene->views().isEmpty())
    {
        return;
    }
    
    switch((int)bonusType)
    {
        case Granatier::Bonus::SHIELD:
            m_mapPlayerInfo.value(player)->bonusShieldDimm->setVisible(percentageElapsed);
            break;
        case Granatier::Bonus::THROW:
            m_mapPlayerInfo.value(player)->bonusThrowDimm->setVisible(percentageElapsed);
            break;
        case Granatier::Bonus::KICK:
            m_mapPlayerInfo.value(player)->bonusKickDimm->setVisible(percentageElapsed);
            break;
        case Granatier::Bonus::HYPERACTIVE:
        case Granatier::Bonus::SLOW:
        case Granatier::Bonus::MIRROR:
        case Granatier::Bonus::SCATTY:
        case Granatier::Bonus::RESTRAIN:
            if(percentageElapsed == 0)
            {
                //set the new bad bonus icon
                KGameRenderer* renderer = m_gameScene->renderer(Granatier::Element::BONUS);
                if(renderer)
                {
                    switch((int)bonusType)
                    {
                        case Granatier::Bonus::HYPERACTIVE:
                            m_badBonusSpriteKey = "bonus_bad_hyperactive";
                            break;
                        case Granatier::Bonus::SLOW:
                            m_badBonusSpriteKey = "bonus_bad_slow";
                            break;
                        case Granatier::Bonus::MIRROR:
                            m_badBonusSpriteKey = "bonus_bad_mirror";
                            break;
                        case Granatier::Bonus::SCATTY:
                            m_badBonusSpriteKey = "bonus_bad_scatty";
                            break;
                        case Granatier::Bonus::RESTRAIN:
                            m_badBonusSpriteKey = "bonus_bad_restrain";
                            break;
                    }
                    
                    if(renderer->spriteExists(m_badBonusSpriteKey))
                    {
                        m_mapPlayerInfo.value(player)->badBonus->setSpriteKey(m_badBonusSpriteKey);
                    }
                    
                    //hide the dimm overlay
                    m_mapPlayerInfo.value(player)->badBonusDimm->setVisible(false);
                    
                    //calculate graphics size
                    QSize renderSize;
                    QPoint topLeft(0, 0);
                    topLeft = m_gameScene->views().first()->mapFromScene(topLeft);
                    QRectF rect;
                    
                    QPoint bottomRight(Granatier::CellSize * 0.5, Granatier::CellSize * 0.5); 
                    bottomRight = m_gameScene->views().first()->mapFromScene(bottomRight);
                    
                    renderSize.setHeight(bottomRight.y() - topLeft.y());
                    renderSize.setWidth(bottomRight.x() - topLeft.x());
                    
                    m_mapPlayerInfo.value(player)->badBonus->setRenderSize(renderSize);
                    m_mapPlayerInfo.value(player)->badBonus->setScale(m_svgScaleFactor);
                    
                    rect = m_mapPlayerInfo.value(player)->badBonusDimm->rect();
                    rect.setWidth(renderSize.width() * m_svgScaleFactor);
                    rect.setHeight(renderSize.height() * m_svgScaleFactor);
                    m_mapPlayerInfo.value(player)->badBonusDimm->setRect(rect);
                }
            }
            else
            {
                m_mapPlayerInfo.value(player)->badBonusDimm->setVisible(true);
                QRectF rect = m_mapPlayerInfo.value(player)->badBonusDimm->rect();
                rect.setHeight(rect.width() * percentageElapsed/100.0);
                m_mapPlayerInfo.value(player)->badBonusDimm->setRect(rect);
            }
            break;
    }
}

void InfoSidebar::updateGraphics(qreal svgScaleFactor)
{
    if(m_gameScene->views().isEmpty())
    {
        return;
    }
    
    QSize renderSize;
    QPoint topLeft(0, 0);
    topLeft = m_gameScene->views().first()->mapFromScene(topLeft);
    QPoint bottomRight;
    QRectF rect;
    
    m_svgScaleFactor = svgScaleFactor;
    
    //update player infosidebar
    QMap <Player*, PlayerInfo*>::iterator iteratorPlayerInfo = m_mapPlayerInfo.begin();
    while (iteratorPlayerInfo != m_mapPlayerInfo.end())
    {
        bottomRight.setX(Granatier::CellSize * 0.45);
        bottomRight.setY(Granatier::CellSize * 0.45);
        bottomRight = m_gameScene->views().first()->mapFromScene(bottomRight);
        renderSize.setHeight(bottomRight.y() - topLeft.y());
        renderSize.setWidth(bottomRight.x() - topLeft.x());
        
        iteratorPlayerInfo.value()->icon->setRenderSize(renderSize);
        iteratorPlayerInfo.value()->icon->setScale(m_svgScaleFactor);
        
        bottomRight.setX(Granatier::CellSize * 0.5);
        bottomRight.setY(Granatier::CellSize * 0.5);
        bottomRight = m_gameScene->views().first()->mapFromScene(bottomRight);
        renderSize.setHeight(bottomRight.y() - topLeft.y());
        renderSize.setWidth(bottomRight.x() - topLeft.x());
        
        iteratorPlayerInfo.value()->bonusShield->setRenderSize(renderSize);
        iteratorPlayerInfo.value()->bonusShield->setScale(m_svgScaleFactor);
        rect = iteratorPlayerInfo.value()->bonusShieldDimm->rect();
        rect.setWidth(renderSize.width() * m_svgScaleFactor);
        rect.setHeight(renderSize.height() * m_svgScaleFactor);
        iteratorPlayerInfo.value()->bonusShieldDimm->setRect(rect);
        
        iteratorPlayerInfo.value()->bonusThrow->setRenderSize(renderSize);
        iteratorPlayerInfo.value()->bonusThrow->setScale(m_svgScaleFactor);
        rect = iteratorPlayerInfo.value()->bonusThrowDimm->rect();
        rect.setWidth(renderSize.width() * m_svgScaleFactor);
        rect.setHeight(renderSize.height() * m_svgScaleFactor);
        iteratorPlayerInfo.value()->bonusThrowDimm->setRect(rect);
        
        iteratorPlayerInfo.value()->bonusKick->setRenderSize(renderSize);
        iteratorPlayerInfo.value()->bonusKick->setScale(m_svgScaleFactor);
        rect = iteratorPlayerInfo.value()->bonusKickDimm->rect();
        rect.setWidth(renderSize.width() * m_svgScaleFactor);
        rect.setHeight(renderSize.height() * m_svgScaleFactor);
        iteratorPlayerInfo.value()->bonusKickDimm->setRect(rect);
        
        iteratorPlayerInfo.value()->badBonus->setRenderSize(renderSize);
        iteratorPlayerInfo.value()->badBonus->setScale(m_svgScaleFactor);
        rect = iteratorPlayerInfo.value()->badBonusDimm->rect();
        rect.setWidth(renderSize.width() * m_svgScaleFactor);
        rect.setHeight(renderSize.height() * m_svgScaleFactor);
        iteratorPlayerInfo.value()->badBonusDimm->setRect(rect);
        
        iteratorPlayerInfo++;
    }
}
