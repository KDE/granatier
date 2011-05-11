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

#ifndef GRANATIER_SOUND_H
#define GRANATIER_SOUND_H

#include <TagaroAudio/Sound>

class QString;

/**
 * @brief This class the game info if paused or a round is over.
 */
class GranatierSound
{

private:
    /** User TagaroAudio for sound */
    Tagaro::Sound* m_sound;

public:

    /**
      * Creates a new Sound instance.
      * @param strFilePath the path to the sound file
      */
    GranatierSound (QString strFilePath);

    /**
      * Deletes the InfoOverlay instance.
      */
    ~GranatierSound();
    
    /**
      * Plays the sound file.
      */
    void play();
};

#endif

