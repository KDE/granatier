/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
    SPDX-FileCopyrightText: 2007-2008 Alexandre Galinier <alex.galinier@hotmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
#include "granatier_random.h"

#include <KgTheme>
#include <KgThemeProvider>
#include <KLocalizedString>

#include <QGraphicsView>
#include <QTimer>

#include <KGameRenderer>
#include <KGameRenderedItem>

#include <cmath>
#include <QStandardPaths>

GameScene::GameScene(Game* p_game, KgThemeProvider* p_themeProvider) : m_game(p_game), m_themeProvider(p_themeProvider)
{
    connect(p_game, &Game::gameStarted, this, &GameScene::start);
    connect(p_game, &Game::pauseChanged, this, &GameScene::setPaused);
    connect(p_game, &Game::bombCreated, this, &GameScene::createBombItem);
    connect(p_game, &Game::infoChanged, this, &GameScene::updateInfo);

    m_SvgScaleFactor = 1;

    m_backgroundResizeTimer = new QTimer();
    m_backgroundResizeTimer->setSingleShot(true);
    connect(m_backgroundResizeTimer, &QTimer::timeout, this, &GameScene::resizeBackground);

    // Create the PlayerItems and the points labels
    QList <Player*> players = p_game->getPlayers();

    PlayerItem* playerItem;
    for(auto & player : players)
    {
        const QString desktopPath = player->getDesktopFilePath();
        KgTheme* theme = new KgTheme(desktopPath.toUtf8());
        theme->readFromDesktopFile(desktopPath);
        auto  playerRenderer = new KGameRenderer(theme);
        m_mapRendererPlayerItems.insert(player, playerRenderer);
        playerItem = new PlayerItem(player, playerRenderer);
        // Corrects the position of the player
        playerItem->update(player->getX(), player->getY());
        // Stops the player animation
        playerItem->stopAnim();

        m_playerItems.append(playerItem);

        connect(this, &GameScene::resizeGraphics, playerItem, &PlayerItem::updateGraphics);
        connect(playerItem, &PlayerItem::bonusItemTaken, this, &GameScene::removeBonusItem);
    }

    // The remaining time
    m_remainingTimeLabel = new QGraphicsTextItem(i18n("0:00"));
    m_remainingTimeLabel->setFont(QFont(QStringLiteral("Helvetica"), static_cast<int>(Granatier::CellSize * 0.35), QFont::Bold, false));
    m_remainingTimeLabel->setDefaultTextColor(QColor("#FFFF00"));
    m_remainingTimeLabel->setZValue(1000);

    m_arenaNameLabel = new QGraphicsTextItem(i18n("Arena Name"));
    m_arenaNameLabel->setFont(QFont(QStringLiteral("Helvetica"), static_cast<int>(Granatier::CellSize * 0.35), QFont::Bold, false));
    m_arenaNameLabel->setDefaultTextColor(QColor("#FFFF00"));
    m_arenaNameLabel->setZValue(1000);

    // setup the theme renderer
    m_rendererSelectedTheme = new KGameRenderer(m_themeProvider);
    m_rendererDefaultTheme = nullptr;
    setupThemeRenderer();

    connect(m_themeProvider, &KgThemeProvider::currentThemeChanged, this, &GameScene::themeChanged);

    // create the info overlay
    m_infoOverlay = new InfoOverlay(m_game, this);
    connect(this, &GameScene::resizeGraphics, m_infoOverlay, &InfoOverlay::updateGraphics);

    init();
}

