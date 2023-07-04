/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "infooverlay.h"
#include "game.h"
#include "gamescene.h"
#include "player.h"
#include "settings.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <KLocalizedString>
#include <KGameRenderer>
#include <KGameRenderedItem>

InfoOverlay::InfoOverlay (Game* p_game, GameScene* p_scene) : QObject()
{
    m_game = p_game;
    m_gameScene = p_scene;
    m_svgScaleFactor = 1;

    int nWinPoints = m_game->getWinPoints();
    const QList <Player*> playerList = m_game->getPlayers();

    KGameRenderer* renderer;
    renderer = m_gameScene->renderer(Granatier::Element::SCORE);

    //create the labels
    for(const auto& player: playerList)
    {
        QList <KGameRenderedItem*> svgItemList;
        auto* playerName = new QGraphicsTextItem(player->getPlayerName());
        playerName->setFont(QFont(QStringLiteral("Helvetica"), static_cast<int>(Granatier::CellSize * 0.35), QFont::Bold, false));
        playerName->setDefaultTextColor(QColor("#FFFF00"));
        playerName->setZValue(2001);

        for(int j = 0; j < nWinPoints; j++)
        {
            auto* score = new KGameRenderedItem(renderer, QStringLiteral("score_star_disabled"));
            score->setZValue(2001);
            svgItemList.append(score);
        }
        m_mapScore.insert(player, svgItemList);
        m_mapPlayerNames.insert(player, playerName);
    }

    QString strContinue (i18n("Press Space to continue"));

    m_continueLabel = new QGraphicsTextItem(strContinue);
    m_continueLabel->setFont(QFont(QStringLiteral("Helvetica"), static_cast<int>(Granatier::CellSize * 0.35), QFont::Bold, false));
    m_continueLabel->setDefaultTextColor(QColor("#FFFF00"));
    m_continueLabel->setZValue(2001);

    m_newGameLabel = new QGraphicsTextItem(i18n("Press Space to start a new Game"));
    m_newGameLabel->setFont(QFont(QStringLiteral("Helvetica"), static_cast<int>(Granatier::CellSize * 0.35), QFont::Bold, false));
    m_newGameLabel->setDefaultTextColor(QColor("#FFFF00"));
    m_newGameLabel->setZValue(2001);

    m_gameOverLabel = new QGraphicsTextItem;;
    m_gameOverLabel->setFont(QFont(QStringLiteral("Helvetica"), static_cast<int>(Granatier::CellSize * 0.5), QFont::Bold, false));
    m_gameOverLabel->setDefaultTextColor(QColor("#FFFF00"));
    m_gameOverLabel->setZValue(2001);

    m_getReadyLabel = new QGraphicsTextItem(i18n("GET READY !!!"));
    m_getReadyLabel->setFont(QFont(QStringLiteral("Helvetica"), static_cast<int>(Granatier::CellSize * 0.6), QFont::Bold, false));
    m_getReadyLabel->setDefaultTextColor(QColor("#FFFF00"));
    m_getReadyLabel->setZValue(2001);

    m_startGameLabel = new QGraphicsTextItem(i18n("Press Space to start"));
    m_startGameLabel->setFont(QFont(QStringLiteral("Helvetica"), static_cast<int>(Granatier::CellSize * 0.35), QFont::Bold, false));
    m_startGameLabel->setDefaultTextColor(QColor("#FFFF00"));
    m_startGameLabel->setZValue(2001);

    m_pauseLabel = new QGraphicsTextItem(i18n("PAUSED"));
    m_pauseLabel->setFont(QFont(QStringLiteral("Helvetica"), static_cast<int>(Granatier::CellSize * 0.8), QFont::Bold, false));
    m_pauseLabel->setDefaultTextColor(QColor("#FFFF00"));
    m_pauseLabel->setZValue(2001);

    m_continueAfterPauseLabel = new QGraphicsTextItem(strContinue);
    m_continueAfterPauseLabel->setFont(QFont(QStringLiteral("Helvetica"), static_cast<int>(Granatier::CellSize * 0.35), QFont::Bold, false));
    m_continueAfterPauseLabel->setDefaultTextColor(QColor("#FFFF00"));
    m_continueAfterPauseLabel->setZValue(2001);

    m_dimmOverlay = new QGraphicsRectItem();
    m_dimmOverlay->setBrush(QBrush(QColor(0,0,0,175)));
    if(Settings::self()->showAllTiles() == 1)
    {
        m_dimmOverlay->setBrush(QBrush(QColor(0,0,0,0)));
    }
    m_dimmOverlay->setZValue(2000);
    m_dimmOverlay->setRect(m_gameScene->sceneRect().x(), m_gameScene->sceneRect().y(),
                           m_gameScene->width(), m_gameScene->height());
}

