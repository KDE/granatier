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
#else
#include <QFile>
#include <QBuffer>
#include <QtMultimedia/QAudioDeviceInfo>
#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/qaudio.h>
#endif

Sound::Sound (QString strFilePath)
{
    #ifdef GRANATIER_USE_GLUON
        KALEngine::instance();//TODO: use KALEngine::instance(Phonon::GameCategory) when it works
        m_sound = new KALSound;
        m_sound->load(strFilePath);
    #else
        QFile soundFile;
        soundFile.setFileName(strFilePath);
        soundFile.open(QIODevice::ReadOnly);
        
        m_soundData = new QByteArray(soundFile.readAll());
        soundFile.close();
        
        m_soundBuffer = new QBuffer(m_soundData, this);
        m_soundBuffer->open(QIODevice::ReadOnly);
        
        QAudioFormat format;
        // Set up the format, eg.
        format.setFrequency(44100);
        format.setChannels(1);
        format.setSampleSize(8);
        format.setCodec("audio/pcm");
        format.setByteOrder(QAudioFormat::LittleEndian);
        format.setSampleType(QAudioFormat::UnSignedInt);
        
        QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
        if (!info.isFormatSupported(format))
        {
            format = info.nearestFormat(format);
        }

        m_sound = new QAudioOutput(QAudioDeviceInfo::defaultOutputDevice(), format, this);
        connect(m_sound,SIGNAL(stateChanged(QAudio::State)),SLOT(finishedPlaying(QAudio::State)));
    #endif
    
    m_lastPlayed = 0;
}

Sound::~Sound()
{
    #ifdef GRANATIER_USE_GLUON
        delete m_sound;
    #else
        m_sound->stop();
        m_soundBuffer->close();
        delete m_soundBuffer;
        delete m_soundData;
        delete m_sound;
    #endif
}

void Sound::play()
{
    #ifdef GRANATIER_USE_GLUON
        m_sound->play();
    #else
        QDateTime now = QDateTime::currentDateTime();
        qint64 timeNow = now.toTime_t() * 1000 + now.time().msec();
        
        if(m_sound->state() == QAudio::ActiveState)
        {
            if(timeNow - m_lastPlayed > 20)
            {
                m_soundBuffer->reset();
                m_sound->start(m_soundBuffer);
                m_lastPlayed = timeNow;
            }
        }
        else
        {
            m_sound->start(m_soundBuffer);
            m_lastPlayed = timeNow;
        }
    #endif
}

#ifndef GRANATIER_USE_GLUON
void Sound::finishedPlaying(QAudio::State state)
{
    if(state == QAudio::IdleState)
    {
        m_sound->stop();
        m_soundBuffer->reset();
    }
}
#endif