void GameScene::setupThemeRenderer()
{
    bool selectedThemeIsDefault = true;

    if(m_themeProvider->currentTheme() != m_themeProvider->defaultTheme())
    {
        // Load the default SVG file as fallback
        selectedThemeIsDefault = false;
        if(m_rendererDefaultTheme == nullptr)
        {
            KgTheme* theme = new KgTheme(m_themeProvider->defaultTheme()->identifier());
            theme->setGraphicsPath(m_themeProvider->defaultTheme()->graphicsPath());
            m_rendererDefaultTheme = new KGameRenderer(theme);
        }
    }

    if(selectedThemeIsDefault || m_rendererSelectedTheme->spriteExists(QStringLiteral("background")))
    {
        m_rendererBackground = m_rendererSelectedTheme;
    }
    else
    {
        m_rendererBackground = m_rendererDefaultTheme;
    }

    // set the renderer for the arena items TODO: add all the arena items
    if(selectedThemeIsDefault || (m_rendererSelectedTheme->spriteExists(QStringLiteral("arena_ground")) &&
        m_rendererSelectedTheme->spriteExists(QStringLiteral("arena_wall")) &&
        m_rendererSelectedTheme->spriteExists(QStringLiteral("arena_block")) &&
        m_rendererSelectedTheme->spriteExists(QStringLiteral("arena_block_highlight")) &&
        m_rendererSelectedTheme->spriteExists(QStringLiteral("arena_ice")) &&
        m_rendererSelectedTheme->spriteExists(QStringLiteral("arena_bomb_mortar")) &&
        m_rendererSelectedTheme->spriteExists(QStringLiteral("arena_arrow_up")) &&
        m_rendererSelectedTheme->spriteExists(QStringLiteral("arena_arrow_right")) &&
        m_rendererSelectedTheme->spriteExists(QStringLiteral("arena_arrow_down")) &&
        m_rendererSelectedTheme->spriteExists(QStringLiteral("arena_arrow_left"))))
    {
        m_rendererArenaItems = m_rendererSelectedTheme;
    }
    else
    {
        m_rendererArenaItems = m_rendererDefaultTheme;
    }

    // set the renderer for the bonus items TODO: add all the bonus items
    if(selectedThemeIsDefault || (m_rendererSelectedTheme->spriteExists(QStringLiteral("bonus_speed")) &&
        m_rendererSelectedTheme->spriteExists(QStringLiteral("bonus_bomb")) &&
        m_rendererSelectedTheme->spriteExists(QStringLiteral("bonus_power")) &&
        m_rendererSelectedTheme->spriteExists(QStringLiteral("bonus_shield")) &&
        m_rendererSelectedTheme->spriteExists(QStringLiteral("bonus_throw")) &&
        m_rendererSelectedTheme->spriteExists(QStringLiteral("bonus_kick")) &&
        m_rendererSelectedTheme->spriteExists(QStringLiteral("bonus_bad_slow")) &&
        m_rendererSelectedTheme->spriteExists(QStringLiteral("bonus_bad_hyperactive")) &&
        m_rendererSelectedTheme->spriteExists(QStringLiteral("bonus_bad_mirror")) &&
        m_rendererSelectedTheme->spriteExists(QStringLiteral("bonus_bad_scatty")) &&
        m_rendererSelectedTheme->spriteExists(QStringLiteral("bonus_bad_restrain")) &&
        m_rendererSelectedTheme->spriteExists(QStringLiteral("bonus_neutral_pandora")) &&
        m_rendererSelectedTheme->spriteExists(QStringLiteral("bonus_neutral_resurrect"))))
    {
        m_rendererBonusItems = m_rendererSelectedTheme;
    }
    else
    {
        m_rendererBonusItems = m_rendererDefaultTheme;
    }

    // set the renderer for the bomb items
    if(selectedThemeIsDefault || (m_rendererSelectedTheme->spriteExists(QStringLiteral("bomb")) &&
        m_rendererSelectedTheme->spriteExists(QStringLiteral("bomb_blast_core_0")) &&
        m_rendererSelectedTheme->spriteExists(QStringLiteral("bomb_blast_north_0")) &&
        m_rendererSelectedTheme->spriteExists(QStringLiteral("bomb_blast_east_0")) &&
        m_rendererSelectedTheme->spriteExists(QStringLiteral("bomb_blast_south_0")) &&
        m_rendererSelectedTheme->spriteExists(QStringLiteral("bomb_blast_west_0"))))
    {
        m_rendererBombItems = m_rendererSelectedTheme;
    }
    else
    {
        m_rendererBombItems = m_rendererDefaultTheme;
    }

    // set the renderer for the score items
    if(selectedThemeIsDefault || (m_rendererSelectedTheme->spriteExists(QStringLiteral("score_star_enabled")) &&
        m_rendererSelectedTheme->spriteExists(QStringLiteral("score_star_disabled"))))
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
    for (auto & playerItem : m_playerItems)
    {
        if(!items().contains(playerItem))
        {
            addItem(playerItem);
        }
        playerItem->resurrect();
    }

    if (!items().contains(m_remainingTimeLabel))
    {
        addItem(m_remainingTimeLabel);
    }
    m_remainingTimeLabel->setDefaultTextColor(QColor("#FFFF00"));
    int nTime = m_game->getRemainingTime();
    m_remainingTimeLabel->setPlainText(QStringLiteral("%1:%2").arg(nTime/60).arg(nTime%60, 2, 10, QLatin1Char('0')));
    m_remainingTimeLabel->setPos(Granatier::CellSize * m_game->getArena()->getNbColumns() - m_remainingTimeLabel->boundingRect().width(), - m_remainingTimeLabel->boundingRect().height());

    if (!items().contains(m_arenaNameLabel))
    {
        addItem(m_arenaNameLabel);
    }
    m_arenaNameLabel->setPlainText(m_game->getArena()->getName());
    m_arenaNameLabel->setPos(0, - m_arenaNameLabel->boundingRect().height());

    //this is needed for info sidebar
    setSceneRect(0, -m_remainingTimeLabel->boundingRect().height(),
                 m_game->getArena()->getNbColumns()*Granatier::CellSize,
                 m_game->getArena()->getNbRows()*Granatier::CellSize + m_remainingTimeLabel->boundingRect().height());

    // create the info sidebar
    m_infoSidebar = new InfoSidebar(m_game, this);
    connect(this, &GameScene::resizeGraphics, m_infoSidebar, &InfoSidebar::updateGraphics);

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
    m_infoSidebar->reset();

    m_infoOverlay->showGetReady();

    //set the minimum size for the scene
    m_minSize = sceneRect();
    int minWidth = static_cast<int>((m_minSize.width() / Granatier::CellSize + 1.1) * Granatier::CellSize);
    int minHeight = static_cast<int>((m_minSize.height() / Granatier::CellSize + 1.1) * Granatier::CellSize);
    m_minSize.setX(m_minSize.x() + (m_minSize.width() - minWidth) / 10);
    m_minSize.setY(m_minSize.y() + (m_minSize.height() - minHeight) / 4);
    m_minSize.setWidth(minWidth);
    m_minSize.setHeight(minHeight);
    setSceneRect(m_minSize);

    resizeSprites();
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
            ArenaItem* arenaItem = new ArenaItem(j * Granatier::CellSize, i * Granatier::CellSize, m_rendererArenaItems, QStringLiteral(""));
            connect(this, &GameScene::resizeGraphics, arenaItem, &ArenaItem::updateGraphics);

            //TODO: use this function call
            //arenaItem->setElementId(m_game->getArena()->getCell(i,j).getElement()->getImageId());
            switch(m_game->getArena()->getCell(i,j).getType())
            {
                case Granatier::Cell::WALL:
                    arenaItem->setSpriteKey(QStringLiteral("arena_wall"));
                    arenaItem->setZValue(-2);
                    break;
                case Granatier::Cell::HOLE:
                    delete arenaItem;
                    arenaItem = nullptr;
                    break;
                case Granatier::Cell::ICE:
                    arenaItem->setSpriteKey(QStringLiteral("arena_ice"));
                    arenaItem->setZValue(0);
                    break;
                case Granatier::Cell::BOMBMORTAR:
                    arenaItem->setSpriteKey(QStringLiteral("arena_bomb_mortar"));
                    arenaItem->setZValue(0);
                    break;
                case Granatier::Cell::ARROWUP:
                    arenaItem->setSpriteKey(QStringLiteral("arena_arrow_up"));
                    arenaItem->setZValue(0);
                    break;
                case Granatier::Cell::ARROWRIGHT:
                    arenaItem->setSpriteKey(QStringLiteral("arena_arrow_right"));
                    arenaItem->setZValue(0);
                    break;
                case Granatier::Cell::ARROWDOWN:
                    arenaItem->setSpriteKey(QStringLiteral("arena_arrow_down"));
                    arenaItem->setZValue(0);
                    break;
                case Granatier::Cell::ARROWLEFT:
                    arenaItem->setSpriteKey(QStringLiteral("arena_arrow_left"));
                    arenaItem->setZValue(0);
                    break;
                case Granatier::Cell::GROUND:
                case Granatier::Cell::BLOCK:
                default:
                    arenaItem->setSpriteKey(QStringLiteral("arena_ground"));
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
                for(const auto& element: qAsConst(blockElements))
                {
                    Block* block = dynamic_cast <Block*> (element);
                    BlockItem* blockItem = new BlockItem(block, m_rendererArenaItems);
                    connect(this, &GameScene::resizeGraphics, blockItem, &BlockItem::updateGraphics);
                    blockItem->setSpriteKey(block->getImageId());
                    blockItem->update(block->getX(), block->getY());
                    blockItem->setZValue(200);
                    if(Settings::self()->showAllTiles() == 1)
                    {
                        blockItem->setZValue(99);
                    }
                    connect(this, &GameScene::resizeGraphics, blockItem, &BlockItem::updateGraphics);
                    connect(blockItem, &BlockItem::blockItemDestroyed, this, &GameScene::removeBlockItem);
                    m_blockItems[i][j] = blockItem;
                    // if the block contains a hidden bonus, create the bonus item
                    Bonus* bonus = block->getBonus();
                    if(bonus)
                    {
                        BonusItem* bonusItem = new BonusItem(bonus, m_rendererBonusItems);
                        switch(bonus->getBonusType())
                        {
                            case Granatier::Bonus::SPEED:
                                bonusItem->setSpriteKey(QStringLiteral("bonus_speed"));
                                break;
                            case Granatier::Bonus::BOMB:
                                bonusItem->setSpriteKey(QStringLiteral("bonus_bomb"));
                                break;
                            case Granatier::Bonus::POWER:
                                bonusItem->setSpriteKey(QStringLiteral("bonus_power"));
                                break;
                            case Granatier::Bonus::SHIELD:
                                bonusItem->setSpriteKey(QStringLiteral("bonus_shield"));
                                break;
                            case Granatier::Bonus::THROW:
                                bonusItem->setSpriteKey(QStringLiteral("bonus_throw"));
                                break;
                            case Granatier::Bonus::KICK:
                                bonusItem->setSpriteKey(QStringLiteral("bonus_kick"));
                                break;
                            case Granatier::Bonus::HYPERACTIVE:
                                bonusItem->setSpriteKey(QStringLiteral("bonus_bad_hyperactive"));
                                break;
                            case Granatier::Bonus::SLOW:
                                bonusItem->setSpriteKey(QStringLiteral("bonus_bad_slow"));
                                break;
                            case Granatier::Bonus::MIRROR:
                                bonusItem->setSpriteKey(QStringLiteral("bonus_bad_mirror"));
                                break;
                            case Granatier::Bonus::SCATTY:
                                bonusItem->setSpriteKey(QStringLiteral("bonus_bad_scatty"));
                                break;
                            case Granatier::Bonus::RESTRAIN:
                                bonusItem->setSpriteKey(QStringLiteral("bonus_bad_restrain"));
                                break;
                            case Granatier::Bonus::RESURRECT:
                                bonusItem->setSpriteKey(QStringLiteral("bonus_neutral_resurrect"));
                                break;
                            default:
                                bonusItem->setSpriteKey(QStringLiteral("bonus_neutral_pandora"));
                        }

                        if(granatier::RNG::fromRange(0, 10) > 9 && bonusItem->spriteKey() != QStringLiteral("bonus_neutral_resurrect"))
                        {
                            bonusItem->setSpriteKey(QStringLiteral("bonus_neutral_pandora"));
                        }

                        bonusItem->update(bonus->getX(), bonus->getY());
                        bonusItem->setZValue(100);
                        m_bonusItems[i][j] = bonusItem;

                        connect(this, &GameScene::resizeGraphics, bonusItem, &BonusItem::updateGraphics);
                        connect(bonusItem, &BonusItem::bonusItemDestroyed, this, &GameScene::removeBonusItem);

                        addItem(bonusItem);
                        if(Settings::self()->showAllTiles() == 0)
                        {
                            bonusItem->hide();
                        }
                    }
                    else
                    {
                        m_bonusItems[i][j] = nullptr;
                    }
                }
            }
            else
            {
                m_blockItems[i][j] = nullptr;
                m_bonusItems[i][j] = nullptr;
            }
        }
    }

    // Create the Bomb items
    for(auto& bomb: m_tempBombList)
    {
        createBombItem(bomb);
    }
    if(!m_tempBombList.isEmpty())
    {
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
            if(m_arenaItem[i][j] != nullptr)
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
            if (m_blockItems[i][j] != nullptr)
            {
                if (!items().contains(m_blockItems[i][j]))
                {
                    addItem(m_blockItems[i][j]);
                }
            }
        }
    }

    //create the background
    m_arenaBackground = new KGameRenderedItem(m_rendererBackground, QStringLiteral("background"));
    m_arenaBackground->setZValue(-5);
    m_arenaBackground->setPos(0, 0);
    m_arenaBackground->setCacheMode(QGraphicsItem::NoCache);  // if cache is set, there are some artifacts; pay attention, that the KGameRenderer cache is used nevertheless
    m_arenaBackground->setRenderSize(QSize(100, 100)); //just to get something in the background, until the right size is rendered
    addItem(m_arenaBackground);

    resizeSprites();
}