InfoOverlay::~InfoOverlay()
{
    //remove the items from the scene
    hideItems();

    // Find the score items and remove them
    for(const auto& scoreItems: std::as_const(m_mapScore))
    {
        qDeleteAll(scoreItems);
    }
    m_mapScore.clear(); //TODO: necessary?
    // Find the player name labels and remove them
    qDeleteAll(m_mapPlayerNames);
    m_mapPlayerNames.clear(); //TODO: necessary?

    delete m_continueLabel;
    delete m_newGameLabel;
    delete m_gameOverLabel;
    delete m_getReadyLabel;
    delete m_startGameLabel;
    delete m_pauseLabel;
    delete m_dimmOverlay;
    delete m_continueAfterPauseLabel;
}

void InfoOverlay::showGetReady ()
{
    hideItems();

    if (!m_gameScene->items().contains(m_dimmOverlay))
    {
        m_gameScene->addItem(m_dimmOverlay);
    }

    if (!m_gameScene->items().contains(m_getReadyLabel))
    {
        m_gameScene->addItem(m_getReadyLabel);
    }

    if (!m_gameScene->items().contains(m_startGameLabel))
    {
        m_gameScene->addItem(m_startGameLabel);
    }
}

void InfoOverlay::showPause ()
{
    hideItems();

    if (!m_gameScene->items().contains(m_dimmOverlay))
    {
        m_gameScene->addItem(m_dimmOverlay);
    }

    if (!m_gameScene->items().contains(m_pauseLabel))
    {
        m_gameScene->addItem(m_pauseLabel);
    }

    if (!m_gameScene->items().contains(m_continueAfterPauseLabel))
    {
        m_gameScene->addItem(m_continueAfterPauseLabel);
    }
}

void InfoOverlay::showScore ()
{
    int nWinPoints = m_game->getWinPoints();
    QList <Player*> players = m_game->getPlayers();
    KGameRenderedItem* svgItem;

    hideItems();

    if (!m_gameScene->items().contains(m_dimmOverlay))
    {
        m_gameScene->addItem(m_dimmOverlay);
    }

    for(auto & player : players)
    {
        if (!m_gameScene->items().contains(m_mapPlayerNames.value(player)))
        {
            // display the player name
            m_gameScene->addItem(m_mapPlayerNames.value(player));
        }

        for(int j = 0; j < nWinPoints; j++)
        {
            svgItem = m_mapScore.value(player).at(j);
            if (player->points() > j)
            {
                svgItem->setSpriteKey(QStringLiteral("score_star_enabled"));
            }

            // if the score was not displayed yet
            if (!m_gameScene->items().contains(svgItem))
            {
                // display the score
                m_gameScene->addItem(svgItem);
            }
        }
    }

    if(m_game->getGameOver() == false)
    {
        if (!m_gameScene->items().contains(m_continueLabel))
        {
            m_gameScene->addItem(m_continueLabel);
        }
    }
    else
    {
        if (!m_gameScene->items().contains(m_newGameLabel))
        {
            m_gameScene->addItem(m_newGameLabel);
        }
        if (m_gameScene->items().contains(m_gameOverLabel))
        {
            m_gameScene->removeItem(m_gameOverLabel);
        }
        m_gameOverLabel->setPlainText(i18n("The winner is %1", m_game->getWinner()));
        m_gameOverLabel->setPos(m_gameScene->sceneRect().x() + (m_gameScene->width() - m_gameOverLabel->boundingRect().width())/2,
                                m_gameOverLabel->y());
        m_gameScene->addItem(m_gameOverLabel);
    }
}

