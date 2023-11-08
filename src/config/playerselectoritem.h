/*
    SPDX-FileCopyrightText: 2012 Mathias Kraus <k.hias@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PLAYERSELECTORITEM_H
#define PLAYERSELECTORITEM_H

#include <QWidget>
#include <QImage>
#include <QPixmap>

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
    QPixmap m_playerPreviewPixmap;
    QImage m_playerPreviewImageAlphaChannel;
    
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
    PlayerSelectorItem(const QString& playerId, PlayerSettings* playerSettings, QWidget* parent = nullptr);
    ~PlayerSelectorItem() override;
    
    void setPlayerPreviewPixmap(const QPixmap& pixmap);
    void setPlayerAuthor(const QString& name, const QString& mail);

private Q_SLOTS:
    void selectionChanged(bool selectionState);
    void settingsChanged();

};

#endif //PLAYERSELECTORITEM_H

