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

#include "sound.h"

#include <KStandardDirs>
#include <KConfig>
#include <QDateTime>

#ifdef GRANATIER_USE_GLUON
    #include <KDE/KALEngine>
    #include <KDE/KALSound>
#endif

Sound::Sound (QString strFilePath)
{
    #ifdef GRANATIER_USE_GLUON
        KALEngine::instance();//TODO: use KALEngine::instance(Phonon::GameCategory) when it works
        m_sound = new KALSound;
        m_sound->load(strFilePath);
    #else
        m_lastPlayedTime = 0;
        m_nextSource = 1;
        
        m_sound1 = Phonon::createPlayer(Phonon::GameCategory);
        m_sound1->setCurrentSource(strFilePath);
        
        m_sound2 = Phonon::createPlayer(Phonon::GameCategory);
        m_sound2->setCurrentSource(strFilePath);
    #endif
}

Sound::~Sound()
{
    #ifdef GRANATIER_USE_GLUON
        delete m_sound;
    #else
        delete m_sound1;
        delete m_sound2;
    #endif
}

void Sound::play()
{
    #ifdef GRANATIER_USE_GLUON
        m_sound->play();
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
                    m_sound1->stop();
                }
                else
                {
                    m_nextSource = 1;
                    m_sound1->stop();
                }
            }
            m_lastPlayedTime = timeNow;
        }
    #endif
}
