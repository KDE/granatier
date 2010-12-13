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
#include "bonusitem.h"
#include "settings.h"
#include "game.h"
#include "player.h"
#include "block.h"
#include "blockitem.h"
#include "bomb.h"
#include "arena.h"
#include "arenaitem.h"
#include "playeritem.h"
#include "bombitem.h"
#include "bombexplosionitem.h"
#include "infooverlay.h"
#include "infosidebar.h"

#include <KGameTheme>
#include <KLocale>
#include <QPainter>
#include <QGraphicsView>

#include <KGameRenderer>
#include <KGameRenderedItem>

GameScene::GameScene(Game* p_game) : m_game(p_game)
{
    connect(p_game, SIGNAL(gameStarted()), this, SLOT(start()));
    connect(p_game, SIGNAL(pauseChanged(bool, bool)), this, SLOT(setPaused(bool, bool)));
    connect(p_game, SIGNAL(bombCreated(Bomb*)), this, SLOT(createBombItem(Bomb*)));
    connect(p_game, SIGNAL(infoChanged(Game::InformationTypes)), this, SLOT(updateInfo(const Game::InformationTypes)));
    
    m_SvgScaleFactor = 1;
    
    // Load the selected SVG file
    m_rendererSelectedTheme = 0;
    loadTheme();
    //uncomment this if the crash in KSharedDataCache appears again
    //m_rendererSelectedTheme->setStrategyEnabled(KGameRenderer::UseDiskCache, false);
    // Load the default SVG file as fallback
    bool selectedThemeIsDefault = true;
    m_rendererDefaultTheme = 0;
    
    if(Settings::self()->theme() != "themes/granatier.desktop" || m_rendererSelectedTheme == 0)
    {
        selectedThemeIsDefault = false;
        m_rendererDefaultTheme = new KGameRenderer("themes/granatier.desktop");
        //uncomment this if the crash in KSharedDataCache appears again
        //m_rendererDefaultTheme->setStrategyEnabled(KGameRenderer::UseDiskCache, false);
    }
    
    if(selectedThemeIsDefault || m_rendererSelectedTheme->spriteExists("background"))
    {
        m_rendererBackground = m_rendererSelectedTheme;
    }
    else
    {
        m_rendererBackground = m_rendererDefaultTheme;
    }
    
    // set the renderer for the arena items TODO: add all the arena items
    if(selectedThemeIsDefault || (m_rendererSelectedTheme->spriteExists("arena_ground") &&
        m_rendererSelectedTheme->spriteExists("arena_wall") &&
        m_rendererSelectedTheme->spriteExists("arena_block") &&
        m_rendererSelectedTheme->spriteExists("arena_block_highlight") &&
        m_rendererSelectedTheme->spriteExists("arena_ice") &&
        m_rendererSelectedTheme->spriteExists("arena_bomb_mortar") &&
        m_rendererSelectedTheme->spriteExists("arena_arrow_up") &&
        m_rendererSelectedTheme->spriteExists("arena_arrow_right") &&
        m_rendererSelectedTheme->spriteExists("arena_arrow_down") &&
        m_rendererSelectedTheme->spriteExists("arena_arrow_left")))
    {
        m_rendererArenaItems = m_rendererSelectedTheme;
    }
    else
    {
        m_rendererArenaItems = m_rendererDefaultTheme;
    }
    
    // set the renderer for the bonus items TODO: add all the bonus items
    if(selectedThemeIsDefault || (m_rendererSelectedTheme->spriteExists("bonus_speed") &&
        m_rendererSelectedTheme->spriteExists("bonus_bomb") &&
        m_rendererSelectedTheme->spriteExists("bonus_power") &&
        m_rendererSelectedTheme->spriteExists("bonus_shield") &&
        m_rendererSelectedTheme->spriteExists("bonus_throw") &&
        m_rendererSelectedTheme->spriteExists("bonus_kick") &&
        m_rendererSelectedTheme->spriteExists("bonus_bad_slow") &&
        m_rendererSelectedTheme->spriteExists("bonus_bad_hyperactive") &&
        m_rendererSelectedTheme->spriteExists("bonus_bad_mirror") &&
        m_rendererSelectedTheme->spriteExists("bonus_bad_scatty") &&
        m_rendererSelectedTheme->spriteExists("bonus_bad_restrain") &&
        m_rendererSelectedTheme->spriteExists("bonus_neutral_pandora") &&
        m_rendererSelectedTheme->spriteExists("bonus_neutral_resurrect")))
    {
        m_rendererBonusItems = m_rendererSelectedTheme;
    }
    else
    {
        m_rendererBonusItems = m_rendererDefaultTheme;
    }
    
    // set the renderer for the bomb items
    if(selectedThemeIsDefault || (m_rendererSelectedTheme->spriteExists("bomb") &&
        m_rendererSelectedTheme->spriteExists("bomb_blast_core_0") &&
        m_rendererSelectedTheme->spriteExists("bomb_blast_north_0") &&
        m_rendererSelectedTheme->spriteExists("bomb_blast_east_0") &&
        m_rendererSelectedTheme->spriteExists("bomb_blast_south_0") &&
        m_rendererSelectedTheme->spriteExists("bomb_blast_west_0")))
    {
        m_rendererBombItems = m_rendererSelectedTheme;
    }
    else
    {
        m_rendererBombItems = m_rendererDefaultTheme;
    }
    
    // set the renderer for the score items
    if(selectedThemeIsDefault || (m_rendererSelectedTheme->spriteExists("score_star_enabled") &&
        m_rendererSelectedTheme->spriteExists("score_star_disabled")))
    {
        m_rendererScoreItems = m_rendererSelectedTheme;
    }
    else
    {
        m_rendererScoreItems = m_rendererDefaultTheme;
    }

    // Create the PlayerItems and the points labels
    QList <Player*> players = p_game->getPlayers();
    
    PlayerItem* playerItem;
    for(int i = 0; i < players.size(); i++)
    {
        KGameRenderer* playerRenderer = new KGameRenderer(players[i]->getDesktopFilePath());
        playerRenderer->setStrategyEnabled(KGameRenderer::UseDiskCache, false);
        m_mapRendererPlayerItems.insert(players[i], playerRenderer);
        playerItem = new PlayerItem(players[i], playerRenderer);
        // Corrects the position of the player
        playerItem->update(players[i]->getX(), players[i]->getY());
        // Stops the player animation
        playerItem->stopAnim();
        
        m_playerItems.append(playerItem);
        
        connect(this, SIGNAL(resizeGraphics(qreal)), playerItem, SLOT(updateGraphics(qreal)));
        connect (playerItem, SIGNAL(bonusItemTaken(BonusItem*)), this, SLOT(removeBonusItem(BonusItem*)));
    }
    
    // The remaining time
    m_remainingTimeLabel = new QGraphicsTextItem(i18n("0:00"));
    m_remainingTimeLabel->setFont(QFont("Helvetica", 15, QFont::Bold, false));
    m_remainingTimeLabel->setDefaultTextColor(QColor("#FFFF00"));
    m_remainingTimeLabel->setZValue(0);
    
    m_arenaNameLabel = new QGraphicsTextItem(i18n("Arena Name"));
    m_arenaNameLabel->setFont(QFont("Helvetica", 15, QFont::Bold, false));
    m_arenaNameLabel->setDefaultTextColor(QColor("#FFFF00"));
    m_arenaNameLabel->setZValue(0);
    
    setSceneRect(0, -m_remainingTimeLabel->boundingRect().height(),
                 m_game->getArena()->getNbColumns()*Cell::SIZE,
                 m_game->getArena()->getNbRows()*Cell::SIZE + m_remainingTimeLabel->boundingRect().height());
    
    //create the background    
    m_rendererBackground->setStrategyEnabled(KGameRenderer::UseDiskCache, false); //TODO: why does it crash with disc cache on?
    m_arenaBackground = new KGameRenderedItem(m_rendererBackground, "background");
    m_arenaBackground->setZValue(-5);
    m_arenaBackground->setPos(0, 0);
    addItem(m_arenaBackground);
    
    // create the info sidebar
    m_infoSidebar = new InfoSidebar(m_game, this);
    //update the sceneRect
    QRectF oldSceneRect = sceneRect();
    QRectF sidebarRect = m_infoSidebar->rect();
    QRectF newSceneRect;
    newSceneRect.setLeft(oldSceneRect.left() < sidebarRect.left() ? oldSceneRect.left() : sidebarRect.left());
    newSceneRect.setRight(oldSceneRect.right() > sidebarRect.right() ? oldSceneRect.right() : sidebarRect.right());
    newSceneRect.setTop(oldSceneRect.top() < sidebarRect.top() ? oldSceneRect.top() : sidebarRect.top());
    newSceneRect.setBottom(oldSceneRect.bottom() > sidebarRect.bottom() ? oldSceneRect.bottom() : sidebarRect.bottom());
    newSceneRect.adjust(-5, -5, 5, 5);
    setSceneRect(newSceneRect);
    
    // create the info overlay
    m_infoOverlay = new InfoOverlay(m_game, this);
    
    init();
    
    //at this point, sceneRect() has the minimum size for the scene
    m_minSize = sceneRect();
    m_minSize.setX((int) ((int)m_minSize.x() - ((int)m_minSize.x() % (int)Cell::SIZE)));
    m_minSize.setY((int) ((int)m_minSize.y() - ((int)m_minSize.y() % (int)Cell::SIZE)));
    m_minSize.setHeight(((int) (m_minSize.height() / Cell::SIZE) + 1) * Cell::SIZE);
    m_minSize.setWidth(((int) (m_minSize.width() / Cell::SIZE) + 1) * Cell::SIZE);
    setSceneRect(m_minSize);
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
            ArenaItem* arenaItem = new ArenaItem(j * Cell::SIZE, i * Cell::SIZE, m_rendererArenaItems, "");
            connect(this, SIGNAL(resizeGraphics(qreal)), arenaItem, SLOT(updateGraphics(qreal)));
            
            //TODO: use this function call
            //arenaItem->setElementId(m_game->getArena()->getCell(i,j).getElement()->getImageId());
            switch(m_game->getArena()->getCell(i,j).getType())
            {
                case Cell::WALL:
                    arenaItem->setSpriteKey("arena_wall");
                    arenaItem->setZValue(-2);
                    break;
                case Cell::HOLE:
                    delete arenaItem;
                    arenaItem = NULL;
                    break;
                case Cell::ICE:
                    arenaItem->setSpriteKey("arena_ice");
                    arenaItem->setZValue(0);
                    break;
                case Cell::BOMBMORTAR:
                    arenaItem->setSpriteKey("arena_bomb_mortar");
                    arenaItem->setZValue(0);
                    break;
                case Cell::ARROWUP:
                    arenaItem->setSpriteKey("arena_arrow_up");
                    arenaItem->setZValue(0);
                    break;
                case Cell::ARROWRIGHT:
                    arenaItem->setSpriteKey("arena_arrow_right");
                    arenaItem->setZValue(0);
                    break;
                case Cell::ARROWDOWN:
                    arenaItem->setSpriteKey("arena_arrow_down");
                    arenaItem->setZValue(0);
                    break;
                case Cell::ARROWLEFT:
                    arenaItem->setSpriteKey("arena_arrow_left");
                    arenaItem->setZValue(0);
                    break;
                case Cell::GROUND:
                case Cell::BLOCK:
                default:
                    arenaItem->setSpriteKey("arena_ground");
                    arenaItem->setZValue(-1);
            }
            m_arenaItem[i][j] = arenaItem;
        }
    }
    
    // Create the Block and Bonus items
    m_blockItems = new BlockItem**[m_game->getArena()->getNbRows()];
    m_bonusItems = new BonusItem**[m_game->getArena()->getNbRows()];
    for (int i = 0; i < m_game->getArena()->getNbRows(); ++i)
    {
        m_blockItems[i] = new BlockItem*[m_game->getArena()->getNbColumns()];
        m_bonusItems[i] = new BonusItem*[m_game->getArena()->getNbColumns()];
        for (int j = 0; j < m_game->getArena()->getNbColumns(); ++j)
        {
            if (m_game->getArena()->getCell(i, j).getElement() != NULL && m_game->getArena()->getCell(i, j).getElement()->getType() == Element::BLOCK)
            {
                // Create the element item and set the image
                Block* block = dynamic_cast <Block*> (m_game->getArena()->getCell(i, j).getElement());
                BlockItem* blockItem = new BlockItem(block, m_rendererArenaItems);
                connect(this, SIGNAL(resizeGraphics(qreal)), blockItem, SLOT(updateGraphics(qreal)));
                blockItem->setSpriteKey(block->getImageId());
                blockItem->update(block->getX(), block->getY());
                blockItem->setZValue(200);
                if(Settings::self()->showAllTiles() == 1)
                {
                    blockItem->setZValue(99);
                }
                connect(this, SIGNAL(resizeGraphics(qreal)), blockItem, SLOT(updateGraphics(qreal)));
                connect(blockItem, SIGNAL(blockItemDestroyed(BlockItem*)), this, SLOT(removeBlockItem(BlockItem*)));
                m_blockItems[i][j] = blockItem;
                // if the block contains a hidden bonus, create the bonus item 
                Bonus* bonus = block->getBonus();
                if(bonus)
                {
                    BonusItem* bonusItem = new BonusItem(bonus, m_rendererBonusItems);
                    switch(bonus->getBonusType())
                    {
                        case Bonus::SPEED:  bonusItem->setSpriteKey("bonus_speed");
                                            break;
                        case Bonus::BOMB:   bonusItem->setSpriteKey("bonus_bomb");
                                            break;
                        case Bonus::POWER:  bonusItem->setSpriteKey("bonus_power");
                                            break;
                        case Bonus::SHIELD: bonusItem->setSpriteKey("bonus_shield");
                                            break;
                        case Bonus::THROW:  bonusItem->setSpriteKey("bonus_throw");
                                            break;
                        case Bonus::KICK:   bonusItem->setSpriteKey("bonus_kick");
                                            break;
                        case Bonus::HYPERACTIVE:   bonusItem->setSpriteKey("bonus_bad_hyperactive");
                                            break;
                        case Bonus::SLOW:   bonusItem->setSpriteKey("bonus_bad_slow");
                                            break;
                        case Bonus::MIRROR: bonusItem->setSpriteKey("bonus_bad_mirror");
                                            break;
                        case Bonus::SCATTY: bonusItem->setSpriteKey("bonus_bad_scatty");
                                            break;
                        case Bonus::RESTRAIN:   bonusItem->setSpriteKey("bonus_bad_restrain");
                                            break;
                        case Bonus::RESURRECT:   bonusItem->setSpriteKey("bonus_neutral_resurrect");
                                            break;
                        default:            bonusItem->setSpriteKey("bonus_neutral_pandora");
                    }
                    
                    if((qrand()/1.0)/RAND_MAX * 10 > 9 && bonusItem->spriteKey() != "bonus_neutral_resurrect")
                    {
                        bonusItem->setSpriteKey("bonus_neutral_pandora");
                    }
                    
                    bonusItem->update(bonus->getX(), bonus->getY());
                    bonusItem->setZValue(100);
                    m_bonusItems[i][j] = bonusItem;
                    
                    connect(this, SIGNAL(resizeGraphics(qreal)), bonusItem, SLOT(updateGraphics(qreal)));
                    connect(bonusItem, SIGNAL(bonusItemDestroyed(BonusItem*)), this, SLOT(removeBonusItem(BonusItem*)));
                    
                    addItem(bonusItem);
                    if(Settings::self()->showAllTiles() == 0)
                    {
                        bonusItem->hide();
                    }
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
    
    // Display each PlayerItem
    for (int i = 0; i < m_playerItems.size(); i++)
    {
        if(!items().contains(m_playerItems[i]))
        {
            addItem(m_playerItems[i]);
        }
        m_playerItems[i]->resurrect();
    }
    
    if (!items().contains(m_remainingTimeLabel))
    {
        addItem(m_remainingTimeLabel);
    }
    m_remainingTimeLabel->setDefaultTextColor(QColor("#FFFF00"));
    int nTime = m_game->getRemainingTime();
    m_remainingTimeLabel->setPlainText(QString("%1:%2").arg(nTime/60).arg(nTime%60, 2, 10, QChar('0')));
    m_remainingTimeLabel->setPos(Cell::SIZE * m_game->getArena()->getNbColumns() - m_remainingTimeLabel->boundingRect().width(), - m_remainingTimeLabel->boundingRect().height());
    
    if (!items().contains(m_arenaNameLabel))
    {
        addItem(m_arenaNameLabel);
    }
    m_arenaNameLabel->setPlainText(m_game->getArena()->getName());
    m_arenaNameLabel->setPos(0, - m_arenaNameLabel->boundingRect().height());
    
    m_infoSidebar->reset();
    
    m_infoOverlay->showGetReady();
    
    resizeBackground();
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
        m_playerItems[i]->stopAnim();
        delete m_playerItems[i];
    }
    
    QMap <Player*, KGameRenderer*>::iterator iteratorRendererPlayer = m_mapRendererPlayerItems.begin();
    while (iteratorRendererPlayer != m_mapRendererPlayerItems.end())
    {
        delete iteratorRendererPlayer.value();
    }
    
    removeItem(m_arenaBackground);
    delete m_arenaBackground;
    
    delete m_infoOverlay;
    delete m_infoSidebar;
    delete m_remainingTimeLabel;
    delete m_arenaNameLabel;
    
    delete m_rendererSelectedTheme;
    delete m_rendererDefaultTheme;
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
    
    m_infoOverlay->hideItems();
    
    if(items().contains(m_remainingTimeLabel))
    {
        removeItem(m_remainingTimeLabel);
    }
    
    if(items().contains(m_arenaNameLabel))
    {
        removeItem(m_arenaNameLabel);
    }
}

KGameRenderer* GameScene::renderer(Element::Type type, Player* player)
{
    switch(type)
    {
        case Element::BLOCK:
            return m_rendererArenaItems;
        case Element::BONUS:
            return m_rendererBonusItems;
        case Element::BOMB:
            return m_rendererBombItems;
        case Element::PLAYER:
            return m_mapRendererPlayerItems.value(player);
        case Element::SCORE:
            return m_rendererScoreItems;
        default:
            return NULL;
    }
}

void GameScene::showScore()
{
    m_infoOverlay->showScore();
}

//TODO: rename this function
void GameScene::resizeBackground()
{
    if(views().isEmpty())
    {
        return;
    }
    
    //calculate the size of the view, that a Cell fits into whole pixels
    qreal rows = m_game->getArena()->getNbRows();
    qreal columns = m_game->getArena()->getNbColumns();
    
    QPoint topLeftArena = views().at(0)->mapFromScene(0, 0);
    QPoint bottomRightArena = views().at(0)->mapFromScene(Cell::SIZE * columns, Cell::SIZE * rows);
    
    qreal columnRatio = (bottomRightArena.x() - topLeftArena.x()) / columns;
    qreal rowRatio = (bottomRightArena.y() - topLeftArena.y()) / rows;
    
    QRectF newSceneRect = views().at(0)->sceneRect();
    newSceneRect.setSize(QSizeF(m_minSize.width() * columnRatio / (int) columnRatio, m_minSize.height() * rowRatio / (int) rowRatio));
    
    qreal newWidth = m_minSize.width() * views().at(0)->size().width() / (((int) (views().at(0)->size().width() / (m_minSize.width() / Cell::SIZE))) * m_minSize.width() / Cell::SIZE);
    qreal newHeigth = m_minSize.height() * views().at(0)->size().height() / (((int) (views().at(0)->size().height() / (m_minSize.height() / Cell::SIZE))) * m_minSize.height() / Cell::SIZE);
    newSceneRect.setSize(QSizeF(newWidth, newHeigth));
    
    views().at(0)->setSceneRect(newSceneRect);
    
    
    qreal x = views().at(0)->x();
    qreal y = views().at(0)->y();
    qreal width = views().at(0)->width();
    qreal height = views().at(0)->height();
    QPointF topLeftView = views().at(0)->mapToScene(0, 0);
    QPointF bottomRightView = views().at(0)->mapToScene(width, height);
    
    x = topLeftView.x();
    y = topLeftView.y();
    width = bottomRightView.x() - x;
    height = bottomRightView.y() - y;
    
    //calculate the scale factor between graphicsscene and graphicsview
    //TODO: calcute with width and views().at(0)->width();
    QPoint topLeft(0, 0);
    QPoint bottomRight(100, 100);
    topLeft = views().at(0)->mapFromScene(topLeft);
    bottomRight = views().at(0)->mapFromScene(bottomRight);
    m_SvgScaleFactor = 100.0 / (bottomRight.x() - topLeft.x());
    
    //get the graphicsview size
    QSize svgSize = views().at(0)->size();
    
    //set pixmap
    m_arenaBackground->setRenderSize(svgSize);
    m_arenaBackground->setPos(x, y);
    m_arenaBackground->setScale(m_SvgScaleFactor);
    //m_arenaBackground->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    //m_arenaBackground->setCacheMode(QGraphicsItem::DeviceCoordinateCache, svgSize);
    
    //update pixmaps
    emit resizeGraphics(m_SvgScaleFactor);
    
    //update overlay
    m_infoOverlay->resizeDimmOverlay(x, y, width, height);
}

Game* GameScene::getGame() const
{
    return m_game;
}

void GameScene::loadTheme()
{
    m_rendererSelectedTheme = new KGameRenderer(Settings::self()->theme());
    
    update(0, 0, width(), height());

    // Update the theme config: if the default theme is selected, no theme entry is written -> the theme selector does not select the theme
    Settings::self()->config()->group("General").writeEntry("Theme", Settings::self()->theme());
}

void GameScene::start()
{
    // hide the info items
    m_infoOverlay->hideItems();
}

void GameScene::setPaused(const bool p_pause, const bool p_fromUser)
{
    // If the game has paused
    if (p_pause)
    {
        // If the pause is due to an action from the user
        if (p_fromUser)
        {
            m_infoOverlay->showPause();
        }
        
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
    {   // If the game has resumed, hide the info items
        m_infoOverlay->hideItems();
        
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

void GameScene::removeBlockItem(BlockItem* blockItem)
{
    // remove the Bonus Items
    for (int i = 0; i < m_game->getArena()->getNbRows(); ++i)
    {
        for (int j = 0; j < m_game->getArena()->getNbColumns(); ++j)
        {
            if (m_blockItems[i][j] != NULL && m_blockItems[i][j] == blockItem)
            {
                if (items().contains(m_blockItems[i][j]))
                {
                    removeItem(m_blockItems[i][j]);
                    m_blockItems[i][j] = NULL;
                    m_game->blockDestroyed(i, j, dynamic_cast <Block*> (blockItem->getModel()));
                    delete blockItem;
                }
            }
        }
    }
}

void GameScene::removeBonusItem(BonusItem* bonusItem)
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
    if(p_info == Game::TimeInfo)
    {
        int nTime = m_game->getRemainingTime();
        if(nTime > 0)
        {
            m_remainingTimeLabel->setPlainText(QString("%1:%2").arg(nTime/60).arg(nTime%60, 2, 10, QChar('0')));
        }
        else
        {
            m_remainingTimeLabel->setPlainText(i18n("Sudden Death"));
            m_remainingTimeLabel->setDefaultTextColor(QColor("#FF0000"));
            m_remainingTimeLabel->setPos(Cell::SIZE * m_game->getArena()->getNbColumns() - m_remainingTimeLabel->boundingRect().width(), - m_remainingTimeLabel->boundingRect().height());
        }
    }
}

void GameScene::createBombItem(Bomb* bomb)
{
    // Create the Bombs
    BombItem* bombItem = new BombItem(bomb, m_rendererBombItems);
    // Corrects the position of the BombItem
    bombItem->update(bomb->getX(), bomb->getY());
    addItem(bombItem);
    m_bombItems[bombItem].append(NULL);
    
    bombItem->updateGraphics(m_SvgScaleFactor); //TODO: use a Renderer class and get the scale factor from a static function during initialization
    
    connect(this, SIGNAL(resizeGraphics(qreal)), bombItem, SLOT(updateGraphics(qreal)));
    connect(bomb, SIGNAL(mortar(int)), bombItem, SLOT(updateMortar(int)));
    connect(bomb, SIGNAL(bombDetonated(Bomb*)), this, SLOT(bombDetonated(Bomb*)));
    connect(bombItem, SIGNAL(bombItemFinished(BombItem*)), this, SLOT(removeBombItem(BombItem*)));
    connect(bombItem, SIGNAL(animationFrameChanged(BombItem*, int)), this, SLOT(updateBombExplosionItemAnimation(BombItem*, int)));
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
    int nBombPower = bomb->bombPower();
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
    
    //check if there is a bomb or block at the position where the bomb exploded (possible when thrown)
    nColumn = m_game->getArena()->getColFromX(bomb->getX());
    nRow = m_game->getArena()->getRowFromY(bomb->getY());
    if(nColumn >= 0 && nColumn < nNumberOfColums && nRow >= 0 && nRow < nNumberOfRows)
    {
        element = m_game->getArena()->getCell(nRow, nColumn).getElement();
        if(element && element->getType() == Element::BOMB && dynamic_cast <Bomb*> (element) != bomb && !(dynamic_cast <Bomb*> (element)->isDetonated()))
        {
            dynamic_cast <Bomb*> (element)->initDetonation(bomb->explosionID(), nDetonationCountdown);
        }
        else if(element && element->getType() == Element::BLOCK)
        {
            dynamic_cast <Block*> (element)->startDestruction(bomb->explosionID());
            if (m_blockItems[nRow][nColumn] != NULL)
            {
                //display bonus if available
                if (m_bonusItems[nRow][nColumn] != NULL)
                {
                    m_bonusItems[nRow][nColumn]->setUndestroyable(bomb->explosionID());
                    m_bonusItems[nRow][nColumn]->show();
                }
            }
        }
        else if(m_bonusItems[nRow][nColumn] != NULL)
        {
            m_bonusItems[nRow][nColumn]->initDestruction(bomb->explosionID());
        }
    }
    
    for(int i = 0; i < nBombPower; i++)
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
                    dynamic_cast <Bomb*> (element)->initDetonation(bomb->explosionID(), nDetonationCountdownNorth);
                    nDetonationCountdownNorth += nDetonationCountdown;
                }
                else if(element && element->getType() == Element::BLOCK)
                {
                    bNorthDone = true;
                    dynamic_cast <Block*> (element)->startDestruction(bomb->explosionID());
                    if (m_blockItems[nRow][nColumn] != NULL)
                    {
                        //display bonus if available
                        if (m_bonusItems[nRow][nColumn] != NULL)
                        {
                            m_bonusItems[nRow][nColumn]->setUndestroyable(bomb->explosionID());
                            m_bonusItems[nRow][nColumn]->show();
                        }
                    }
                }
                else if(m_bonusItems[nRow][nColumn] != NULL)
                {
                    m_bonusItems[nRow][nColumn]->initDestruction(bomb->explosionID());
                }
                bombExplosionItem = new BombExplosionItem (bomb, BombExplosionItem::NORTH, nBombPower - i, m_rendererBombItems, m_SvgScaleFactor);
                bombExplosionItem->setPosition(bomb->getX(), bomb->getY() - (i+1)*Cell::SIZE);
                bombExplosionItem->setZValue(300 + nBombPower+3 - i);
                connect(this, SIGNAL(resizeGraphics(qreal)), bombExplosionItem, SLOT(updateGraphics(qreal)));
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
                    dynamic_cast <Bomb*> (element)->initDetonation(bomb->explosionID(), nDetonationCountdownEast);
                    nDetonationCountdownEast += nDetonationCountdown;
                }
                else if(element && element->getType() == Element::BLOCK)
                {
                    bEastDone = true;
                    dynamic_cast <Block*> (element)->startDestruction(bomb->explosionID());
                    if (m_blockItems[nRow][nColumn] != NULL)
                    {
                        //display bonus if available
                        if (m_bonusItems[nRow][nColumn] != NULL)
                        {
                            m_bonusItems[nRow][nColumn]->setUndestroyable(bomb->explosionID());
                            m_bonusItems[nRow][nColumn]->show();
                        }
                    }
                }
                else if(m_bonusItems[nRow][nColumn] != NULL)
                {
                    m_bonusItems[nRow][nColumn]->initDestruction(bomb->explosionID());
                }
                bombExplosionItem = new BombExplosionItem (bomb, BombExplosionItem::EAST, nBombPower - i, m_rendererBombItems, m_SvgScaleFactor);
                bombExplosionItem->setPosition(bomb->getX() + (i+1)*Cell::SIZE, bomb->getY());
                bombExplosionItem->setZValue(300 + nBombPower+3 - i);
                connect(this, SIGNAL(resizeGraphics(qreal)), bombExplosionItem, SLOT(updateGraphics(qreal)));
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
                    dynamic_cast <Bomb*> (element)->initDetonation(bomb->explosionID(), nDetonationCountdownSouth);
                    nDetonationCountdownSouth += nDetonationCountdown;
                }
                else if(element && element->getType() == Element::BLOCK)
                {
                    bSouthDone = true;
                    dynamic_cast <Block*> (element)->startDestruction(bomb->explosionID());
                    if (m_blockItems[nRow][nColumn] != NULL)
                    {
                        //display bonus if available
                        if (m_bonusItems[nRow][nColumn] != NULL)
                        {
                            m_bonusItems[nRow][nColumn]->setUndestroyable(bomb->explosionID());
                            m_bonusItems[nRow][nColumn]->show();
                        }
                    }
                }
                else if(m_bonusItems[nRow][nColumn] != NULL)
                {
                    m_bonusItems[nRow][nColumn]->initDestruction(bomb->explosionID());
                }
                bombExplosionItem = new BombExplosionItem (bomb, BombExplosionItem::SOUTH, nBombPower - i, m_rendererBombItems, m_SvgScaleFactor);
                bombExplosionItem->setPosition(bomb->getX(), bomb->getY() + (i+1)*Cell::SIZE);
                bombExplosionItem->setZValue(300 + nBombPower+3 - i);
                connect(this, SIGNAL(resizeGraphics(qreal)), bombExplosionItem, SLOT(updateGraphics(qreal)));
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
                    dynamic_cast <Bomb*> (element)->initDetonation(bomb->explosionID(), nDetonationCountdownWest);
                    nDetonationCountdownWest += nDetonationCountdown;
                }
                else if(element && element->getType() == Element::BLOCK)
                {
                    bWestDone = true;
                    dynamic_cast <Block*> (element)->startDestruction(bomb->explosionID());
                    if (m_blockItems[nRow][nColumn] != NULL)
                    {
                        //display bonus if available
                        if (m_bonusItems[nRow][nColumn] != NULL)
                        {
                            m_bonusItems[nRow][nColumn]->setUndestroyable(bomb->explosionID());
                            m_bonusItems[nRow][nColumn]->show();
                        }
                    }
                }
                else if(m_bonusItems[nRow][nColumn] != NULL)
                {
                    m_bonusItems[nRow][nColumn]->initDestruction(bomb->explosionID());
                }
                bombExplosionItem = new BombExplosionItem (bomb, BombExplosionItem::WEST, nBombPower - i, m_rendererBombItems, m_SvgScaleFactor);
                bombExplosionItem->setPosition(bomb->getX() - (i+1)*Cell::SIZE, bomb->getY());
                bombExplosionItem->setZValue(300 + nBombPower+3 - i);
                connect(this, SIGNAL(resizeGraphics(qreal)), bombExplosionItem, SLOT(updateGraphics(qreal)));
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

void GameScene::updateBombExplosionItemAnimation(BombItem* bombItem, int nFrame)
{
    // Find the BombItems and update the frame
    BombExplosionItem* bombExplosionItem;
    QHash<BombItem*, QList<BombExplosionItem*> >::iterator i = m_bombItems.begin();
    while (i != m_bombItems.end())
    {
        if(i.key() == bombItem)
        {
            for(int nIndex = 0; nIndex < i.value().count(); nIndex++)
            {
                bombExplosionItem = i.value().at(nIndex);
                if(bombExplosionItem)
                {
                    bombExplosionItem->updateAnimationn(nFrame);
                }
            }
            break;
        }
        else
        {
            ++i;
        }
    }
}
