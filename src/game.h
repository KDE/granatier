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

#include <QObject>

#ifndef GRANATIER_USE_GLUON
#include <Phonon/MediaObject>
#endif

class GameScene;
class Arena;
class Player;
class Bonus;
class Bomb;
class Block;
class Element;
class PlayerSettings;
class QPointF;
class QTimer;
class QKeyEvent;

#ifdef GRANATIER_USE_GLUON
class KALEngine;
class KALBuffer;
class KALSound;
#endif

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

    /** The points which are needed to win */
    int m_winPoints;
    
    /** Flag if the round is over */
    int m_roundFinished;
    
    /** Flag if the game is over */
    bool m_gameOver;
    
    /** Name of the playe who won the game */
    QString m_strWinner;
    
    /** Flag if sound is enabled */
    bool m_soundEnabled;
    
    /** Flag to use wilhelm scream for dying  */
    bool m_wilhelmScream;
    
    #ifdef GRANATIER_USE_GLUON
    /** User KALEngine for sound */
    KALEngine* soundEngine;
    KALSound* soundPutBomb;
    KALSound* soundExplode;
    KALSound* soundBonus;
    KALBuffer* soundBufferDie;
    KALBuffer* soundBufferWilhelmScream;
    KALSound* soundDie;
    KALSound* soundWilhelmScream;
    #else
    /** Use Phonon for sound */
    QTimer* m_phononPutBombTimer;
    QList <Phonon::MediaObject*> m_phononPutBomb;
    QTimer* m_phononExplodeTimer;
    QList <Phonon::MediaObject*> m_phononExplode;
    QTimer* m_phononBonusTimer;
    QList <Phonon::MediaObject*> m_phononBonus;
    QTimer* m_phononDieTimer;
    QList <Phonon::MediaObject*> m_phononDie;
    #endif
    
public:

    /** The different types of information about the game */
    enum Information
    {
        NoInfo = 0,
        TimeInfo = 1,     // Remaining time
        ArenaInfo = 2,    // The name of the current arena
        PlayerInfo = 4,   // Current level
        AllInfo = TimeInfo | ArenaInfo | PlayerInfo
    };
    /** A flag for the Information enum */
    Q_DECLARE_FLAGS(InformationTypes, Information)

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
    * @param p_locked if true the player will be unable to unset the pause.
    */
    void switchPause(bool p_locked = false);

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
    QList<Bonus*> getBonus();

    /**
    * @return the main timer
    */
    QTimer* getTimer() const;
    
    /**
    * @return true if the Game is paused, false otherwise
    */
    bool isPaused() const;

    /**
    * @return the winner of the game
    */
    QString getWinner() const;
    
    /**
    * Create the hidden Bonuses
    */
    void createBonus();
    
    /**
    * @param bonus the Bonus to remove
    */
    void removeBonus(Bonus* bonus);
    
    /**
    * Create a new Block
    * @param p_position the Block position
    * @param p_imageId the image of the Block
    */
    void createBlock(QPointF p_position, const QString& p_imageId);
    
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
    * Checks if the round has finished.
    */
    void checkRoundFinished();
    
    /**
    * Creates a bomb in the Cell with the coordinates x and y
    */
    void createBomb(Player* player, qreal x, qreal y);

private slots:

    /**
    * Updates the Game data.
    */
    void update();
    
    /**
    * Manages the loss of a life.
    */
    void playerDeath(Player* player);
    
    /**
    * Plays the detonation sound
    */
    void slot_bombDetonated(Bomb* bomb);
    
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
};

#endif

