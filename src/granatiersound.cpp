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

#include "granatiersound.h"

#include <KStandardDirs>
#include <KConfig>
#include <QDateTime>

GranatierSound::GranatierSound (QString strFilePath)
{
    #ifdef GRANATIER_USE_OPENAL
        m_sound = new Tagaro::Sound(strFilePath);
    #else
        m_lastPlayedTime = 0;
        m_nextSource = 1;
        
        m_sound1 = Phonon::createPlayer(Phonon::GameCategory);
        m_sound1->setCurrentSource(strFilePath);
        
        m_sound2 = Phonon::createPlayer(Phonon::GameCategory);
        m_sound2->setCurrentSource(strFilePath);
    #endif
}

GranatierSound::~GranatierSound()
{
    #ifdef GRANATIER_USE_OPENAL
        delete m_sound;
    #else
        delete m_sound1;
        delete m_sound2;
    #endif
}

void GranatierSound::play()
{
    #ifdef GRANATIER_USE_OPENAL
        m_sound->start();
    #else
        QDateTime now = QDateTime::currentDateTime();
        qint64 timeNow = now.toTime_t() * 1000 + now.time().msec();
        
        if(timeNow - m_lastPlayedTime > 20)
        {
            if(m_nextSource == 1)
            {                    
                if(m_sound1->state() == Phonon::StoppedState)
                {
                    m_nextSource = 2;
                    m_sound1->play();
                }
                else
                {
                    m_sound1->stop();
                }
            }
            else
            {
                if(m_sound2->state() == Phonon::StoppedState)
                {
                    m_nextSource = 1;
                    m_sound2->play();
                }
                else
                {
                    m_sound2->stop();
                }
            }
            m_lastPlayedTime = timeNow;
        }
    #endif
}
