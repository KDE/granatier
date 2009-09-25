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

#include "gamescene.h"
#include "cell.h"
#include "bonus.h"
#include "settings.h"
#include "game.h"
#include "player.h"
#include "block.h"
#include "bomb.h"
#include "arena.h"
#include "elementitem.h"
#include "arenaitem.h"
#include "playeritem.h"
#include "bombitem.h"
#include "bombexplosionitem.h"

#include <QTimer>
#include <KGameTheme>
#include <KLocale>
#include <KPixmapCache>
#include <KSvgRenderer>

GameScene::GameScene(Game* p_game) : m_game(p_game)
{
    connect(p_game, SIGNAL(gameStarted()), this, SLOT(start()));
    connect(p_game, SIGNAL(pauseChanged(bool, bool)), this, SLOT(setPaused(bool, bool)));
    connect(p_game, SIGNAL(bombCreated(Bomb*)), this, SLOT(createBombItem(Bomb*)));

    // Set the pixmap cache limit to improve performance
    setItemIndexMethod(NoIndex);
    m_cache = new KPixmapCache("granatier_cache");
    m_cache->setCacheLimit(3 * 1024);

    // Load the SVG file
    m_renderer = new KSvgRenderer();
    loadTheme();

    // Create the PlayerItems and the points labels
    QList <Player*> players = p_game->getPlayers();
    PlayerItem* playerItem;
    for(int i = 0; i < players.size(); i++)
    {
        playerItem = new PlayerItem(players[i]);
        // Corrects the position of the player
        playerItem->update(players[i]->getX(), players[i]->getY());
        playerItem->setZValue(210+i);
        // Stops the player animation
        playerItem->stopAnim();
        
        m_playerItems.append(playerItem);
        
        connect (playerItem, SIGNAL(bonusItemTaken(ElementItem*)), this, SLOT(removeBonusItem(ElementItem*)));
        
        m_playerPointsLabels.append(new QGraphicsTextItem(players[i]->getPlayerName()));
        m_playerPointsLabels.last()->setFont(QFont("Helvetica", 15, QFont::Bold, false));
        m_playerPointsLabels.last()->setDefaultTextColor(QColor("#FFFF00"));
        m_playerPointsLabels.last()->setZValue(1001);
    }

    // Create the labels background
    m_dimmOverlay = new QGraphicsRectItem();
    m_dimmOverlay->setBrush(QBrush(QColor(0,0,0,175)));
    m_dimmOverlay->setZValue(1000);
    // Create the introduction labels
    m_introLabel = new QGraphicsTextItem(i18n("GET READY !!!"));
    m_introLabel->setFont(QFont("Helvetica", 25, QFont::Bold, false));
    m_introLabel->setDefaultTextColor(QColor("#FFFF00"));
    m_introLabel->setZValue(1001);
    m_introLabel2 = new QGraphicsTextItem(i18n("Press Space to start"));
    m_introLabel2->setFont(QFont("Helvetica", 15, QFont::Bold, false));
    m_introLabel2->setDefaultTextColor(QColor("#FFFF00"));
    m_introLabel2->setZValue(1001);
    m_introLabel3 = new QGraphicsTextItem(i18n("Press Space to continue ..."));
    m_introLabel3->setFont(QFont("Helvetica", 15, QFont::Bold, false));
    m_introLabel3->setDefaultTextColor(QColor("#FFFF00"));
    m_introLabel3->setZValue(1001);
    // Create the pause label
    m_pauseLabel = new QGraphicsTextItem(i18n("PAUSED"));
    m_pauseLabel->setFont(QFont("Helvetica", 35, QFont::Bold, false));
    m_pauseLabel->setDefaultTextColor(QColor("#FFFF00"));
    m_pauseLabel->setZValue(1001);

    // Display each PlayerItem
    for (int i = 0; i < m_playerItems.size(); i++)
    {
        addItem(m_playerItems[i]);
    }

    // Initialize the information labels (score, lives and label)
    // Display the score label
    //addItem(m_scoreLabel);
    //m_scoreLabel->setPos(Cell::SIZE, height() + Cell::SIZE);
    // Display the lives label
    //addItem(m_livesLabel);
    //m_livesLabel->setPos(width() - m_livesLabel->boundingRect().width() - 20 , height() - Cell::SIZE - m_livesLabel->boundingRect().height() / 2);
    // Display the level label
    //addItem(m_levelLabel);
    //m_levelLabel->setPos((width() - m_levelLabel->boundingRect().width()) / 2 , height() - Cell::SIZE - m_levelLabel->boundingRect().height() / 2);

    init();
}