void InfoOverlay::hideItems ()
{
    int nWinPoints = m_game->getWinPoints();
    QList <Player*> players = m_game->getPlayers();

    for(auto & player : players)
    {
        if (m_gameScene->items().contains(m_mapPlayerNames.value(player)))
        {
            // display the player name
            m_gameScene->removeItem(m_mapPlayerNames.value(player));
        }

        for(int j = 0; j < nWinPoints; j++)
        {
            KGameRenderedItem* svgItem = m_mapScore.value(player).at(j);
            // if the score is displayed
            if (m_gameScene->items().contains(svgItem))
            {
                // remove the score
                m_gameScene->removeItem(svgItem);
            }
        }
    }

    if (m_gameScene->items().contains(m_continueLabel))
    {
        m_gameScene->removeItem(m_continueLabel);
    }

    if (m_gameScene->items().contains(m_newGameLabel))
    {
        m_gameScene->removeItem(m_newGameLabel);
    }

    if (m_gameScene->items().contains(m_gameOverLabel))
    {
        m_gameScene->removeItem(m_gameOverLabel);
    }

    if (m_gameScene->items().contains(m_getReadyLabel))
    {
        m_gameScene->removeItem(m_getReadyLabel);
    }

    if (m_gameScene->items().contains(m_startGameLabel))
    {
        m_gameScene->removeItem(m_startGameLabel);
    }

    if (m_gameScene->items().contains(m_pauseLabel))
    {
        m_gameScene->removeItem(m_pauseLabel);
    }

    if (m_gameScene->items().contains(m_continueAfterPauseLabel))
    {
        m_gameScene->removeItem(m_continueAfterPauseLabel);
    }

    if (m_gameScene->items().contains(m_dimmOverlay))
    {
        m_gameScene->removeItem(m_dimmOverlay);
    }
}

void InfoOverlay::resizeDimmOverlay(qreal x, qreal y, qreal width, qreal height)
{
    m_dimmOverlay->setRect(x, y, width, height);
}

