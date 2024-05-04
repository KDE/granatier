/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "playersettings.h"
#include "settings.h"

#include <KConfig>
#include <QDir>
#include <QStandardPaths>

PlayerSettings::PlayerSettings()
{
    QStringList playersAvailable;
    QStringList playerFilesAvailable;
    QStringList playerNamesAvailable;

    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::AppDataLocation, QStringLiteral("players"), QStandardPaths::LocateDirectory);
    for(const auto& dir: dirs) {
         const QStringList fileNames = QDir(dir).entryList({QStringLiteral("*.desktop")});
         for(const auto& file: fileNames) {
                playersAvailable.append(file);
         }
    }
    
    playersAvailable.removeDuplicates();
    playersAvailable.sort();
    for(const auto& player: playersAvailable)
    {
        StructPlayerSettings settings;
        settings.strPlayerID = player;

        KConfig desktopFile(QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("players/") + settings.strPlayerID), KConfig::SimpleConfig);

        settings.strPlayerDesktopFilePath = desktopFile.name();
        settings.strPlayerName = desktopFile.group(QStringLiteral("KGameTheme")).readEntry("Name", QString());
        settings.strPlayerGraphicsFile = desktopFile.group(QStringLiteral("KGameTheme")).readEntry("FileName", QString());
        settings.enabled = false;
        
        m_playerSettings.insert(settings.strPlayerID, settings);
    }
    
    
    QStringList playersGroupList;
    QString strPlayerID;
    KConfig granatierConfig(QStringLiteral("granatierrc"), KConfig::NoGlobals );
    int nEnableCount = 0;
    if(granatierConfig.hasGroup(QStringLiteral("Player")))
    {
        playersGroupList = granatierConfig.group(QStringLiteral("Player")).groupList();
        playersGroupList.sort();
        for(const auto& player: playersGroupList)
        {
            strPlayerID = granatierConfig.group(QStringLiteral("Player")).group(player).readEntry("PlayerID", QString());
            if(m_playerSettings.contains(strPlayerID))
            {
                m_playerSettings.find(strPlayerID).value().strPlayerName = granatierConfig.group(QStringLiteral("Player")).group(player).readEntry<QString>("Name", QStringLiteral(""));
                m_playerSettings.find(strPlayerID).value().enabled = granatierConfig.group(QStringLiteral("Player")).group(player).readEntry<int>("Enabled", 0);
                if(m_playerSettings.find(strPlayerID).value().enabled)
                {
                    nEnableCount++;
                }
                
                m_playerSettings.find(strPlayerID).value().keyUp = QKeySequence(granatierConfig.group(QStringLiteral("Player")).group(player).readEntry<QString>("KeyUp", QStringLiteral("")));
                m_playerSettings.find(strPlayerID).value().keyRight = QKeySequence(granatierConfig.group(QStringLiteral("Player")).group(player).readEntry<QString>("KeyRight", QStringLiteral("")));
                m_playerSettings.find(strPlayerID).value().keyDown = QKeySequence(granatierConfig.group(QStringLiteral("Player")).group(player).readEntry<QString>("KeyDown", QStringLiteral("")));
                m_playerSettings.find(strPlayerID).value().keyLeft = QKeySequence(granatierConfig.group(QStringLiteral("Player")).group(player).readEntry<QString>("KeyLeft", QStringLiteral("")));
                m_playerSettings.find(strPlayerID).value().keyPutBomb = QKeySequence(granatierConfig.group(QStringLiteral("Player")).group(player).readEntry<QString>("KeyPutBomb", QStringLiteral("")));
            }
        }
    }
    
    auto settingsIterator = m_playerSettings.begin();
    while (nEnableCount < 2 && settingsIterator != m_playerSettings.end())
    {
        if(!(*settingsIterator).enabled)
        {
            (*settingsIterator).enabled = true;
            nEnableCount++;
        }
        ++settingsIterator;
    }
    
    int nFirstRun = granatierConfig.group(QStringLiteral("General")).readEntry<int>("FirstRun", 1);
    if(nFirstRun)
    {
        settingsIterator = m_playerSettings.begin();
        (*settingsIterator).keyUp = QKeySequence(Qt::Key_Up);
        (*settingsIterator).keyRight = QKeySequence(Qt::Key_Right);
        (*settingsIterator).keyDown = QKeySequence(Qt::Key_Down);
        (*settingsIterator).keyLeft = QKeySequence(Qt::Key_Left);
        (*settingsIterator).keyPutBomb = QKeySequence(Qt::Key_Return);
        ++settingsIterator;
        (*settingsIterator).keyUp = QKeySequence(Qt::Key_W);
        (*settingsIterator).keyRight = QKeySequence(Qt::Key_D);
        (*settingsIterator).keyDown = QKeySequence(Qt::Key_S);
        (*settingsIterator).keyLeft = QKeySequence(Qt::Key_A);
        (*settingsIterator).keyPutBomb = QKeySequence(Qt::Key_Q);
        ++settingsIterator;
        (*settingsIterator).keyUp = QKeySequence(Qt::Key_I);
        (*settingsIterator).keyRight = QKeySequence(Qt::Key_L);
        (*settingsIterator).keyDown = QKeySequence(Qt::Key_K);
        (*settingsIterator).keyLeft = QKeySequence(Qt::Key_J);
        (*settingsIterator).keyPutBomb = QKeySequence(Qt::Key_Space);
        m_pendingPlayerSettings = m_playerSettings;
        savePlayerSettings();
        granatierConfig.group(QStringLiteral("General")).writeEntry("FirstRun", 0);
    }
    
    m_pendingPlayerSettings = m_playerSettings;
}