void GameScene::init()
{
    // Create the ArenaItems
    m_arenaItem = new ArenaItem**[m_game->getArena()->getNbRows()];
    for (int i = 0; i < m_game->getArena()->getNbRows(); ++i)
    {
        m_arenaItem[i] = new ArenaItem*[m_game->getArena()->getNbColumns()];
        for (int j = 0; j < m_game->getArena()->getNbColumns(); ++j)
        {
            // Create the ArenaItem and set the image
            ArenaItem* arenaItem = new ArenaItem(j * Cell::SIZE, i * Cell::SIZE);
            arenaItem->setSharedRenderer(m_renderer);
            //TODO: use this function call
            //arenaItem->setElementId(m_game->getArena()->getCell(i,j).getElement()->getImageId());
            switch(m_game->getArena()->getCell(i,j).getType())
            {
                case Cell::WALL:
                    arenaItem->setElementId("arena_wall");
                    arenaItem->setZValue(-2);
                    break;
                case Cell::HOLE:
                    delete arenaItem;
                    arenaItem = NULL;
                    break;
                case Cell::GROUND:
                default:
                    arenaItem->setElementId("arena_ground");
                    arenaItem->setZValue(-1);
            }
            m_arenaItem[i][j] = arenaItem;
        }
    }
    
    // Create the Block and Bonus items
    m_blockItems = new ElementItem**[m_game->getArena()->getNbRows()];
    m_bonusItems = new ElementItem**[m_game->getArena()->getNbRows()];
    for (int i = 0; i < m_game->getArena()->getNbRows(); ++i)
    {
        m_blockItems[i] = new ElementItem*[m_game->getArena()->getNbColumns()];
        m_bonusItems[i] = new ElementItem*[m_game->getArena()->getNbColumns()];
        for (int j = 0; j < m_game->getArena()->getNbColumns(); ++j)
        {
            if (m_game->getArena()->getCell(i, j).getElement() != NULL && m_game->getArena()->getCell(i, j).getElement()->getType() == Element::BLOCK)
            {
                // Create the element item and set the image
                Element* element = m_game->getArena()->getCell(i, j).getElement();
                ElementItem* elementItem = new ElementItem(element);
                elementItem->setSharedRenderer(m_renderer);
                elementItem->setElementId(element->getImageId());
                elementItem->update(element->getX(), element->getY());
                elementItem->setZValue(200);
                m_blockItems[i][j] = elementItem;
                // if the block contains a hidden bonus, create the bonus item 
                Bonus* bonus = (dynamic_cast <Block*> (element))->getBonus();
                if(bonus)
                {
                  
                    ElementItem* bonusItem = new ElementItem(bonus);
                    bonusItem->setSharedRenderer(m_renderer);
                    switch(bonus->getBonusType())
                    {
                        case Bonus::SPEED:  bonusItem->setElementId("bonus_speed");
                                            break;
                        case Bonus::BOMB:   bonusItem->setElementId("bonus_bomb");
                                            break;
                        case Bonus::RANGE:  bonusItem->setElementId("bonus_range");
                                            break;
                        case Bonus::THROW:  bonusItem->setElementId("bonus_throw");
                                            break;
                        case Bonus::MOVE:   bonusItem->setElementId("bonus_move");
                                            break;
                        default:            bonusItem->setElementId("bonus_speed");
                    }
                    bonusItem->update(bonus->getX(), bonus->getY());
                    bonusItem->setZValue(100);
                    m_bonusItems[i][j] = bonusItem;
                }
                else
                {
                    m_bonusItems[i][j] = NULL;
                }
            }
            else
            {
                m_blockItems[i][j] = NULL;
                m_bonusItems[i][j] = NULL;
            }
        }
    }
    
    // Display the ArenaItem
    for (int i = 0; i < m_game->getArena()->getNbRows();++i)
    {
        for (int j = 0; j < m_game->getArena()->getNbColumns(); ++j)
        {
            if(m_arenaItem[i][j] != NULL)
            {
                addItem(m_arenaItem[i][j]);
            }
        }
    }
    
    // Display the Block Items
    for (int i = 0; i < m_game->getArena()->getNbRows(); ++i)
    {
        for (int j = 0; j < m_game->getArena()->getNbColumns(); ++j)
        {
            if (m_blockItems[i][j] != NULL)
            {
                if (!items().contains(m_blockItems[i][j]))
                {
                    addItem(m_blockItems[i][j]);
                }
            }
        }
    }
    
    // Display the introduction label
    if (!items().contains(m_introLabel))
    {
        addItem(m_introLabel);
    }
    m_introLabel->setPos((width() - m_introLabel->boundingRect().width()) / 2, (height() - m_introLabel->boundingRect().height()) / 2);
    
    if (!items().contains(m_introLabel2))
    {
        addItem(m_introLabel2);
    }
    m_introLabel2->setPos((width() - m_introLabel2->boundingRect().width()) / 2, (height() - m_introLabel2->boundingRect().height() + m_introLabel->boundingRect().height()) / 2);
    
    if (!items().contains(m_dimmOverlay))
    {
        addItem(m_dimmOverlay);
    }
    m_dimmOverlay->setRect(0, 0, width(), height());
}

