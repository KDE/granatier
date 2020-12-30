/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QGraphicsView>

class QKeyEvent;
class Game;
class GameScene;

/**
 * @brief This class manages the drawing of each element of the Game instance.
 * It creates a GameScene instance associated to the given Game instance in order to manage the elements to be drawn at each moment of the game.
 */
class GameView : public QGraphicsView {

Q_OBJECT

public:

    /**
      * Creates a new GameView instance.
      * @param p_scene the Game scene
      * @param p_game the Game instance whose elements have to be drawn
      */
    GameView(GameScene* p_scene, Game* p_game);

    /**
      * Deletes the GameView instance.
      */
    ~GameView() override;

    /**
      * Resizes the items when the view is resized.
      * @param p_event the resize event
      */
    void resizeEvent(QResizeEvent* p_event) override;

protected:

    /**
      * Manages the player actions by hanlding the key press events.
      * @param p_event the key press event
      */
    void keyPressEvent(QKeyEvent* p_event) override;

    /**
      * Manages the player actions by hanlding the key release events.
      * @param p_event the key release event
      */
    void keyReleaseEvent(QKeyEvent* p_event) override;

    /**
      * Pauses the game on focus lost.
      * @param p_event the focus event
      */
    void focusOutEvent(QFocusEvent* p_event) override;

Q_SIGNALS:

    /**
      * Emitted on key press event for the Game instance
      * @param p_event the key press event
      */
    void keyPressed(QKeyEvent* p_event);

    /**
      * Emitted on key release event for the Game instance
      * @param p_event the key release event
      */
    void keyReleased(QKeyEvent* p_event);
};

#endif