GameScene::~GameScene()
{
    delete m_backgroundResizeTimer;

    cleanUp();

    delete m_infoOverlay;

    for (auto & playerItem : m_playerItems)
    {
        if(items().contains(playerItem))
        {
            removeItem(playerItem);
        }
        playerItem->stopAnim();
        delete playerItem;
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

    delete m_infoSidebar;

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
            if (m_arenaItem[i][j] != nullptr)
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
            if (m_blockItems[i][j] != nullptr)
            {
                if (items().contains(m_blockItems[i][j]))
                {
                    removeItem(m_blockItems[i][j]);
                }
                delete m_blockItems[i][j];
            }
            if (m_bonusItems[i][j] != nullptr)
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

void GameScene::themeChanged()
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
    m_infoSidebar->themeChanged();
    m_infoOverlay->themeChanged();

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
            return nullptr;
    }
}

void GameScene::showScore()
{
    m_infoOverlay->showScore();
}

void GameScene::resizeSprites(int delayForBackground)
{
    if(views().isEmpty())
    {
        return;
    }

    //calculate the scaling factor for the SVGs
    int horizontalPixelsPerCell = static_cast<int>(views().constFirst()->size().width() / (m_minSize.width()/Granatier::CellSize));
    int verticalPixelsPerCell = static_cast<int>(views().constFirst()->size().height() / (m_minSize.height()/Granatier::CellSize));
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
    views().constFirst()->setTransform(transform);
    views().constFirst()->centerOn(sceneRect().center());
    views().constFirst()->updateSceneRect(m_minSize);;

    //update pixmaps
    Q_EMIT resizeGraphics(m_SvgScaleFactor);

    //update overlay
    QRect viewRect = views().constFirst()->rect();
    QRectF viewRectToScene = QRectF(views().constFirst()->mapToScene(viewRect.topLeft()), views().constFirst()->mapToScene(viewRect.bottomRight()));
    m_infoOverlay->resizeDimmOverlay(viewRectToScene.x(), viewRectToScene.y(), viewRectToScene.width(), viewRectToScene.height());

    //update background pixmap
    m_arenaBackground->setPos(views().constFirst()->mapToScene(viewRect.topLeft()));
    m_arenaBackground->setScale(m_SvgScaleFactor);

    m_arenaBackground->setPixmap(m_arenaBackground->pixmap().scaled(viewRect.size()));

    m_backgroundResizeTimer->stop();
    m_backgroundResizeTimer->start(delayForBackground);
}

