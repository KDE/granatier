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

#include "playerselectoritem.h"
#include "playersettings.h"

#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QSpacerItem>
#include <QtGui/QCheckBox>

#include <KDE/KLineEdit>
#include <KDE/KKeySequenceWidget>
#include <KDE/KLocalizedString>


PlayerSelectorItem::PlayerSelectorItem(const QString& playerId, PlayerSettings* playerSettings, QWidget* parent) : QWidget(parent), m_playerId(playerId), m_playerSettings(playerSettings)
{
    m_selectCheckBox = new QCheckBox;
    m_selectCheckBox->setChecked(m_playerSettings->enabled(playerId));
    m_playerName = new KLineEdit(m_playerSettings->playerName(playerId));
    m_playerName->setFixedWidth(200);
    m_playerPreviewPixmap = new QPixmap(QSize(64, 64));
    m_playerPreviewPixmapAlphaChannel = new QPixmap(QSize(64, 64));
    m_playerPreviewPixmapLabel = new QLabel;
    m_playerAuthor = new QLabel;
    
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setMargin(6);
    
    QGridLayout* gridLayoutPlayer = new QGridLayout();
    gridLayoutPlayer->setMargin(0);
    
    QVBoxLayout* verticalLayoutKeySequence = new QVBoxLayout();
    verticalLayoutKeySequence->setMargin(0);
    
    mainLayout->addLayout(gridLayoutPlayer);
    mainLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    mainLayout->addLayout(verticalLayoutKeySequence);
    mainLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    
    gridLayoutPlayer->addWidget(m_selectCheckBox, 0, 0);
    gridLayoutPlayer->addWidget(m_playerName, 0, 1, 1, 3);
    gridLayoutPlayer->addWidget(m_playerPreviewPixmapLabel, 1, 1, 2, 1);
    gridLayoutPlayer->addWidget(m_playerAuthor, 2, 2);//, 1, 1, Qt::AlignBottom);
    
    QGridLayout* gridLayoutKeySequence = new QGridLayout();
    gridLayoutKeySequence->setMargin(0);
    
    verticalLayoutKeySequence->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
    verticalLayoutKeySequence->addLayout(gridLayoutKeySequence);
    verticalLayoutKeySequence->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
    
    m_moveLeft = new KKeySequenceWidget(this);
    m_moveLeft->setMultiKeyShortcutsAllowed(false);
    m_moveLeft->setModifierlessAllowed(true);
    m_moveLeft->setClearButtonShown(false);
    m_moveLeft->setKeySequence(m_playerSettings->keyLeft(playerId));
    m_labelMoveLeft = new QLabel(i18nc("Label for the key sequence to move the player.", "Left:"));
    gridLayoutKeySequence->addWidget(m_labelMoveLeft, 1, 1, Qt::AlignRight);
    gridLayoutKeySequence->addWidget(m_moveLeft, 1, 2);
    
    m_moveUp = new KKeySequenceWidget(this);
    m_moveUp->setMultiKeyShortcutsAllowed(false);
    m_moveUp->setModifierlessAllowed(true);
    m_moveUp->setClearButtonShown(false);
    m_moveUp->setKeySequence(m_playerSettings->keyUp(playerId));
    m_labelMoveUp = new QLabel(i18nc("Label for the key sequence to move the player.", "Up:"));
    gridLayoutKeySequence->addWidget(m_labelMoveUp, 0, 3, Qt::AlignRight);
    gridLayoutKeySequence->addWidget(m_moveUp, 0, 4);
    
    m_moveRight = new KKeySequenceWidget(this);
    m_moveRight->setMultiKeyShortcutsAllowed(false);
    m_moveRight->setModifierlessAllowed(true);
    m_moveRight->setClearButtonShown(false);
    m_moveRight->setKeySequence(m_playerSettings->keyRight(playerId));
    m_labelMoveRight = new QLabel(i18nc("Label for the key sequence to move the player.", "Right:"));
    gridLayoutKeySequence->addWidget(m_labelMoveRight, 1, 5, Qt::AlignRight);
    gridLayoutKeySequence->addWidget(m_moveRight, 1, 6);
    
    m_moveDown = new KKeySequenceWidget(this);
    m_moveDown->setMultiKeyShortcutsAllowed(false);
    m_moveDown->setModifierlessAllowed(true);
    m_moveDown->setClearButtonShown(false);
    m_moveDown->setKeySequence(m_playerSettings->keyDown(playerId));
    m_labelMoveDown = new QLabel(i18nc("Label for the key sequence to move the player.", "Down:"));
    gridLayoutKeySequence->addWidget(m_labelMoveDown, 2, 3, Qt::AlignRight);
    gridLayoutKeySequence->addWidget(m_moveDown, 2, 4);
    
    m_dropBomb = new KKeySequenceWidget(this);
    m_dropBomb->setMultiKeyShortcutsAllowed(false);
    m_dropBomb->setModifierlessAllowed(true);
    m_dropBomb->setClearButtonShown(false);
    m_dropBomb->setKeySequence(m_playerSettings->keyPutBomb(playerId));
    m_labelDropBomb = new QLabel(i18nc("Label for the key sequence to move the player.", "Bomb:"));
    gridLayoutKeySequence->addWidget(m_labelDropBomb, 1, 3, Qt::AlignRight);
    gridLayoutKeySequence->addWidget(m_dropBomb, 1, 4);
    
    selectionChanged(m_selectCheckBox->isChecked());
    connect(m_selectCheckBox, SIGNAL(toggled(bool)), this, SLOT(selectionChanged(bool)));
    connect(m_selectCheckBox, SIGNAL(toggled(bool)), this, SLOT(settingsChanged()));
    connect(m_playerName, SIGNAL(textEdited(QString)), this, SLOT(settingsChanged()));
    connect(m_moveLeft, SIGNAL(keySequenceChanged(QKeySequence)), this, SLOT(settingsChanged()));
    connect(m_moveUp, SIGNAL(keySequenceChanged(QKeySequence)), this, SLOT(settingsChanged()));
    connect(m_moveRight, SIGNAL(keySequenceChanged(QKeySequence)), this, SLOT(settingsChanged()));
    connect(m_moveDown, SIGNAL(keySequenceChanged(QKeySequence)), this, SLOT(settingsChanged()));
    connect(m_dropBomb, SIGNAL(keySequenceChanged(QKeySequence)), this, SLOT(settingsChanged()));
}

