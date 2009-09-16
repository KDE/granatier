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

#include "playersettings.h"

#include <QtCore>

#include <KStandardDirs>
#include <KConfig>
#include <KConfigGroup>

PlayerSettings::PlayerSettings()
{
    QStringList playersAvailable;
    QStringList playerFilesAvailable;
    QStringList playerNamesAvailable;
    KGlobal::dirs()->addResourceType("players", "data", "granatier/players/");
    KGlobal::dirs()->findAllResources("players", "*.desktop", KStandardDirs::Recursive, playersAvailable);
    
    playersAvailable.removeDuplicates();
    playersAvailable.sort();
    for(int i = 0; i < playersAvailable.count(); i++)
    {
        StructPlayerSettings settings;
        settings.strPlayerID = playersAvailable[i];
        
        KConfig desktopFile(KStandardDirs::locate("appdata", "players/" + settings.strPlayerID), KConfig::SimpleConfig);
        
        settings.strPlayerName = desktopFile.group("Player").readEntry<QString>("Name", "");
        settings.strPlayerFile = desktopFile.group("Player").readEntry<QString>("FileName", "");
        settings.strPlayerPreviewFile = desktopFile.group("Player").readEntry<QString>("Preview", "");
        settings.enabled = false;
        
        m_playerSettings.insert(settings.strPlayerID, settings);
    }
    
    
    QStringList playersGroupList;
    QString strPlayerID;
    KConfig granatierConfig("granatierrc", KConfig::NoGlobals );
    int nEnableCount = 0;
    if(granatierConfig.hasGroup("Player"))
    {
        playersGroupList = granatierConfig.group("Player").groupList();
        playersGroupList.sort();
        for(int i = 0; i < playersGroupList.count(); i++)
        {
            strPlayerID = granatierConfig.group("Player").group(playersGroupList[i]).readEntry<QString>("PlayerID", "");
            if(m_playerSettings.contains(strPlayerID))
            {
                m_playerSettings.find(strPlayerID).value().strPlayerName = granatierConfig.group("Player").group(playersGroupList[i]).readEntry<QString>("Name", "");
                m_playerSettings.find(strPlayerID).value().enabled = granatierConfig.group("Player").group(playersGroupList[i]).readEntry<int>("Enabled", 0);
                if(m_playerSettings.find(strPlayerID).value().enabled)
                {
                    nEnableCount++;
                }
            }
        }
    }
    
    QMap<QString, StructPlayerSettings>::iterator settingsIterator = m_playerSettings.begin();
    while (nEnableCount < 2 && settingsIterator != m_playerSettings.end())
    {
        if(!(settingsIterator.value().enabled))
        {
            settingsIterator.value().enabled = true;
            nEnableCount++;
        }
        ++settingsIterator;
    }
    
    m_tempPlayerSettings = m_playerSettings;
}

PlayerSettings::~PlayerSettings()
{
}

QStringList PlayerSettings::playerIDs()
{
    return m_playerSettings.keys();
}

QString PlayerSettings::playerName(const QString& strPlayerID) const
{
    return m_playerSettings.value(strPlayerID).strPlayerName;
}

QString PlayerSettings::playerFile(const QString& strPlayerID) const
{
    return m_playerSettings.value(strPlayerID).strPlayerFile;
}

QString PlayerSettings::playerPreviewFile(const QString& strPlayerID) const
{
    return m_playerSettings.value(strPlayerID).strPlayerPreviewFile;
}

bool PlayerSettings::enabled(const QString& strPlayerID) const
{
    return m_playerSettings.value(strPlayerID).enabled;
}

Qt::Key PlayerSettings::keyUp(const QString& strPlayerID) const
{
    return m_playerSettings.value(strPlayerID).keyUp;
}

Qt::Key PlayerSettings::keyRight(const QString& strPlayerID) const
{
    return m_playerSettings.value(strPlayerID).keyRight;
}

Qt::Key PlayerSettings::keyDown(const QString& strPlayerID) const
{
    return m_playerSettings.value(strPlayerID).keyDown;
}

Qt::Key PlayerSettings::keyLeft(const QString& strPlayerID) const
{
    return m_playerSettings.value(strPlayerID).keyLeft;
}

Qt::Key PlayerSettings::keyPutBomb(const QString& strPlayerID) const
{
    return m_playerSettings.value(strPlayerID).keyPutBomb;
}