void GameScene::resizeBackground()
{
    if(views().isEmpty())
    {
        return;
    }
    QRect viewRect = views().constFirst()->rect();
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
        for (auto & playerItem : m_playerItems)
        {
            playerItem->pauseAnim();
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
        for (auto & playerItem : m_playerItems)
        {
            playerItem->resumeAnim();
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
            if (m_blockItems[i][j] != nullptr && m_blockItems[i][j] == blockItem)
            {
                if (items().contains(m_blockItems[i][j]))
                {
                    removeItem(m_blockItems[i][j]);
                    m_blockItems[i][j] = nullptr;
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
            if (m_bonusItems[i][j] != nullptr && m_bonusItems[i][j] == bonusItem)
            {
                if (items().contains(m_bonusItems[i][j]))
                {
                    removeItem(m_bonusItems[i][j]);
                    m_bonusItems[i][j] = nullptr;
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
            m_remainingTimeLabel->setPlainText(QStringLiteral("%1:%2").arg(nTime/60).arg(nTime%60, 2, 10, QLatin1Char('0')));
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
    m_bombItems[bombItem].append(nullptr);

    bombItem->updateGraphics(m_SvgScaleFactor); //TODO: use a Renderer class and get the scale factor from a static function during initialization

    connect(this, &GameScene::resizeGraphics, bombItem, &BombItem::updateGraphics);
    connect(bomb, &Bomb::mortar, bombItem, &BombItem::updateMortar);
    connect(bomb, &Bomb::bombDetonated, this, &GameScene::bombDetonated);
    connect(bombItem, &BombItem::bombItemFinished, this, &GameScene::removeBombItem);
    connect(bombItem, &BombItem::animationFrameChanged, this, &GameScene::updateBombExplosionItemAnimation);
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
    BombItem* bombItem = nullptr;
    BombExplosionItem* bombExplosionItem = nullptr;
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
        for(const auto& element: qAsConst(bombElements))
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
            for(const auto& element: qAsConst(blockElements))
            {
                dynamic_cast <Block*> (element)->startDestruction();
                if (m_blockItems[nRow][nColumn] != nullptr)
                {
                    //display bonus if available
                    if (m_bonusItems[nRow][nColumn] != nullptr)
                    {
                        m_bonusItems[nRow][nColumn]->setUndestroyable(bomb->explosionID());
                        m_bonusItems[nRow][nColumn]->show();
                    }
                }
            }
        }
        else if(m_bonusItems[nRow][nColumn] != nullptr)
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
                    for(const auto& element: qAsConst(bombElements))
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
                        for(const auto& element: qAsConst(blockElements))
                        {
                            dynamic_cast <Block*> (element)->startDestruction();
                            if (m_blockItems[nRow][nColumn] != nullptr)
                            {
                                //display bonus if available
                                if (m_bonusItems[nRow][nColumn] != nullptr)
                                {
                                    m_bonusItems[nRow][nColumn]->setUndestroyable(bomb->explosionID());
                                    m_bonusItems[nRow][nColumn]->show();
                                }
                            }
                        }
                    }
                    else if(m_bonusItems[nRow][nColumn] != nullptr)
                    {
                        m_bonusItems[nRow][nColumn]->initDestruction(bomb->explosionID());
                    }

                    bombExplosionItem = new BombExplosionItem (bomb, direction, nBombPower - i, m_rendererBombItems, m_SvgScaleFactor);
                    bombExplosionItem->setPosition(xPos, yPos);
                    connect(this, &GameScene::resizeGraphics, bombExplosionItem, &BombExplosionItem::updateGraphics);
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