GameScene::~GameScene()
{
    cleanUp();
    
    for (int i = 0; i < m_playerItems.size(); i++)
    {
        if(items().contains(m_playerItems[i]))
        {
            removeItem(m_playerItems[i]);
        }
        delete m_playerItems[i];
        delete m_playerPointsLabels[i];
    }
    
    delete m_introLabel;
    delete m_introLabel2;
    delete m_introLabel3;
    delete m_pauseLabel;
    delete m_dimmOverlay;
    
    delete m_cache;
    delete m_renderer;
}

void GameScene::cleanUp()
{
    // remove the arena items
    for (int i = 0; i < m_game->getArena()->getNbRows();++i)
    {
        for (int j = 0; j < m_game->getArena()->getNbColumns(); ++j)
        {
            if (m_arenaItem[i][j] != NULL)
            {
                if (items().contains(m_arenaItem[i][j]))
                {
                    removeItem(m_arenaItem[i][j]);
                }
                delete m_arenaItem[i][j];
            }
        }
        delete[] m_arenaItem[i];
    }
    delete[] m_arenaItem;
    
    // Find the BombItem and remove it
    BombExplosionItem* bombExplosionItem;
    QHash<BombItem*, QList<BombExplosionItem*> >::iterator i = m_bombItems.begin();
    while (i != m_bombItems.end())
    {
        while(!i.value().isEmpty())
        {
            bombExplosionItem = i.value().takeFirst();
            if(items().contains(bombExplosionItem))
            {
                removeItem(bombExplosionItem);
            }
            delete bombExplosionItem;
        }
        if(items().contains(i.key()))
        {
            removeItem(i.key());
        }
        delete i.key();
        i = m_bombItems.erase(i);
    }
    
    // Find the BlockItems and BonusItems and remove it
    for (int i = 0; i < m_game->getArena()->getNbRows();++i)
    {
        for (int j = 0; j < m_game->getArena()->getNbColumns(); ++j)
        {
            if (m_blockItems[i][j] != NULL)
            {
                if (items().contains(m_blockItems[i][j]))
                {
                    removeItem(m_blockItems[i][j]);
                }
                delete m_blockItems[i][j];
            }
            if (m_bonusItems[i][j] != NULL)
            {
                if (items().contains(m_bonusItems[i][j]))
                {
                    removeItem(m_bonusItems[i][j]);
                }
                delete m_bonusItems[i][j];
            }
        }
        delete[] m_blockItems[i];
        delete[] m_bonusItems[i];
    }
    delete[] m_blockItems;
    delete[] m_bonusItems;
    
    // remove the player points labels from the gamescene
    for (int i = 0; i < m_playerPointsLabels.size(); i++)
    {
        if(items().contains(m_playerPointsLabels[i]))
        {
            removeItem(m_playerPointsLabels[i]);
        }
    }
    if(items().contains(m_introLabel3))
    {
        removeItem(m_introLabel3);
    }
}

