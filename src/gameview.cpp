/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "gameview.h"
#include "gamescene.h"
#include "game.h"

#include <QKeyEvent>
#include <QTimer>

GameView::GameView(GameScene* p_scene, Game * p_game) : QGraphicsView(p_scene)
{
    setFrameStyle(QFrame::NoFrame);
    setFocusPolicy(Qt::StrongFocus);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Forward the key press events to the Game instance
    connect(this, &GameView::keyPressed, p_game, &Game::keyPressEvent);
    connect(this, &GameView::keyReleased, p_game, &Game::keyReleaseEvent);
}

GameView::~GameView()
= default;

void GameView::resizeEvent(QResizeEvent*)
{
    dynamic_cast <GameScene*> (scene())->resizeSprites(250);
}

void GameView::focusOutEvent(QFocusEvent*)
{
    // Pause the game if it is not already paused
    if (((GameScene*)scene())->getGame()->getTimer()->isActive())
    {
        ((GameScene*)scene())->getGame()->switchPause();
    }
}

void GameView::keyPressEvent(QKeyEvent* p_event)
{
    if(p_event->isAutoRepeat())
    {
        return;
    }
    Q_EMIT keyPressed(p_event);
}

void GameView::keyReleaseEvent(QKeyEvent* p_event)
{
    if(p_event->isAutoRepeat())
    {
        return;
    }
    Q_EMIT keyReleased(p_event);
}

#include "moc_gameview.cpp"
