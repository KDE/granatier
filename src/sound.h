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
    #include <QtMultimedia/qaudio.h>
    class QAudioOutput;
    class QBuffer;
    class QByteArray;
#endif

class QString;

/**
 * @brief This class the game info if paused or a round is over.
 */
class Sound : public QObject
{

Q_OBJECT

private:
    qint64 m_lastPlayed;
    
    #ifdef GRANATIER_USE_GLUON
        /** User KALEngine for sound */
        KALSound* m_sound;
    #else
        /** Use QtMultimedia for sound */
        QAudioOutput* m_sound;
        QBuffer* m_soundBuffer;
        QByteArray* m_soundData;
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

private slots:
    #ifndef GRANATIER_USE_GLUON
        void finishedPlaying(QAudio::State state);
    #endif
};

#endif

