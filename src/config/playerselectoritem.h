/*
 * Copyright 2012 Mathias Kraus <k.hias@gmx.de>
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

#ifndef PLAYERSELECTORITEM_H
#define PLAYERSELECTORITEM_H

#include <QWidget>

class PlayerSettings;
class QCheckBox;
class QLabel;
class QLineEdit;
class KKeySequenceWidget;

class PlayerSelectorItem : public QWidget
{
    Q_OBJECT
    
private:
    QCheckBox* m_selectCheckBox;
    
    QLineEdit* m_playerName;
    QPixmap* m_playerPreviewPixmap;
    QImage* m_playerPreviewImageAlphaChannel;
    
    QLabel* m_playerPreviewPixmapLabel;
    QLabel* m_playerAuthor;
    
    KKeySequenceWidget* m_moveLeft;
    KKeySequenceWidget* m_moveUp;
    KKeySequenceWidget* m_moveRight;
    KKeySequenceWidget* m_moveDown;
    KKeySequenceWidget* m_dropBomb;
    
    QLabel* m_labelMoveLeft;
    QLabel* m_labelMoveUp;
    QLabel* m_labelMoveRight;
    QLabel* m_labelMoveDown;
    QLabel* m_labelDropBomb;
    
    QString m_playerId;
    //don't delete this, it's just a pointer to the real settings
    PlayerSettings* m_playerSettings;
    
public:
    PlayerSelectorItem(const QString& playerId, PlayerSettings* playerSettings, QWidget* parent = 0);
    ~PlayerSelectorItem();
    
    void setPlayerPreviewPixmap(const QPixmap& pixmap);
    void setPlayerAuthor(const QString& name, const QString& mail);

private Q_SLOTS:
    void selectionChanged(bool selectionState);
    void settingsChanged();

};

#endif //PLAYERSELECTORITEM_H