void GameScene::showScore(int p_winPoints)
{
    QList <Player*> players = m_game->getPlayers();
    for(int i = 0; i < players.length(); i++)
    {
        // If the label was not displayed yet
        if (!items().contains(m_playerPointsLabels[i]))
        {
            // Display the pause label
            addItem(m_playerPointsLabels[i]);
        }
        m_playerPointsLabels[i]->setPlainText(QString("%1 %2").arg(players[i]->getPlayerName()).arg(QString(players[i]->points(), QChar('X')), -p_winPoints, QChar('O')));
        m_playerPointsLabels[i]->setPos((width() - m_playerPointsLabels[i]->boundingRect().width()) / 2, (height() - (players.length() - i) * m_playerPointsLabels[i]->boundingRect().height()) / 2);
    }
    
    if (!items().contains(m_introLabel3))
    {
        addItem(m_introLabel3);
    }
    m_introLabel3->setPos((width() - m_introLabel3->boundingRect().width()) / 2, (height() + m_introLabel3->boundingRect().height()) / 2);
}

Game* GameScene::getGame() const
{
    return m_game;
}

void GameScene::loadTheme()
{
    KGameTheme theme;
    if (!theme.load(Settings::self()->theme()))
    {
        return;
    }
    if (!m_renderer->load(theme.graphics()))
    {
        return;
    }
    m_cache->discard();
    update(0, 0, width(), height());

    // Update the theme config: if the default theme is selected, no theme entry is written -> the theme selector does not select the theme
    Settings::self()->config()->group("General").writeEntry("Theme", Settings::self()->theme());
}

void GameScene::start()
{
    // If the introduction and new level labels were displayed then remove them
    if (items().contains(m_introLabel))
    {
        removeItem(m_introLabel);
    }
    if (items().contains(m_introLabel2))
    {
        removeItem(m_introLabel2);
    }
    if (items().contains(m_dimmOverlay))
    {
        removeItem(m_dimmOverlay);
    }
}

void GameScene::setPaused(const bool p_pause, const bool p_fromUser)
{
    // If the game has paused
    if (p_pause)
    {
        // If the pause is due to an action from the user
        if (p_fromUser)
        {
            // If the label was not displayed yet
            if (!items().contains(m_pauseLabel))
            {
                // Display the pause label
                addItem(m_pauseLabel);
            }
            m_pauseLabel->setPos((width() - m_pauseLabel->boundingRect().width()) / 2, (height() - m_pauseLabel->boundingRect().height()) / 2);
        }
        if (!items().contains(m_dimmOverlay))
        {
            addItem(m_dimmOverlay);
        }
        m_dimmOverlay->setRect(0, 0, width(), height());
        // Stop player animation
        for (int i = 0; i < m_playerItems.size(); i++)
        {
            m_playerItems[i]->pauseAnim();
        }
        // Stop bomb animation
        QHash<BombItem*, QList<BombExplosionItem*> >::iterator i = m_bombItems.begin();
        while (i != m_bombItems.end())
        {
            i.key()->pauseAnim();
            ++i;
        }
    }
    else
    {   // If the game has resumed
        // If the pause was due to an action from the user
        if (p_fromUser) {
            // If the label was displayed
            if (items().contains(m_pauseLabel))
            {
                removeItem(m_pauseLabel);
            }
        }
        if (items().contains(m_dimmOverlay))
        {
            removeItem(m_dimmOverlay);
        }
        // Resume player animation
        for (int i = 0; i < m_playerItems.size(); i++)
        {
            m_playerItems[i]->resumeAnim();
        }
        // Resume bomb animation
        QHash<BombItem*, QList<BombExplosionItem*> >::iterator i = m_bombItems.begin();
        while (i != m_bombItems.end())
        {
            i.key()->resumeAnim();
            ++i;
        }
    }
}

