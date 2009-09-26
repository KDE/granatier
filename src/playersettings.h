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

#ifndef PLAYERSETTINGS_H
#define PLAYERSETTINGS_H

#include <QKeySequence>
#include <QString>
#include <QMap>

class QStringList;
class QKeySequence;

class PlayerSettings
{
public:
    PlayerSettings();
    ~PlayerSettings();
    
    const QStringList playerIDs();
    const QString playerName(const QString& strPlayerID) const;
    const QString playerFile(const QString& strPlayerID) const;
    const QString playerPreviewFile(const QString& strPlayerID) const;
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
        QString strPlayerFile;
        QString strPlayerPreviewFile;
        bool enabled;
        QKeySequence keyUp;
        QKeySequence keyRight;
        QKeySequence keyDown;
        QKeySequence keyLeft;
        QKeySequence keyPutBomb;
    };
    
    QMap <QString, StructPlayerSettings> m_playerSettings;
    QMap <QString, StructPlayerSettings> m_tempPlayerSettings;
}; 

#endif
