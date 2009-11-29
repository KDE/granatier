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

#ifndef PLAYER_H
#define PLAYER_H

#include "character.h"
#include "bonus.h"

class PlayerSettings;
class QKeyEvent;
class QString;
class QTimer;
class Cell;
/**
 * @brief This class represents the main character of the game.
 */
class Player : public Character
{
    Q_OBJECT

private:
    /** The path to the Player image */
    QString m_playerName;
    QString m_graphicsPath;
    
    /** Player asked speed */
    qreal m_askedXSpeed, m_askedYSpeed;
    
    /** the direction the player is moving/looking */
    int m_direction;
    
    /** Player bomb power */
    int m_bombPower;
    
    /** Player bomb total armory */
    int m_maxBombArmory;
    
    /** Player bomb armory */
    int m_bombArmory;
    
    /** Player death flag */
    bool m_death;
    
    /** Points from the player */
    int m_points;
    
    /** flag if the player is on ice */
    bool m_onIce;
    
    /** flag if the player falls into a hole */
    bool m_falling;
    
    /** flag if the player player has the throw bomb bonus */
    bool m_throwBomb;
    
    /** flag if the player player has the kick bomb bonus */
    bool m_kickBomb;
    
    /** a list with the gathered shields from the shield bonus */
    QList <int> m_listShield;
    
    /** the bad bonus type */
    Bonus::BonusType m_badBonusType;
    
    /** the speed before a bad bonus was taken */
    qreal m_normalSpeed;
    
    /** the bomb armory before a bad bonus was taken */
    int m_normalBombArmory;
    
    /** flag if the mirror bad bunus was taken */
    bool m_moveMirrored;
    
    /** timer for the bad bonus to disapear */
    QTimer* m_badBonusCountdownTimer;
    /** elapsed milliseconds since the last bad bonus */
    int m_badBonusMillisecondsElapsed;
    
    

public:
    /**
      * Creates a new Player instance.
      * @param p_x the initial x-coordinate
      * @param p_y the initial y-coordinate
      * @param p_playerID the Player ID for PlayerSettings
      * @param p_playerSettings the PlayerSettings
      * @param p_arena the Arena the Player is on
      */
    Player(qreal p_x, qreal p_y, const QString& p_playerID, const PlayerSettings* p_playerSettings, Arena* p_arena);
    
    /**
      * Deletes the Player instance.
      */
    ~Player();

    /**
      * Shortcuts for moving and dropping bombs
      */
    void setShortcuts(const Shortcuts &keys);

    /**
      * Gets the path to the Player SVG.
      * @return the path to the Player SVG
      */
    QString getGraphicsPath() const;
    
    /**
      * Gets the path to the Player image.
      * @return the path to the Player image
      */
    QString getPlayerName() const;

    /**
      * Initializes the Player.
      */
    void init();

    /**
      * Makes the Player ask to go up
      */
    void goUp();

    /**
      * Makes the Player ask to go down
      */
    void goDown();

    /**
      * Makes the Player ask to go to the right
      */
    void goRight();

    /**
      * Makes the Player ask to go to the left
      */
    void goLeft();

    /**
      * Moves the Player function of its current coordinates and speed.
      */
    void move(qreal x, qreal y);
    
    /**
      * Updates the Player move
      */
    void updateMove();

    /**
      * @return the asked x speed value
      */
    qreal getAskedXSpeed() const;

    /**
      * @return the asked y speed value
      */
    qreal getAskedYSpeed() const;
    
    /**
      * @return the direction the player is moving/looking
      */
    int direction();

    /**
      * @return the bomb power
      */
    int getBombPower() const;
    
    /**
      * decrements the bomb armory
      */
    void decrementBombArmory();
    
    /**
      * Manages the points won
      * @param p_element reference to the element eaten
      */
    void addBonus(Bonus* p_bonus);
    
    /**
      * Manages the points won
      * @param n_ExplosionID the ID from the explosion which hit the player
      * @return true if there is an unused shield, false otherwise
      */
    bool shield(int n_ExplosionID);
    
    /**
      * @return true if the player has the shield bonus
      */
    bool hasShield();
    
    /**
      * @return true if the player has the throw bonus
      */
    bool hasThrowBomb();
    
    /**
      * @return true if the player has the kick bonus
      */
    bool hasKickBomb();
    
    /**
      * @return true if the player has a bad bonus
      */
    bool hasBadBonus();
    
    /**
      * Implements the Character function
      */
    void die();
    
    /**
      *  returns if the player is alive
      */
    bool isAlive() const;
    
    /**
      * resurrects the player
      */
    void resurrect();
    
    /**
      * returns the points
      */
    int points() const;
    
    /**
      * adds a point
      */
    void addPoint();
    
    /**
      * Emits a signal to PlayerItem in order to manage collisions
      */
    void emitGameUpdated();
    
    /**
      * Manages the keys for moving and dropping bombs.
      */
    void keyPressed(QKeyEvent* keyEvent);
    
    /**
      * Manages the keys for moving and dropping bombs.
      */
    void keyReleased(QKeyEvent* keyEvent);
    
    /**
      * Returns the sign of a value with a positive sign for zero
      */
      //TODO: find a better place
    int signZeroPositive(const qreal value);
    /**
      * Returns the sign of a value with 0 for zero
      */
    int sign(const qreal value);
        
private:
    /**
      * Updates the Player direction with the asked direction
      */
    void updateDirection();

    /**
      * @return the next cell the player will move on with its asked direction
      */
    Cell getAskedNextCell();

    /**
      * Stops moving the Player
      */
    void stopMoving();

public slots:
    /**
     * refills the bomb armory after a bomb is exploded
     */
    void slot_refillBombArmory();
    
private slots:
    /**
     * removes the bad bonus from the player
     */
    void slot_removeBadBonus();
    
    /**
     * emits the signal with the elapsed bad bonus time for the infosidebar
     */
    void slot_badBonusTimerTimeout();

signals:
    /**
      * Emitted when the direction changed
      */
    void directionChanged();

    /**
      * Signals to PlayerItem that the game has been updated
      */
      void gameUpdated();

      /**
      * Emitted when the player stops moving
      */
      void stopped();
      
      /**
      * Emitted when the player drops a bomb
      */
      void bombDropped(Player* player, qreal x, qreal y, int throwDistance);
      
      /**
      * Emitted when the player is dying
      */
      void dying(Player* player);
      
      /**
      * Emitted when the player is falling in a hole
      */
      void falling();
      
      /**
      * Emitted when the player has taken the resurrect bonus
      */
      void resurrectBonusTaken();
      
       /**
      * Emitted when the player has taken a bonus
      * @param player the player which info changed
      * @param bonusType the bonus that was taken
      * @param percentageElapsed the bad bonus time that has elapsed
      */
      void bonusUpdated(Player* player, Bonus::BonusType bonusType, int percentageElapsed);
};

#endif