void GameScene::removeBonusItem(ElementItem* bonusItem)
{
    // remove the Bonus Items
    for (int i = 0; i < m_game->getArena()->getNbRows(); ++i)
    {
        for (int j = 0; j < m_game->getArena()->getNbColumns(); ++j)
        {
            if (m_bonusItems[i][j] != NULL && m_bonusItems[i][j] == bonusItem)
            {
                if (items().contains(m_bonusItems[i][j]))
                {
                    removeItem(m_bonusItems[i][j]);
                    m_bonusItems[i][j] = NULL;
                    m_game->removeBonus(dynamic_cast <Bonus*> (bonusItem->getModel()));
                    delete bonusItem;
                }
            }
        }
    }
}

void GameScene::updateInfo(const Game::InformationTypes p_info)
{
}

void GameScene::createBombItem(Bomb* bomb)
{
    // Create the Bombs
    BombItem* bombItem = new BombItem(bomb);
    bombItem->setSharedRenderer(m_renderer);
    bombItem->setElementId("bomb");
    // Corrects the position of the BombItem
    bombItem->update(bomb->getX(), bomb->getY());
    bombItem->setZValue(200);
    addItem(bombItem);
    m_bombItems[bombItem].append(NULL);
    
    connect(bomb, SIGNAL(bombDetonated(Bomb*)), this, SLOT(bombDetonated(Bomb*)));
    connect(bombItem, SIGNAL(bombItemFinished(BombItem*)), this, SLOT(removeBombItem(BombItem*)));
}

void GameScene::removeBombItem(BombItem* bombItem)
{
    m_game->removeBomb(dynamic_cast <Bomb*> (bombItem->getModel()));
    // Find the BombItem and remove it
    BombExplosionItem* bombExplosionItem;
    QHash<BombItem*, QList<BombExplosionItem*> >::iterator i = m_bombItems.begin();
    while (i != m_bombItems.end())
    {
        if(i.key() == bombItem)
        {
            while(!i.value().isEmpty())
            {
                bombExplosionItem = i.value().takeFirst();
                if(items().contains(bombExplosionItem))
                {
                    removeItem(bombExplosionItem);
                }
                delete bombExplosionItem;
            }
            if(items().contains(i.key()))
            {
                removeItem(i.key());
            }
            delete i.key();
            i = m_bombItems.erase(i);
            break;
        }
        else
        {
            ++i;
        }
    }
}

