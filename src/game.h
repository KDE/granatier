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

#ifndef GAME_H
#define GAME_H

#include "granatierglobals.h"

#include <QObject>

class GameScene;
class Arena;
class Player;
class Bonus;
class Bomb;
class Block;
class PlayerSettings;
class QTimer;
class QKeyEvent;

class KgSound;

/**
 * @brief This class manages the game main loop : it regularly checks the key press events, computes the character moves and updates their coordinates.
 */
class Game : public QObject
{

Q_OBJECT

private :

    /** Number of FPS */
    static const int FPS;

    /** The game different states : RUNNING, PAUSED_LOCKED, PAUSED_UNLOCKED */
    enum State
    {
        RUNNING,            // Game running
        PAUSED_LOCKED,      // Game paused and user is not allowed to unpause
        PAUSED_UNLOCKED     // Game paused and user is allowed to unpause
    };
    /** A flag for the State enum */
    Q_DECLARE_FLAGS(GameStates, State)

    /** The game state */
    State m_state;

    /** The Game main timer */
    QTimer* m_timer;
    
    /** The Round timer */
    QTimer* m_roundTimer;
    
    /** The timer to add a little delay after the round is finished after there's at most only one player alive */
    QTimer* m_setRoundFinishedTimer;
    
    /** The gamecene */
    GameScene* m_gameScene;
    
    /** The player settings */
    PlayerSettings* m_playerSettings;
    
    /** The Arena */
    Arena* m_arena;

    /** The Players */
    QList<Player*> m_players;

    /** The Bombs */
    QList<Bomb*> m_bombs;
    
    /** The Blocks */
    QList<Block*> m_blocks;
    
    /** The Bonuses */
    QList<Bonus*> m_bonus;

    /** The absolute number of bombs, used for the bomb and explosion ID */
    int m_bombCount;
    
    /** The points which are needed to win */
    int m_winPoints;
    
    /** The remaining time for a round */
    int m_remainingTime;
    
    /** Flag if the game is over */
    bool m_gameOver;
    
    /** Name of the playe who won the game */
    QString m_strWinner;
    
    /** Flag if sound is enabled */
    bool m_soundEnabled;
    
    /** Flag to use wilhelm scream for dying  */
    bool m_wilhelmScream;
    
    KgSound* m_soundPutBomb;
    KgSound* m_soundExplode;
    KgSound* m_soundBonus;
    KgSound* m_soundFalling;
    KgSound* m_soundDie;
    
public:

    /**
    * Creates a new Game instance.
    * @param playerSettings the player settings
    */
    Game(PlayerSettings* playerSettings);

    /**
    * Deletes the Game instance.
    */
    ~Game();

    /**
    * Starts the Game.
    */
    void start();

    /**
    * Pauses the Game.
    * @param p_locked if true the player will be unable to unset the pause.
    */
    void pause(bool p_locked = false);

    /**
    * Pauses / unpauses the game.
    */
    void switchPause();

    /**
    * Enables / disables the sounds.
    * @param p_enabled if true the sounds will be enabled, otherwise they will be disabled
    */
    void setSoundsEnabled(bool p_enabled);
    
    /**
    * @return the Arena instance
    */
    Arena* getArena() const;
    
    /**
    * @return the Player models
    */
    QList<Player*> getPlayers() const;
    
    /**
    * @return the Bonus instance
    */
    QList<Bonus*> getBonus() const;

    /**
    * @return the main timer
    */
    QTimer* getTimer() const;
    
    /**
    * @return the remaining round time
    */
    int getRemainingTime() const;
    
    /**
    * @return true if the Game is paused, false otherwise
    */
    bool isPaused() const;

    /**
    * @return flag if the game is over
    */
    bool getGameOver() const;
    
    /**
    * @return the winner of the game
    */
    QString getWinner() const;
    
     /**
    * @return the points needed to win the game
    */
    int getWinPoints() const;
    
    /**
    * Create the hidden Bonuses
    */
    void createBonus();
    
    /**
    * @param bonus the Bonus to remove
    */
    void removeBonus(Bonus* bonus);
    
    /**
    * remove Block from list and decide to give bonus
    */
    void blockDestroyed(const int row, const int col, Block* block);
    
    /**
    * Removes exploded bombs from the bomb list
    */
    void removeBomb(Bomb* bomb);
    
    /**
    * Sets the games gamescene
    * @param p_gameScene the gamescene
    */
    void setGameScene(GameScene* p_gameScene);
    
private:

    /**
    * Initializes class
    */
    void init();
    /**
    * Cleans class
    */
    void cleanUp();
    
    /**
    * Initializes the character coordinates.
    */
    void initCharactersPosition();

public slots:

    /**
    * Manages the key press events.
    * @param p_event the key press event
    */
    void keyPressEvent(QKeyEvent* p_event);
    
    /**
    * Manages the key release events.
    * @param p_event the key release event
    */
    void keyReleaseEvent(QKeyEvent* p_event);
    
    /**
    * Checks if the round has finished and set it finished.
    */
    void setRoundFinished();
    
    /**
    * Creates a bomb in the Cell with the coordinates x and y or throws the bomb from that possition if the player has the throw bonus
    * @param player the player who dropped the bomb
    * @param x the x-position where to put the bomb
    * @param y the x-position where to put the bomb
    * @param newBomb flag if a new bomb can be created or if only throwing is possible
    * @param throwDistance the throw distance if the player has the throw bonus and the bomb shall be thrown
    */
    void createBomb(Player* player, qreal x, qreal y, bool newBomb, int throwDistance);

private slots:

    /**
    * Updates the Game data.
    */
    void update();
    
    /**
    * Plays the sound of a player falling in a hole.
    */
    void playerFalling();
    
    /**
    * Manages the loss of a life.
    */
    void playerDeath();
    
    /**
    * Resurrects the death players
    */
    void resurrectBonusTaken();
    
    /**
    * Plays the detonation sound
    */
    void bombDetonated();
    
    /**
    * Decrement the remaining round time
    */
    void decrementRemainingRoundTime();
    
signals:

    /**
    * Emitted when the Game is started.
    */
    void gameStarted();
    
    /**
    * Emitted when the Game is over.
    * @param p_unused this parameter must always be true !
    */
    void gameOver(const bool p_unused);
    
    /**
    * Emitted when the pause state has changed.
    * @param p_pause true if the Game is paused, false otherwise
    * @param p_fromUser true if the Game has been paused due to an action the player has done, false otherwise
    */
    void pauseChanged(const bool p_pause, const bool p_fromUser);
    
    /**
    * Emitted when a bomb was created.
    */
    void bombCreated(Bomb* bomb);
    
    /**
    * Emitted when a bomb was removed.
    */
    void bombRemoved(Bomb* bomb);
    
    /**
    * Emitted when something to display has changed.
    */
    void infoChanged(const Granatier::Info::Type p_info);
};

#endif