PlayerSelectorItem::~PlayerSelectorItem()
{
    delete m_selectCheckBox;
    
    delete m_playerName;
    delete m_playerPreviewPixmap;
    delete m_playerPreviewPixmapAlphaChannel;;
    delete m_playerPreviewPixmapLabel;
    delete m_playerAuthor;
    
    delete m_moveLeft;
    delete m_moveUp;
    delete m_moveRight;
    delete m_moveDown;
    delete m_dropBomb;
    
    delete m_labelMoveLeft;
    delete m_labelMoveUp;
    delete m_labelMoveRight;
    delete m_labelMoveDown;
    delete m_labelDropBomb;
}

void PlayerSelectorItem::setPlayerPreviewPixmap(const QPixmap& pixmap)
{
    delete m_playerPreviewPixmap;
    m_playerPreviewPixmap = new QPixmap(pixmap);
    
    QPixmap tempPixmap(*m_playerPreviewPixmap);
    tempPixmap.setAlphaChannel(*m_playerPreviewPixmapAlphaChannel);
    m_playerPreviewPixmapLabel->setPixmap(tempPixmap);
}

void PlayerSelectorItem::setPlayerAuthor(const QString& name, const QString& mail)
{
    QString mailString;
    if(!mail.isEmpty())
    {
        mailString = "<br />" + mail;
    }
    m_playerAuthor->setText("<i>" + i18nc("Author attribution, e.g. \"by Jack\"", "by %1", name) + "</i>" + mailString);
}

void PlayerSelectorItem::selectionChanged(bool selectionState)
{
    m_playerName->setEnabled(selectionState);
    m_playerAuthor->setEnabled(selectionState);
    
    if(selectionState == true)
    {
        m_playerPreviewPixmapAlphaChannel->fill(QColor(255, 255, 255, 255));
    }
    else
    {
        m_playerPreviewPixmapAlphaChannel->fill(QColor(64, 64, 64, 255));
    }
    QPixmap tempPixmap(*m_playerPreviewPixmap);
    tempPixmap.setAlphaChannel(*m_playerPreviewPixmapAlphaChannel);
    m_playerPreviewPixmapLabel->setPixmap(tempPixmap);
    
    m_moveLeft->setEnabled(selectionState);
    m_moveUp->setEnabled(selectionState);
    m_moveRight->setEnabled(selectionState);
    m_moveDown->setEnabled(selectionState);
    m_dropBomb->setEnabled(selectionState);
    
    m_labelMoveLeft->setEnabled(selectionState);
    m_labelMoveUp->setEnabled(selectionState);
    m_labelMoveRight->setEnabled(selectionState);
    m_labelMoveDown->setEnabled(selectionState);
    m_labelDropBomb->setEnabled(selectionState);
}

void PlayerSelectorItem::settingsChanged()
{
    m_playerSettings->setEnabled(m_playerId, m_selectCheckBox->isChecked());
    m_playerSettings->setPlayerName(m_playerId, m_playerName->text());
    m_playerSettings->setKeyLeft(m_playerId, m_moveLeft->keySequence());
    m_playerSettings->setKeyUp(m_playerId, m_moveUp->keySequence());
    m_playerSettings->setKeyRight(m_playerId, m_moveRight->keySequence());
    m_playerSettings->setKeyDown(m_playerId, m_moveDown->keySequence());
    m_playerSettings->setKeyPutBomb(m_playerId, m_dropBomb->keySequence());
}
