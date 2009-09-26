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
                
                m_playerSettings.find(strPlayerID).value().keyUp = QKeySequence(granatierConfig.group("Player").group(playersGroupList[i]).readEntry<QString>("KeyUp", ""));
                m_playerSettings.find(strPlayerID).value().keyRight = QKeySequence(granatierConfig.group("Player").group(playersGroupList[i]).readEntry<QString>("KeyRight", ""));
                m_playerSettings.find(strPlayerID).value().keyDown = QKeySequence(granatierConfig.group("Player").group(playersGroupList[i]).readEntry<QString>("KeyDown", ""));
                m_playerSettings.find(strPlayerID).value().keyLeft = QKeySequence(granatierConfig.group("Player").group(playersGroupList[i]).readEntry<QString>("KeyLeft", ""));
                m_playerSettings.find(strPlayerID).value().keyPutBomb = QKeySequence(granatierConfig.group("Player").group(playersGroupList[i]).readEntry<QString>("KeyPutBomb", ""));
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
    
    int nFirstRun = granatierConfig.group("General").readEntry<int>("FirstRun", 1);
    if(nFirstRun)
    {
        settingsIterator = m_playerSettings.begin();
        settingsIterator.value().keyUp = QKeySequence(Qt::Key_Up);
        settingsIterator.value().keyRight = QKeySequence(Qt::Key_Right);
        settingsIterator.value().keyDown = QKeySequence(Qt::Key_Down);
        settingsIterator.value().keyLeft = QKeySequence(Qt::Key_Left);
        settingsIterator.value().keyPutBomb = QKeySequence(Qt::Key_Return);
        ++settingsIterator;
        settingsIterator.value().keyUp = QKeySequence(Qt::Key_W);
        settingsIterator.value().keyRight = QKeySequence(Qt::Key_D);
        settingsIterator.value().keyDown = QKeySequence(Qt::Key_S);
        settingsIterator.value().keyLeft = QKeySequence(Qt::Key_A);
        settingsIterator.value().keyPutBomb = QKeySequence(Qt::Key_Q);
        ++settingsIterator;
        settingsIterator.value().keyUp = QKeySequence(Qt::Key_I);
        settingsIterator.value().keyRight = QKeySequence(Qt::Key_L);
        settingsIterator.value().keyDown = QKeySequence(Qt::Key_K);
        settingsIterator.value().keyLeft = QKeySequence(Qt::Key_J);
        settingsIterator.value().keyPutBomb = QKeySequence(Qt::Key_Space);
        m_tempPlayerSettings = m_playerSettings;
        savePlayerSettings();
        granatierConfig.group("General").writeEntry("FirstRun", 0);
    }
    
    m_tempPlayerSettings = m_playerSettings;
}

PlayerSettings::~PlayerSettings()
{
}

const QStringList PlayerSettings::playerIDs()
{
    return m_playerSettings.keys();
}

const QString PlayerSettings::playerName(const QString& strPlayerID) const
{
    return m_playerSettings.value(strPlayerID).strPlayerName;
}

const QString PlayerSettings::playerFile(const QString& strPlayerID) const
{
    return m_playerSettings.value(strPlayerID).strPlayerFile;
}

const QString PlayerSettings::playerPreviewFile(const QString& strPlayerID) const
{
    return m_playerSettings.value(strPlayerID).strPlayerPreviewFile;
}

bool PlayerSettings::enabled(const QString& strPlayerID) const
{
    return m_playerSettings.value(strPlayerID).enabled;
}

const QKeySequence PlayerSettings::keyUp(const QString& strPlayerID) const
{
    return m_playerSettings.value(strPlayerID).keyUp;
}

const QKeySequence PlayerSettings::keyRight(const QString& strPlayerID) const
{
    return m_playerSettings.value(strPlayerID).keyRight;
}

const QKeySequence PlayerSettings::keyDown(const QString& strPlayerID) const
{
    return m_playerSettings.value(strPlayerID).keyDown;
}

const QKeySequence PlayerSettings::keyLeft(const QString& strPlayerID) const
{
    return m_playerSettings.value(strPlayerID).keyLeft;
}

const QKeySequence PlayerSettings::keyPutBomb(const QString& strPlayerID) const
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
            
            granatierConfig.group("Player").group(QString("%1").arg(nPlayersGroupIndex)).writeEntry("KeyUp", i.value().keyUp.toString());
            granatierConfig.group("Player").group(QString("%1").arg(nPlayersGroupIndex)).writeEntry("KeyRight", i.value().keyRight.toString());
            granatierConfig.group("Player").group(QString("%1").arg(nPlayersGroupIndex)).writeEntry("KeyDown", i.value().keyDown.toString());
            granatierConfig.group("Player").group(QString("%1").arg(nPlayersGroupIndex)).writeEntry("KeyLeft", i.value().keyLeft.toString());
            granatierConfig.group("Player").group(QString("%1").arg(nPlayersGroupIndex)).writeEntry("KeyPutBomb", i.value().keyPutBomb.toString());
            
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
            
            granatierConfig.group("Player").group(QString("%1").arg(nPlayersGroupIndex)).writeEntry("KeyUp", i.value().keyUp.toString());
            granatierConfig.group("Player").group(QString("%1").arg(nPlayersGroupIndex)).writeEntry("KeyRight", i.value().keyRight.toString());
            granatierConfig.group("Player").group(QString("%1").arg(nPlayersGroupIndex)).writeEntry("KeyDown", i.value().keyDown.toString());
            granatierConfig.group("Player").group(QString("%1").arg(nPlayersGroupIndex)).writeEntry("KeyLeft", i.value().keyLeft.toString());
            granatierConfig.group("Player").group(QString("%1").arg(nPlayersGroupIndex)).writeEntry("KeyPutBomb", i.value().keyPutBomb.toString());
            
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

void PlayerSettings::setKeyUp(const QString& strPlayerID, const QKeySequence& key)
{
    if(m_tempPlayerSettings.contains(strPlayerID))
    {
        m_tempPlayerSettings.find(strPlayerID).value().keyUp = key;
    }
}

void PlayerSettings::setKeyRight(const QString& strPlayerID, const QKeySequence& key)
{
    if(m_tempPlayerSettings.contains(strPlayerID))
    {
        m_tempPlayerSettings.find(strPlayerID).value().keyRight = key;
    }
}

void PlayerSettings::setKeyDown(const QString& strPlayerID, const QKeySequence& key)
{
    if(m_tempPlayerSettings.contains(strPlayerID))
    {
        m_tempPlayerSettings.find(strPlayerID).value().keyDown = key;
    }
}

void PlayerSettings::setKeyLeft(const QString& strPlayerID, const QKeySequence& key)
{
    if(m_tempPlayerSettings.contains(strPlayerID))
    {
        m_tempPlayerSettings.find(strPlayerID).value().keyLeft = key;
    }
}

void PlayerSettings::setKeyPutBomb(const QString& strPlayerID, const QKeySequence& key)
{
    if(m_tempPlayerSettings.contains(strPlayerID))
    {
        m_tempPlayerSettings.find(strPlayerID).value().keyPutBomb = key;
    }
}
