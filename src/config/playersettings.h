/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PLAYERSETTINGS_H
#define PLAYERSETTINGS_H

#include <QKeySequence>
#include <QString>
#include <QMap>
#include <QStringList>

class PlayerSettings
{
public:
    PlayerSettings();
    ~PlayerSettings();
    
    const QStringList playerIDs() const;
    const QString playerName(const QString& strPlayerID) const;
    const QByteArray playerThemeId(const QString& strPlayerID) const;
    const QString playerDesktopFilePath(const QString& strPlayerID) const;
    const QString playerGraphicsFile(const QString& strPlayerID) const;
    bool enabled(const QString& strPlayerID) const;
    const QKeySequence keyUp(const QString& strPlayerID) const;
    const QKeySequence keyRight(const QString& strPlayerID) const;
    const QKeySequence keyDown(const QString& strPlayerID) const;
    const QKeySequence keyLeft(const QString& strPlayerID) const;
    const QKeySequence keyPutBomb(const QString& strPlayerID) const;
    
    void savePlayerSettings();
    void discardUnsavedSettings();
    void setPlayerName(const QString& strPlayerID, const QString& strName);
    void setEnabled(const QString& strPlayerID, const bool enabled);
    void setKeyUp(const QString& strPlayerID, const QKeySequence& key);
    void setKeyRight(const QString& strPlayerID, const QKeySequence& key);
    void setKeyDown(const QString& strPlayerID, const QKeySequence& key);
    void setKeyLeft(const QString& strPlayerID, const QKeySequence& key);
    void setKeyPutBomb(const QString& strPlayerID, const QKeySequence& key);
    
private:
    struct StructPlayerSettings
    {
        QString strPlayerID;
        QString strPlayerName;
        QByteArray playerThemeId;
        QString strPlayerDesktopFilePath;
        QString strPlayerGraphicsFile;
        bool enabled;
        QKeySequence keyUp;
        QKeySequence keyRight;
        QKeySequence keyDown;
        QKeySequence keyLeft;
        QKeySequence keyPutBomb;
    };
    
    QMap <QString, StructPlayerSettings> m_playerSettings;
    QMap <QString, StructPlayerSettings> m_pendingPlayerSettings;
}; 

#endif
