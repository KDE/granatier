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

#include <KgTheme>
#include <KgThemeProvider>
#include <KLocale>
#include <KStandardDirs>
#include <QPainter>
#include <QGraphicsView>
#include <QTimer>

#include <KGameRenderer>
#include <KGameRenderedItem>

GameScene::GameScene(Game* p_game, KgThemeProvider* p_themeProvider) : m_game(p_game), m_themeProvider(p_themeProvider)
{
    connect(p_game, SIGNAL(gameStarted()), this, SLOT(start()));
    connect(p_game, SIGNAL(pauseChanged(bool,bool)), this, SLOT(setPaused(bool,bool)));
    connect(p_game, SIGNAL(bombCreated(Bomb*)), this, SLOT(createBombItem(Bomb*)));
    connect(p_game, SIGNAL(infoChanged(Granatier::Info::Type)), this, SLOT(updateInfo(Granatier::Info::Type)));
    
    m_SvgScaleFactor = 1;
    
    m_backgroundResizeTimer = new QTimer();
    m_backgroundResizeTimer->setSingleShot(true);
    connect(m_backgroundResizeTimer, SIGNAL(timeout()), this, SLOT(resizeBackground()));

    // Create the PlayerItems and the points labels
    QList <Player*> players = p_game->getPlayers();
    
    PlayerItem* playerItem;
    for(int i = 0; i < players.size(); i++)
    {
        const QString desktopPath = players[i]->getDesktopFilePath();
        KgTheme* theme = new KgTheme(desktopPath.toUtf8());
        theme->readFromDesktopFile(KStandardDirs::locate("appdata", desktopPath));
        KGameRenderer* playerRenderer = new KGameRenderer(theme);
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
    m_remainingTimeLabel->setZValue(1000);
    
    m_arenaNameLabel = new QGraphicsTextItem(i18n("Arena Name"));
    m_arenaNameLabel->setFont(QFont("Helvetica", 15, QFont::Bold, false));
    m_arenaNameLabel->setDefaultTextColor(QColor("#FFFF00"));
    m_arenaNameLabel->setZValue(1000);
    
    setSceneRect(0, -m_remainingTimeLabel->boundingRect().height(),
                 m_game->getArena()->getNbColumns()*Granatier::CellSize,
                 m_game->getArena()->getNbRows()*Granatier::CellSize + m_remainingTimeLabel->boundingRect().height());
    
    // setup the theme renderer
    //KgTheme* theme = new KgTheme(m_themeProvider->currentTheme()->identifier());
    //theme->setGraphicsPath(m_themeProvider->currentTheme()->graphicsPath());
    m_rendererSelectedTheme = new KGameRenderer(m_themeProvider);//theme);
    m_rendererDefaultTheme = 0;
    setupThemeRenderer();
    
    connect(m_themeProvider, SIGNAL(currentThemeChanged(const KgTheme*)), this, SLOT(themeChanged(const KgTheme*)));
    
    // create the info sidebar
    m_infoSidebar = new InfoSidebar(m_game, this);
    connect(this, SIGNAL(resizeGraphics(qreal)), m_infoSidebar, SLOT(updateGraphics(qreal)));
    
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
    connect(this, SIGNAL(resizeGraphics(qreal)), m_infoOverlay, SLOT(updateGraphics(qreal)));
    
    init();
    
    //at this point, sceneRect() has the minimum size for the scene
    m_minSize = sceneRect();
    m_minSize.setX((int) ((int)m_minSize.x() - ((int)m_minSize.x() % (int)Granatier::CellSize)) - Granatier::CellSize/4);
    m_minSize.setY((int) ((int)m_minSize.y() - ((int)m_minSize.y() % (int)Granatier::CellSize)) - Granatier::CellSize/4);
    m_minSize.setHeight(((int) (m_minSize.height() / Granatier::CellSize) + 1) * Granatier::CellSize);
    m_minSize.setWidth(((int) (m_minSize.width() / Granatier::CellSize) + 1) * Granatier::CellSize);
    setSceneRect(m_minSize);
}

void GameScene::setupThemeRenderer()
{
    bool selectedThemeIsDefault = true;
    
    if(m_themeProvider->currentTheme() != m_themeProvider->defaultTheme())
    {
        // Load the default SVG file as fallback
        selectedThemeIsDefault = false;
        if(m_rendererDefaultTheme == 0)
        {
            KgTheme* theme = new KgTheme(m_themeProvider->defaultTheme()->identifier());
            theme->setGraphicsPath(m_themeProvider->defaultTheme()->graphicsPath());
            m_rendererDefaultTheme = new KGameRenderer(theme);
        }
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
}

void GameScene::init()
{
    initItemsWithGraphicsFromTheme();
    
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
    m_remainingTimeLabel->setPos(Granatier::CellSize * m_game->getArena()->getNbColumns() - m_remainingTimeLabel->boundingRect().width(), - m_remainingTimeLabel->boundingRect().height());
    
    if (!items().contains(m_arenaNameLabel))
    {
        addItem(m_arenaNameLabel);
    }
    m_arenaNameLabel->setPlainText(m_game->getArena()->getName());
    m_arenaNameLabel->setPos(0, - m_arenaNameLabel->boundingRect().height());
    
    m_infoSidebar->reset();
    
    m_infoOverlay->showGetReady();
}

void GameScene::initItemsWithGraphicsFromTheme()
{
    // Create the ArenaItems
    m_arenaItem = new ArenaItem**[m_game->getArena()->getNbRows()];
    
    for (int i = 0; i < m_game->getArena()->getNbRows(); ++i)
    {
        m_arenaItem[i] = new ArenaItem*[m_game->getArena()->getNbColumns()];
        for (int j = 0; j < m_game->getArena()->getNbColumns(); ++j)
        {
            // Create the ArenaItem and set the image
            ArenaItem* arenaItem = new ArenaItem(j * Granatier::CellSize, i * Granatier::CellSize, m_rendererArenaItems, "");
            connect(this, SIGNAL(resizeGraphics(qreal)), arenaItem, SLOT(updateGraphics(qreal)));
            
            //TODO: use this function call
            //arenaItem->setElementId(m_game->getArena()->getCell(i,j).getElement()->getImageId());
            switch(m_game->getArena()->getCell(i,j).getType())
            {
                case Granatier::Cell::WALL:
                    arenaItem->setSpriteKey("arena_wall");
                    arenaItem->setZValue(-2);
                    break;
                case Granatier::Cell::HOLE:
                    delete arenaItem;
                    arenaItem = NULL;
                    break;
                case Granatier::Cell::ICE:
                    arenaItem->setSpriteKey("arena_ice");
                    arenaItem->setZValue(0);
                    break;
                case Granatier::Cell::BOMBMORTAR:
                    arenaItem->setSpriteKey("arena_bomb_mortar");
                    arenaItem->setZValue(0);
                    break;
                case Granatier::Cell::ARROWUP:
                    arenaItem->setSpriteKey("arena_arrow_up");
                    arenaItem->setZValue(0);
                    break;
                case Granatier::Cell::ARROWRIGHT:
                    arenaItem->setSpriteKey("arena_arrow_right");
                    arenaItem->setZValue(0);
                    break;
                case Granatier::Cell::ARROWDOWN:
                    arenaItem->setSpriteKey("arena_arrow_down");
                    arenaItem->setZValue(0);
                    break;
                case Granatier::Cell::ARROWLEFT:
                    arenaItem->setSpriteKey("arena_arrow_left");
                    arenaItem->setZValue(0);
                    break;
                case Granatier::Cell::GROUND:
                case Granatier::Cell::BLOCK:
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
    QList<Element*> blockElements;
    for (int i = 0; i < m_game->getArena()->getNbRows(); ++i)
    {
        m_blockItems[i] = new BlockItem*[m_game->getArena()->getNbColumns()];
        m_bonusItems[i] = new BonusItem*[m_game->getArena()->getNbColumns()];
        for (int j = 0; j < m_game->getArena()->getNbColumns(); ++j)
        {
            blockElements = m_game->getArena()->getCell(i, j).getElements(Granatier::Element::BLOCK);
            if (!blockElements.isEmpty())
            {
                // Create the element item and set the image
                foreach(Element* element, blockElements)
                {
                    Block* block = dynamic_cast <Block*> (element);
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
                            case Granatier::Bonus::SPEED:
                                bonusItem->setSpriteKey("bonus_speed");
                                break;
                            case Granatier::Bonus::BOMB:
                                bonusItem->setSpriteKey("bonus_bomb");
                                break;
                            case Granatier::Bonus::POWER:
                                bonusItem->setSpriteKey("bonus_power");
                                break;
                            case Granatier::Bonus::SHIELD:
                                bonusItem->setSpriteKey("bonus_shield");
                                break;
                            case Granatier::Bonus::THROW:
                                bonusItem->setSpriteKey("bonus_throw");
                                break;
                            case Granatier::Bonus::KICK:
                                bonusItem->setSpriteKey("bonus_kick");
                                break;
                            case Granatier::Bonus::HYPERACTIVE:
                                bonusItem->setSpriteKey("bonus_bad_hyperactive");
                                break;
                            case Granatier::Bonus::SLOW:
                                bonusItem->setSpriteKey("bonus_bad_slow");
                                break;
                            case Granatier::Bonus::MIRROR:
                                bonusItem->setSpriteKey("bonus_bad_mirror");
                                break;
                            case Granatier::Bonus::SCATTY:
                                bonusItem->setSpriteKey("bonus_bad_scatty");
                                break;
                            case Granatier::Bonus::RESTRAIN:
                                bonusItem->setSpriteKey("bonus_bad_restrain");
                                break;
                            case Granatier::Bonus::RESURRECT:
                                bonusItem->setSpriteKey("bonus_neutral_resurrect");
                                break;
                            default:
                                bonusItem->setSpriteKey("bonus_neutral_pandora");
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
            }
            else
            {
                m_blockItems[i][j] = NULL;
                m_bonusItems[i][j] = NULL;
            }
        }
    }
    
    // Create the Bomb items
    foreach(Bomb* bomb, m_tempBombList)
    {
        createBombItem(bomb);
    }
    if(!m_tempBombList.isEmpty())
    {
        BombExplosionItem* bombExplosionItem;
        QHash<BombItem*, QList<BombExplosionItem*> >::iterator i = m_bombItems.begin();
        while (i != m_bombItems.end())
        {
            i.key()->pauseAnim();
            i++;
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
    
    //create the background
    m_arenaBackground = new KGameRenderedItem(m_rendererBackground, "background");
    m_arenaBackground->setZValue(-5);
    m_arenaBackground->setPos(0, 0);
    m_arenaBackground->setCacheMode(QGraphicsItem::NoCache);  // if cache is set, there are some artifacts; pay attention, that the KGameRenderer cache is used nevertheless
    m_arenaBackground->setRenderSize(QSize(100, 100)); //just to get something in the background, until the right size is rendered
    addItem(m_arenaBackground);
    
    resizeSprites();
    resizeBackground();
}

GameScene::~GameScene()
{
    delete m_backgroundResizeTimer;
    
    cleanUp();
    
    delete m_infoOverlay;
    delete m_infoSidebar;
    
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
        iteratorRendererPlayer++;
    }
    
    delete m_rendererSelectedTheme;
    delete m_rendererDefaultTheme;
}

void GameScene::cleanUp()
{
    cleanUpItemsWithGraphicsFromTheme();
    
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

void GameScene::cleanUpItemsWithGraphicsFromTheme()
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
        if(dynamic_cast <Bomb*> (i.key()->getModel())->isDetonated())
        {
            dynamic_cast <Bomb*> (i.key()->getModel())->slot_detonationCompleted();
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
    
    removeItem(m_arenaBackground);
    delete m_arenaBackground;
}

void GameScene::themeChanged(const KgTheme* newTheme)
{
    m_tempBombList.clear();
    QHash<BombItem*, QList<BombExplosionItem*> >::iterator i = m_bombItems.begin();
    while (i != m_bombItems.end())
    {
        if(!(dynamic_cast <Bomb*> (i.key()->getModel())->isDetonated()))
        {
            m_tempBombList.append(dynamic_cast <Bomb*> (i.key()->getModel()));
        }
        i++;
    }
    
    cleanUpItemsWithGraphicsFromTheme();
    setupThemeRenderer();
    initItemsWithGraphicsFromTheme();
    
    m_tempBombList.clear();
}

KGameRenderer* GameScene::renderer(Granatier::Element::Type type, Player* player)
{
    switch(type)
    {
        case Granatier::Element::BLOCK:
            return m_rendererArenaItems;
        case Granatier::Element::BONUS:
            return m_rendererBonusItems;
        case Granatier::Element::BOMB:
            return m_rendererBombItems;
        case Granatier::Element::PLAYER:
            return m_mapRendererPlayerItems.value(player);
        case Granatier::Element::SCORE:
            return m_rendererScoreItems;
        default:
            return NULL;
    }
}

void GameScene::showScore()
{
    m_infoOverlay->showScore();
}

void GameScene::resizeSprites()
{
    if(views().isEmpty())
    {
        return;
    }
    
    //calculate the scaling factor for the SVGs
    int horizontalPixelsPerCell = views().first()->size().width() / (m_minSize.width()/Granatier::CellSize);
    int verticalPixelsPerCell = views().first()->size().height() / (m_minSize.height()/Granatier::CellSize);
    if(horizontalPixelsPerCell < verticalPixelsPerCell)
    {
        m_SvgScaleFactor = Granatier::CellSize / horizontalPixelsPerCell;
    }
    else
    {
        m_SvgScaleFactor = Granatier::CellSize / verticalPixelsPerCell;
    }
    QTransform transform;
    transform.scale(1/m_SvgScaleFactor, 1/m_SvgScaleFactor);
    //transform.translate(-20,-10);
    views().first()->setTransform(transform);
    views().first()->centerOn(sceneRect().center());
    views().first()->updateSceneRect(m_minSize);;
    
    //update pixmaps
    emit resizeGraphics(m_SvgScaleFactor);
    
    //update overlay
    QRect viewRect = views().first()->rect();
    QRectF viewRectToScene = QRectF(views().first()->mapToScene(viewRect.topLeft()), views().first()->mapToScene(viewRect.bottomRight()));
    m_infoOverlay->resizeDimmOverlay(viewRectToScene.x(), viewRectToScene.y(), viewRectToScene.width(), viewRectToScene.height());
    
    //update background pixmap
    m_arenaBackground->setPos(views().first()->mapToScene(viewRect.topLeft()));
    m_arenaBackground->setScale(m_SvgScaleFactor);
    
    m_arenaBackground->setPixmap(m_arenaBackground->pixmap().scaled(viewRect.size()));
    
    m_backgroundResizeTimer->stop();
    m_backgroundResizeTimer->start(250);
}

void GameScene::resizeBackground()
{
    if(views().isEmpty())
    {
        return;
    }
    QRect viewRect = views().first()->rect();
    m_arenaBackground->setRenderSize(viewRect.size());
}

Game* GameScene::getGame() const
{
    return m_game;
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

void GameScene::updateInfo(const Granatier::Info::Type p_info)
{
    if(p_info == Granatier::Info::TimeInfo)
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
            m_remainingTimeLabel->setPos(Granatier::CellSize * m_game->getArena()->getNbColumns() - m_remainingTimeLabel->boundingRect().width(), - m_remainingTimeLabel->boundingRect().height());
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
    connect(bombItem, SIGNAL(animationFrameChanged(BombItem*,int)), this, SLOT(updateBombExplosionItemAnimation(BombItem*,int)));
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
    QList<Element*> blockElements;
    QList<Element*> bombElements;
    int nBombPower = bomb->bombPower();
    int nNumberOfColums = m_game->getArena()->getNbColumns();
    int nNumberOfRows = m_game->getArena()->getNbRows();
    int nColumn;
    int nRow;
    qreal xPos = 0;
    qreal yPos = 0;
    bool abDirectionsDone[] = {false, false, false, false};
    int nDetonationCountdown = 75;
    int anDirectionDetonationCountdown[] = {nDetonationCountdown, nDetonationCountdown, nDetonationCountdown, nDetonationCountdown};
    
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
        bombElements = m_game->getArena()->getCell(nRow, nColumn).getElements(Granatier::Element::BOMB);
        int tempBombDetonationCountdown = nDetonationCountdown;
        foreach(element, bombElements)
        {
            if(dynamic_cast <Bomb*> (element) != bomb && !(dynamic_cast <Bomb*> (element)->isDetonated()))
            {
                dynamic_cast <Bomb*> (element)->initDetonation(bomb->explosionID(), tempBombDetonationCountdown);
                tempBombDetonationCountdown += 10;
            }
        }
        
        blockElements = m_game->getArena()->getCell(nRow, nColumn).getElements(Granatier::Element::BLOCK);
        if(!blockElements.isEmpty())
        {
            foreach(element, blockElements)
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
        }
        else if(m_bonusItems[nRow][nColumn] != NULL)
        {
            m_bonusItems[nRow][nColumn]->initDestruction(bomb->explosionID());
        }
    }
    
    for(int i = 0; i < nBombPower; i++)
    {
        Granatier::Direction::Type direction = Granatier::Direction::NORTH;
        do
        {
            switch(direction)
            {
                case Granatier::Direction::NORTH:
                    xPos = bomb->getX();
                    yPos = bomb->getY() - (i+1)*Granatier::CellSize;
                    break;
                case Granatier::Direction::EAST:
                    xPos = bomb->getX() + (i+1)*Granatier::CellSize;
                    yPos = bomb->getY();
                    break;
                case Granatier::Direction::SOUTH:
                    xPos = bomb->getX();
                    yPos = bomb->getY() + (i+1)*Granatier::CellSize;
                    break;
                case Granatier::Direction::WEST:
                    xPos = bomb->getX() - (i+1)*Granatier::CellSize;
                    yPos = bomb->getY();
                    break;
            }
            nColumn = m_game->getArena()->getColFromX(xPos);
            nRow = m_game->getArena()->getRowFromY(yPos);
            
            if(!abDirectionsDone[direction] && nColumn >= 0 && nColumn < nNumberOfColums && nRow >= 0 && nRow < nNumberOfRows)
            {
                bombElements = m_game->getArena()->getCell(nRow, nColumn).getElements(Granatier::Element::BOMB);
                blockElements = m_game->getArena()->getCell(nRow, nColumn).getElements(Granatier::Element::BLOCK);
                if(m_game->getArena()->getCell(nRow, nColumn).getType() != Granatier::Cell::WALL)
                {
                    int tempBombDetonationCountdown = anDirectionDetonationCountdown[direction];
                    bool increaseDetonationTimeout = false;
                    foreach(element, bombElements)
                    {
                        if(dynamic_cast <Bomb*> (element) != bomb && !(dynamic_cast <Bomb*> (element)->isDetonated()))
                        {
                            dynamic_cast <Bomb*> (element)->initDetonation(bomb->explosionID(), tempBombDetonationCountdown);
                            tempBombDetonationCountdown += 10;
                            increaseDetonationTimeout = true;
                        }
                    }
                    if(increaseDetonationTimeout)
                    {
                        anDirectionDetonationCountdown[direction] += nDetonationCountdown;
                    }
                    
                    if(!blockElements.isEmpty())
                    {
                        abDirectionsDone[direction] = true;
                        foreach(element, blockElements)
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
                    }
                    else if(m_bonusItems[nRow][nColumn] != NULL)
                    {
                        m_bonusItems[nRow][nColumn]->initDestruction(bomb->explosionID());
                    }
                    
                    bombExplosionItem = new BombExplosionItem (bomb, direction, nBombPower - i, m_rendererBombItems, m_SvgScaleFactor);
                    bombExplosionItem->setPosition(xPos, yPos);
                    bombExplosionItem->setZValue(300 + nBombPower+3 - i);
                    connect(this, SIGNAL(resizeGraphics(qreal)), bombExplosionItem, SLOT(updateGraphics(qreal)));
                    addItem(bombExplosionItem);
                    m_bombItems[bombItem].append(bombExplosionItem);
                }
                else
                {
                    abDirectionsDone[direction] = true;
                }
            }
            else
            {
                abDirectionsDone[direction] = true;
            }
            
            switch(direction)
            {
                case Granatier::Direction::NORTH:
                    direction = Granatier::Direction::EAST;
                    break;
                case Granatier::Direction::EAST:
                    direction = Granatier::Direction::SOUTH;
                    break;
                case Granatier::Direction::SOUTH:
                    direction = Granatier::Direction::WEST;
                    break;
                case Granatier::Direction::WEST:
                    direction = Granatier::Direction::NORTH;
                    break;
            }
        }
        while(direction != Granatier::Direction::NORTH);
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