void GameScene::bombDetonated(Bomb* bomb)
{
    BombItem* bombItem = NULL;
    BombExplosionItem* bombExplosionItem = NULL;
    Element* element = NULL;
    int nBombRange = bomb->bombRange();
    int nNumberOfColums = m_game->getArena()->getNbColumns();
    int nNumberOfRows = m_game->getArena()->getNbRows();
    int nColumn;
    int nRow;
    bool bNorthDone = false;
    bool bEastDone = false;
    bool bSouthDone = false;
    bool bWestDone = false;
    int nDetonationCountdown = 75;
    int nDetonationCountdownNorth = nDetonationCountdown;
    int nDetonationCountdownEast = nDetonationCountdown;
    int nDetonationCountdownSouth = nDetonationCountdown;
    int nDetonationCountdownWest = nDetonationCountdown;
    
    // Find the BombItem from the Bomb
    QHash<BombItem*, QList<BombExplosionItem*> >::iterator i = m_bombItems.begin();
    while (i != m_bombItems.end())
    {
        if(i.key()->getModel() == bomb)
        {
            bombItem = i.key();
            break;
        }
        ++i;
    }
    
    if(!bombItem)
    {
        return;
    }
    
    for(int i = 0; i < nBombRange; i++)
    {
        // north
        nColumn = m_game->getArena()->getColFromX(bomb->getX());
        nRow = m_game->getArena()->getRowFromY(bomb->getY() - (i+1)*Cell::SIZE);
        if(!bNorthDone && nColumn >= 0 && nColumn < nNumberOfColums && nRow >= 0 && nRow < nNumberOfRows)
        {
            element = m_game->getArena()->getCell(nRow, nColumn).getElement();
            if(m_game->getArena()->getCell(nRow, nColumn).isWalkable() ||
                (element && (element->getType() == Element::BOMB || element->getType() == Element::BLOCK)))
            {
                if(element && element->getType() == Element::BOMB && !(dynamic_cast <Bomb*> (element)->isDetonated()))
                {
                    dynamic_cast <Bomb*> (element)->setDetonationCountdown(nDetonationCountdownNorth);
                    nDetonationCountdownNorth += nDetonationCountdown;
                }
                else if(element && element->getType() == Element::BLOCK)
                {
                    bNorthDone = true;
                    if (m_blockItems[nRow][nColumn] != NULL)
                    {
                        removeItem(m_blockItems[nRow][nColumn]);
                        m_game->blockDestroyed(nRow, nColumn, dynamic_cast <Block*> (element));
                        delete m_blockItems[nRow][nColumn]; //this will also delete the block instance
                        m_blockItems[nRow][nColumn] = NULL;
                        //display bonus if available
                        if (m_bonusItems[nRow][nColumn] != NULL)
                        {
                            if (!items().contains(m_bonusItems[nRow][nColumn]))
                            {
                                addItem(m_bonusItems[nRow][nColumn]);
                            }
                        }
                    }
                }
                bombExplosionItem = new BombExplosionItem (bomb, BombExplosionItem::NORTH, i);
                bombExplosionItem->setSharedRenderer(m_renderer);
                bombExplosionItem->update(bomb->getX(), bomb->getY() - (i+1)*Cell::SIZE);
                bombExplosionItem->setZValue(300 + nBombRange+3 - i);
                addItem(bombExplosionItem);
                m_bombItems[bombItem].append(bombExplosionItem);
            }
            else
            {
                bNorthDone = true;
            }
        }
        else
        {
          bNorthDone = true;
        }
        // east
        nColumn = m_game->getArena()->getColFromX(bomb->getX() + (i+1)*Cell::SIZE);
        nRow = m_game->getArena()->getRowFromY(bomb->getY());
        if(!bEastDone && nColumn >= 0 && nColumn < nNumberOfColums && nRow >= 0 && nRow < nNumberOfRows)
        {
            element = m_game->getArena()->getCell(nRow, nColumn).getElement();
            if(m_game->getArena()->getCell(nRow, nColumn).isWalkable() ||
                (element && (element->getType() == Element::BOMB || element->getType() == Element::BLOCK)))
            {
                if(element && element->getType() == Element::BOMB && !(dynamic_cast <Bomb*> (element)->isDetonated()))
                {
                    dynamic_cast <Bomb*> (element)->setDetonationCountdown(nDetonationCountdownEast);
                    nDetonationCountdownEast += nDetonationCountdown;
                }
                else if(element && element->getType() == Element::BLOCK)
                {
                    bEastDone = true;
                    if (m_blockItems[nRow][nColumn] != NULL)
                    {
                        removeItem(m_blockItems[nRow][nColumn]);
                        m_game->blockDestroyed(nRow, nColumn, dynamic_cast <Block*> (element));
                        delete m_blockItems[nRow][nColumn]; //this will also delete the block instance
                        m_blockItems[nRow][nColumn] = NULL;
                        //display bonus if available
                        if (m_bonusItems[nRow][nColumn] != NULL)
                        {
                            if (!items().contains(m_bonusItems[nRow][nColumn]))
                            {
                                addItem(m_bonusItems[nRow][nColumn]);
                            }
                        }
                    }
                }
                bombExplosionItem = new BombExplosionItem (bomb, BombExplosionItem::EAST, i);
                bombExplosionItem->setSharedRenderer(m_renderer);
                bombExplosionItem->update(bomb->getX() + (i+1)*Cell::SIZE, bomb->getY());
                bombExplosionItem->setZValue(300 + nBombRange+3 - i);
                addItem(bombExplosionItem);
                m_bombItems[bombItem].append(bombExplosionItem);
            }
            else
            {
                bEastDone = true;
            }
        }
        else
        {
          bEastDone = true;
        }
        // south
        nColumn = m_game->getArena()->getColFromX(bomb->getX());
        nRow = m_game->getArena()->getRowFromY(bomb->getY() + (i+1)*Cell::SIZE);
        if(!bSouthDone && nColumn >= 0 && nColumn < nNumberOfColums && nRow >= 0 && nRow < nNumberOfRows)
        {
            element = m_game->getArena()->getCell(nRow, nColumn).getElement();
            if(m_game->getArena()->getCell(nRow, nColumn).isWalkable() ||
                (element && (element->getType() == Element::BOMB || element->getType() == Element::BLOCK)))
            {
                if(element && element->getType() == Element::BOMB && !(dynamic_cast <Bomb*> (element)->isDetonated()))
                {
                    dynamic_cast <Bomb*> (element)->setDetonationCountdown(nDetonationCountdownSouth);
                    nDetonationCountdownSouth += nDetonationCountdown;
                }
                else if(element && element->getType() == Element::BLOCK)
                {
                    bSouthDone = true;
                    if (m_blockItems[nRow][nColumn] != NULL)
                    {
                        removeItem(m_blockItems[nRow][nColumn]);
                        m_game->blockDestroyed(nRow, nColumn, dynamic_cast <Block*> (element));
                        delete m_blockItems[nRow][nColumn]; //this will also delete the block instance
                        m_blockItems[nRow][nColumn] = NULL;
                        //display bonus if available
                        if (m_bonusItems[nRow][nColumn] != NULL)
                        {
                            if (!items().contains(m_bonusItems[nRow][nColumn]))
                            {
                                addItem(m_bonusItems[nRow][nColumn]);
                            }
                        }
                    }
                }
                bombExplosionItem = new BombExplosionItem (bomb, BombExplosionItem::SOUTH, i);
                bombExplosionItem->setSharedRenderer(m_renderer);
                bombExplosionItem->update(bomb->getX(), bomb->getY() + (i+1)*Cell::SIZE);
                bombExplosionItem->setZValue(300 + nBombRange+3 - i);
                addItem(bombExplosionItem);
                m_bombItems[bombItem].append(bombExplosionItem);
            }
            else
            {
                bSouthDone = true;
            }
        }
        else
        {
          bSouthDone = true;
        }
        //west
        nColumn = m_game->getArena()->getColFromX(bomb->getX() - (i+1)*Cell::SIZE);
        nRow = m_game->getArena()->getRowFromY(bomb->getY());
        if(!bWestDone && nColumn >= 0 && nColumn < nNumberOfColums && nRow >= 0 && nRow < nNumberOfRows)
        {
            element = m_game->getArena()->getCell(nRow, nColumn).getElement();
            if(m_game->getArena()->getCell(nRow, nColumn).isWalkable() ||
                (element && (element->getType() == Element::BOMB || element->getType() == Element::BLOCK)))
            {
                if(element && element->getType() == Element::BOMB && !(dynamic_cast <Bomb*> (element)->isDetonated()))
                {
                    dynamic_cast <Bomb*> (element)->setDetonationCountdown(nDetonationCountdownWest);
                    nDetonationCountdownWest += nDetonationCountdown;
                }
                else if(element && element->getType() == Element::BLOCK)
                {
                    bWestDone = true;
                    if (m_blockItems[nRow][nColumn] != NULL)
                    {
                        removeItem(m_blockItems[nRow][nColumn]);
                        m_game->blockDestroyed(nRow, nColumn, dynamic_cast <Block*> (element));
                        delete m_blockItems[nRow][nColumn]; //this will also delete the block instance
                        m_blockItems[nRow][nColumn] = NULL;
                        //display bonus if available
                        if (m_bonusItems[nRow][nColumn] != NULL)
                        {
                            if (!items().contains(m_bonusItems[nRow][nColumn]))
                            {
                                addItem(m_bonusItems[nRow][nColumn]);
                            }
                        }
                    }
                }
                bombExplosionItem = new BombExplosionItem (bomb, BombExplosionItem::WEST, i);
                bombExplosionItem->setSharedRenderer(m_renderer);
                bombExplosionItem->update(bomb->getX() - (i+1)*Cell::SIZE, bomb->getY());
                bombExplosionItem->setZValue(300 + nBombRange+3 - i);
                addItem(bombExplosionItem);
                m_bombItems[bombItem].append(bombExplosionItem);
            }
            else
            {
                bWestDone = true;
            }
        }
        else
        {
          bWestDone = true;
        }
    }
}