void PlayerSettings::savePlayerSettings()
{
    m_playerSettings = m_tempPlayerSettings;
    
    QStringList playersGroupList;
    QStringList strPlayerIDList;
    KConfig granatierConfig("granatierrc", KConfig::NoGlobals );
    if(granatierConfig.hasGroup("Player"))
    {
        playersGroupList = granatierConfig.group("Player").groupList();
        playersGroupList.sort();
        for(int i = 0; i < playersGroupList.count(); i++)
        {
            strPlayerIDList.append(granatierConfig.group("Player").group(playersGroupList[i]).readEntry<QString>("PlayerID", ""));
        }
        
        int nPlayersGroupIndex;
        QMap<QString, StructPlayerSettings>::const_iterator i = m_playerSettings.constBegin();
        while (i != m_playerSettings.constEnd())
        {
            nPlayersGroupIndex = strPlayerIDList.indexOf(i.key());
            if(nPlayersGroupIndex < 0)
            {
                for(int j = 0; j < playersGroupList.count(); j++)
                {
                    if(playersGroupList[j].toInt() > j+1)
                    {
                        playersGroupList.append(QString("%1").arg(j+1));
                        strPlayerIDList.append(i.key());
                        nPlayersGroupIndex = j;
                    }
                }
                if(nPlayersGroupIndex < 0)
                {
                    playersGroupList.append(QString("%1").arg(playersGroupList.count()));
                    strPlayerIDList.append(i.key());
                    nPlayersGroupIndex = playersGroupList.count();
                }
            }
            
            nPlayersGroupIndex += 1;
            
            granatierConfig.group("Player").group(QString("%1").arg(nPlayersGroupIndex)).writeEntry("PlayerID", i.value().strPlayerID);
            granatierConfig.group("Player").group(QString("%1").arg(nPlayersGroupIndex)).writeEntry("Name", i.value().strPlayerName);
            granatierConfig.group("Player").group(QString("%1").arg(nPlayersGroupIndex)).writeEntry("Enabled", (i.value().enabled ? 1 : 0));
            
            ++i;
        }
    }
    else
    {
        int nPlayersGroupIndex = 1;
        QMap<QString, StructPlayerSettings>::const_iterator i = m_playerSettings.constBegin();
        while (i != m_playerSettings.constEnd())
        {
            granatierConfig.group("Player").group(QString("%1").arg(nPlayersGroupIndex)).writeEntry("PlayerID", i.value().strPlayerID);
            granatierConfig.group("Player").group(QString("%1").arg(nPlayersGroupIndex)).writeEntry("Name", i.value().strPlayerName);
            granatierConfig.group("Player").group(QString("%1").arg(nPlayersGroupIndex)).writeEntry("Enabled", (i.value().enabled ? 1 : 0));
            
            nPlayersGroupIndex++;
            ++i;
        }
    }
}

void PlayerSettings::discardUnsavedSettings()
{
    m_tempPlayerSettings = m_playerSettings;
}

void PlayerSettings::setPlayerName(const QString& strPlayerID, const QString& strName)
{
    if(m_tempPlayerSettings.contains(strPlayerID))
    {
        m_tempPlayerSettings.find(strPlayerID).value().strPlayerName = strName;
    }
}

void PlayerSettings::setEnabled(const QString& strPlayerID, const bool enabled)
{
    if(m_tempPlayerSettings.contains(strPlayerID))
    {
        m_tempPlayerSettings.find(strPlayerID).value().enabled = enabled;
    }
}

void PlayerSettings::setKeyUp(const QString& strPlayerID, const Qt::Key key)
{
    if(m_tempPlayerSettings.contains(strPlayerID))
    {
        m_tempPlayerSettings.find(strPlayerID).value().keyUp = key;
    }
}

void PlayerSettings::setKeyRight(const QString& strPlayerID, const Qt::Key key)
{
    if(m_tempPlayerSettings.contains(strPlayerID))
    {
        m_tempPlayerSettings.find(strPlayerID).value().keyRight = key;
    }
}

void PlayerSettings::setKeyDown(const QString& strPlayerID, const Qt::Key key)
{
    if(m_tempPlayerSettings.contains(strPlayerID))
    {
        m_tempPlayerSettings.find(strPlayerID).value().keyDown = key;
    }
}

void PlayerSettings::setKeyLeft(const QString& strPlayerID, const Qt::Key key)
{
    if(m_tempPlayerSettings.contains(strPlayerID))
    {
        m_tempPlayerSettings.find(strPlayerID).value().keyLeft = key;
    }
}

void PlayerSettings::setKeyPutBomb(const QString& strPlayerID, const Qt::Key key)
{
    if(m_tempPlayerSettings.contains(strPlayerID))
    {
        m_tempPlayerSettings.find(strPlayerID).value().keyPutBomb = key;
    }
}