void InfoOverlay::updateGraphics(qreal svgScaleFactor)
{
    m_svgScaleFactor = svgScaleFactor;
    
    if(m_gameScene->views().isEmpty())
    {
        return;
    }

    int nWinPoints = m_game->getWinPoints();
    QList <Player*> playerList = m_game->getPlayers();
    int nMaxPlayerNameLength = 0;
    int nTop = 0;
    int nLeft = 0;

    QGraphicsTextItem* playerName;
    QGraphicsTextItem em(QStringLiteral("M"));
    em.setFont(QFont(QStringLiteral("Helvetica"), static_cast<int>(Granatier::CellSize * 0.35), QFont::Bold, false));
    //calculate max player name length and top-left position
    for(int i = 0; i < playerList.count(); i++)
    {
        playerName = m_mapPlayerNames.value(playerList.at(i));
        if(nMaxPlayerNameLength < playerName->boundingRect().width())
        {
            nMaxPlayerNameLength = static_cast<int>(playerName->boundingRect().width());
        }
        if(i == playerList.count() - 1)
        {
            nLeft = static_cast<int>(m_gameScene->sceneRect().x() + m_gameScene->width()/2 - (nMaxPlayerNameLength + em.boundingRect().width()/2 + nWinPoints * (Granatier::CellSize * 0.6 + em.boundingRect().width()/5))/2);
            nTop = static_cast<int>(m_gameScene->sceneRect().y() + m_gameScene->height()/2 - (playerList.count()+2)/2 * em.boundingRect().height());
        }
    }

    QSize svgSize;
    QPoint topLeft(0, 0);
    topLeft = m_gameScene->views().constFirst()->mapFromScene(topLeft);

    KGameRenderedItem* score;
    for(int i = 0; i < playerList.count(); i++)
    {
        playerName = m_mapPlayerNames.value(playerList.at(i));
        playerName->setPos(nLeft, nTop + i * em.boundingRect().height() * 1.1);

        for(int j = 0; j < nWinPoints; j++)
        {
            score = m_mapScore.value(playerList.at(i)).at(j);

            QPoint bottomRight(static_cast<int>(Granatier::CellSize * 0.6), static_cast<int>(Granatier::CellSize * 0.6));
            bottomRight = m_gameScene->views().constFirst()->mapFromScene(bottomRight);

            svgSize.setHeight(bottomRight.y() - topLeft.y());
            svgSize.setWidth(bottomRight.x() - topLeft.x());

            score->setRenderSize(svgSize);
            score->setScale(svgScaleFactor);

            score->setPos(nLeft + nMaxPlayerNameLength + em.boundingRect().width()/2 + j * (Granatier::CellSize * 0.6 + em.boundingRect().width()/5),
                          playerName->pos().y() + em.boundingRect().height()/2.5 - (Granatier::CellSize * 0.6) / 2);

        }
    }

    m_continueLabel->setPos(m_gameScene->sceneRect().x() + (m_gameScene->width() - m_continueLabel->boundingRect().width())/2,
                            nTop + (playerList.count()+1) * (m_continueLabel->boundingRect().height()));
    m_newGameLabel->setPos(m_gameScene->sceneRect().x() + (m_gameScene->width() - m_newGameLabel->boundingRect().width())/2,
                           nTop + (playerList.count()+1) * (m_newGameLabel->boundingRect().height()));
    m_gameOverLabel->setPos(m_gameScene->sceneRect().x() + (m_gameScene->width() - m_gameOverLabel->boundingRect().width())/2,
                            nTop - m_gameOverLabel->boundingRect().height());
    m_getReadyLabel->setPos(m_gameScene->sceneRect().x() + (m_gameScene->width() - m_getReadyLabel->boundingRect().width()) / 2,
                            m_gameScene->sceneRect().y() + (m_gameScene->height() - m_getReadyLabel->boundingRect().height() -  m_startGameLabel->boundingRect().height()) / 2);
    m_startGameLabel->setPos(m_gameScene->sceneRect().x() + (m_gameScene->width() - m_startGameLabel->boundingRect().width()) / 2,
                             m_gameScene->sceneRect().y() + (m_gameScene->height() + m_getReadyLabel->boundingRect().height() - m_startGameLabel->boundingRect().height()) / 2);
    m_pauseLabel->setPos(m_gameScene->sceneRect().x() + (m_gameScene->width() - m_pauseLabel->boundingRect().width()) / 2,
                         m_gameScene->sceneRect().y() + (m_gameScene->height() - m_pauseLabel->boundingRect().height() - m_continueAfterPauseLabel->boundingRect().height()) / 2);
    m_continueAfterPauseLabel->setPos(m_gameScene->sceneRect().x() + (m_gameScene->width() - m_continueAfterPauseLabel->boundingRect().width()) / 2,
                                      m_gameScene->sceneRect().y() + (m_gameScene->height() + m_pauseLabel->boundingRect().height() - m_continueAfterPauseLabel->boundingRect().height()) / 2);
}

void InfoOverlay::themeChanged()
{
    KGameRenderer* renderer = m_gameScene->renderer(Granatier::Element::SCORE);
    KGameRenderedItem* tempItem;

    for(auto& score: m_mapScore)
    {
        for(auto& star: score)
        {
            tempItem = star;
            star = new KGameRenderedItem(renderer, tempItem->spriteKey());
            star->setZValue(tempItem->zValue());
            star->setPos(tempItem->pos());

            if(m_gameScene->items().contains(tempItem))
            {
                m_gameScene->removeItem(tempItem);
                m_gameScene->addItem(star);
            }
            delete tempItem;
        }
    }

    updateGraphics(m_svgScaleFactor);
}

#include "moc_infooverlay.cpp"
