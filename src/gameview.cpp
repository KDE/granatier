/*
 * Copyright 2009 Mathias Kraus <k.hias@gmx.de>
 * Copyright 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
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

#include "gameview.h"
#include "gamescene.h"
#include "game.h"

#include <QKeyEvent>

GameView::GameView(Game * p_game) : QGraphicsView(new GameScene(p_game))
{
    setFrameStyle(QFrame::NoFrame);
    setFocusPolicy(Qt::StrongFocus);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    // Forward the key press events to the Game instance
    connect(this, SIGNAL(keyPressed(QKeyEvent*)), p_game, SLOT(keyPressEvent(QKeyEvent*)));
    connect(this, SIGNAL(keyReleased(QKeyEvent*)), p_game, SLOT(keyReleaseEvent(QKeyEvent*)));
}

GameView::~GameView()
{

}

void GameView::resizeEvent(QResizeEvent*)
{
    dynamic_cast <GameScene*> (scene())->resizeSprites();
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
    emit(keyPressed(p_event));
}

void GameView::keyReleaseEvent(QKeyEvent* p_event)
{
    if(p_event->isAutoRepeat())
    {
        return;
    }
    emit(keyReleased(p_event));
}