PlayerSettings::~PlayerSettings()
= default;

const QStringList PlayerSettings::playerIDs() const
{
    return m_playerSettings.keys();
}

const QString PlayerSettings::playerName(const QString& strPlayerID) const
{
    return m_playerSettings.value(strPlayerID).strPlayerName;
}

const QString PlayerSettings::playerDesktopFilePath(const QString& strPlayerID) const
{
  return m_playerSettings.value(strPlayerID).strPlayerDesktopFilePath;
}

const QString PlayerSettings::playerGraphicsFile(const QString& strPlayerID) const
{
    return m_playerSettings.value(strPlayerID).strPlayerGraphicsFile;
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
    m_playerSettings = m_pendingPlayerSettings;
    
    QStringList playersGroupList;
    QStringList strPlayerIDList;
    KConfig granatierConfig(QStringLiteral("granatierrc"), KConfig::NoGlobals );
    if(granatierConfig.hasGroup(QStringLiteral("Player")))
    {
        playersGroupList = granatierConfig.group(QStringLiteral("Player")).groupList();
        playersGroupList.sort();
        for(const auto& player: std::as_const(playersGroupList))
        {
            strPlayerIDList.append(granatierConfig.group(QStringLiteral("Player")).group(player).readEntry<QString>("PlayerID", QStringLiteral("")));
        }
        
        int nPlayersGroupIndex;
        int nIndex;
        auto player = m_playerSettings.constBegin();
        while ( player != m_playerSettings.constEnd())
        {
            playersGroupList.sort();
            nIndex = strPlayerIDList.indexOf( player.key());
            if(nIndex < 0)
            {
                for(int j = 0; j < playersGroupList.count(); j++)
                {
                    if(playersGroupList[j].toInt() > j+1)
                    {
                        playersGroupList.append(QStringLiteral("%1").arg(j+1));
                        strPlayerIDList.append( player.key());
                        nPlayersGroupIndex = j+1;
                        nIndex = nPlayersGroupIndex;
                        break;
                    }
                }
                if(nIndex < 0)
                {
                    playersGroupList.append(QStringLiteral("%1").arg(playersGroupList.count() + 1));
                    strPlayerIDList.append( player.key());
                    nPlayersGroupIndex = playersGroupList.count();
                }
            }
            else
            {
                nPlayersGroupIndex = playersGroupList[nIndex].toInt();
            }
            
            granatierConfig.group(QStringLiteral("Player")).group(QStringLiteral("%1").arg(nPlayersGroupIndex)).writeEntry("PlayerID", (*player).strPlayerID);
            granatierConfig.group(QStringLiteral("Player")).group(QStringLiteral("%1").arg(nPlayersGroupIndex)).writeEntry("Name", (*player).strPlayerName);
            granatierConfig.group(QStringLiteral("Player")).group(QStringLiteral("%1").arg(nPlayersGroupIndex)).writeEntry("Enabled", ((*player).enabled ? 1 : 0));
            
            granatierConfig.group(QStringLiteral("Player")).group(QStringLiteral("%1").arg(nPlayersGroupIndex)).writeEntry("KeyUp", (*player).keyUp.toString());
            granatierConfig.group(QStringLiteral("Player")).group(QStringLiteral("%1").arg(nPlayersGroupIndex)).writeEntry("KeyRight", (*player).keyRight.toString());
            granatierConfig.group(QStringLiteral("Player")).group(QStringLiteral("%1").arg(nPlayersGroupIndex)).writeEntry("KeyDown", (*player).keyDown.toString());
            granatierConfig.group(QStringLiteral("Player")).group(QStringLiteral("%1").arg(nPlayersGroupIndex)).writeEntry("KeyLeft", (*player).keyLeft.toString());
            granatierConfig.group(QStringLiteral("Player")).group(QStringLiteral("%1").arg(nPlayersGroupIndex)).writeEntry("KeyPutBomb", (*player).keyPutBomb.toString());
            
            ++player;
        }
    }
    else
    {
        int nPlayersGroupIndex = 1;
        for (const auto& player: m_playerSettings)
        {
            granatierConfig.group(QStringLiteral("Player")).group(QStringLiteral("%1").arg(nPlayersGroupIndex)).writeEntry("PlayerID", player.strPlayerID);
            granatierConfig.group(QStringLiteral("Player")).group(QStringLiteral("%1").arg(nPlayersGroupIndex)).writeEntry("Name", player.strPlayerName);
            granatierConfig.group(QStringLiteral("Player")).group(QStringLiteral("%1").arg(nPlayersGroupIndex)).writeEntry("Enabled", ( player.enabled ? 1 : 0));
            
            granatierConfig.group(QStringLiteral("Player")).group(QStringLiteral("%1").arg(nPlayersGroupIndex)).writeEntry("KeyUp", player.keyUp.toString());
            granatierConfig.group(QStringLiteral("Player")).group(QStringLiteral("%1").arg(nPlayersGroupIndex)).writeEntry("KeyRight", player.keyRight.toString());
            granatierConfig.group(QStringLiteral("Player")).group(QStringLiteral("%1").arg(nPlayersGroupIndex)).writeEntry("KeyDown", player.keyDown.toString());
            granatierConfig.group(QStringLiteral("Player")).group(QStringLiteral("%1").arg(nPlayersGroupIndex)).writeEntry("KeyLeft", player.keyLeft.toString());
            granatierConfig.group(QStringLiteral("Player")).group(QStringLiteral("%1").arg(nPlayersGroupIndex)).writeEntry("KeyPutBomb", player.keyPutBomb.toString());
            
            nPlayersGroupIndex++;
        }
    }
}

