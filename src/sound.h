/*
 * Copyright 2009 Mathias Kraus <k.hias@gmx.de>
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

#ifndef SOUND_H
#define SOUND_H

#include <QObject>

#ifdef GRANATIER_USE_GLUON
    class KALSound;
#else
    #include <Phonon/MediaObject>
#endif

class QString;

/**
 * @brief This class the game info if paused or a round is over.
 */
class Sound : public QObject
{

Q_OBJECT

private:
    #ifdef GRANATIER_USE_GLUON
        /** User KALEngine for sound */
        KALSound* m_sound;
    #else
        /** Use Phonon for sound */
        qint64 m_lastPlayedTime;
        int m_nextSource;
        Phonon::MediaObject* m_sound1;
        Phonon::MediaObject* m_sound2;
    #endif

public:

    /**
      * Creates a new Sound instance.
      * @param strFilePath the path to the sound file
      */
    Sound (QString strFilePath);

    /**
      * Deletes the InfoOverlay instance.
      */
    ~Sound();
    
    /**
      * Plays the sound file.
      */
    void play();
};

#endif