void PlayerSettings::discardUnsavedSettings()
{
    m_pendingPlayerSettings = m_playerSettings;
}

void PlayerSettings::setPlayerName(const QString& strPlayerID, const QString& strName)
{
    if( m_pendingPlayerSettings.contains(strPlayerID))
    {
        m_pendingPlayerSettings.find(strPlayerID).value().strPlayerName = strName;
        Settings::self()->setDummy(Settings::self()->dummy() + 3);
    }
}

void PlayerSettings::setEnabled(const QString& strPlayerID, const bool enabled)
{
    if( m_pendingPlayerSettings.contains(strPlayerID))
    {
        m_pendingPlayerSettings.find(strPlayerID).value().enabled = enabled;
        Settings::self()->setDummy(Settings::self()->dummy() + 3);
    }
}

void PlayerSettings::setKeyUp(const QString& strPlayerID, const QKeySequence& key)
{
    if( m_pendingPlayerSettings.contains(strPlayerID))
    {
        m_pendingPlayerSettings.find(strPlayerID).value().keyUp = key;
        Settings::self()->setDummy(Settings::self()->dummy() + 3);
    }
}

void PlayerSettings::setKeyRight(const QString& strPlayerID, const QKeySequence& key)
{
    if( m_pendingPlayerSettings.contains(strPlayerID))
    {
        m_pendingPlayerSettings.find(strPlayerID).value().keyRight = key;
        Settings::self()->setDummy(Settings::self()->dummy() + 3);
    }
}

void PlayerSettings::setKeyDown(const QString& strPlayerID, const QKeySequence& key)
{
    if( m_pendingPlayerSettings.contains(strPlayerID))
    {
        m_pendingPlayerSettings.find(strPlayerID).value().keyDown = key;
        Settings::self()->setDummy(Settings::self()->dummy() + 3);
    }
}

void PlayerSettings::setKeyLeft(const QString& strPlayerID, const QKeySequence& key)
{
    if( m_pendingPlayerSettings.contains(strPlayerID))
    {
        m_pendingPlayerSettings.find(strPlayerID).value().keyLeft = key;
        Settings::self()->setDummy(Settings::self()->dummy() + 3);
    }
}

void PlayerSettings::setKeyPutBomb(const QString& strPlayerID, const QKeySequence& key)
{
    if( m_pendingPlayerSettings.contains(strPlayerID))
    {
        m_pendingPlayerSettings.find(strPlayerID).value().keyPutBomb = key;
        Settings::self()->setDummy(Settings::self()->dummy() + 3);
    }
}
