/*
    SPDX-FileCopyrightText: 2012 Mathias Kraus <k.hias@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "playerselectoritem.h"
#include "playersettings.h"

#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSpacerItem>
#include <QCheckBox>

#include <QLineEdit>
#include <KKeySequenceWidget>
#include <KLocalizedString>


PlayerSelectorItem::PlayerSelectorItem(const QString& playerId, PlayerSettings* playerSettings, QWidget* parent) : QWidget(parent), m_playerId(playerId), m_playerSettings(playerSettings)
{
    m_selectCheckBox = new QCheckBox;
    m_selectCheckBox->setChecked(m_playerSettings->enabled(playerId));
    m_playerName = new QLineEdit(m_playerSettings->playerName(playerId));
    m_playerName->setFixedWidth(200);
    const qreal dpr = qApp->devicePixelRatio();
    const int previewSize = 64 * dpr;
    m_playerPreviewPixmap = QPixmap(QSize(previewSize, previewSize));
    m_playerPreviewPixmap.setDevicePixelRatio(dpr);
    m_playerPreviewImageAlphaChannel = QImage(QSize(previewSize, previewSize), QImage::Format_ARGB32_Premultiplied);
    m_playerPreviewPixmapLabel = new QLabel;
    m_playerAuthor = new QLabel;
    
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    
    auto* gridLayoutPlayer = new QGridLayout();
    gridLayoutPlayer->setContentsMargins(0, 0, 0, 0);
    
    auto* verticalLayoutKeySequence = new QVBoxLayout();
    verticalLayoutKeySequence->setContentsMargins(0, 0, 0, 0);
    
    mainLayout->addLayout(gridLayoutPlayer);
    mainLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    mainLayout->addLayout(verticalLayoutKeySequence);
    mainLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    
    gridLayoutPlayer->addWidget(m_selectCheckBox, 0, 0);
    gridLayoutPlayer->addWidget(m_playerName, 0, 1, 1, 3);
    gridLayoutPlayer->addWidget(m_playerPreviewPixmapLabel, 1, 1, 2, 1);
    gridLayoutPlayer->addWidget(m_playerAuthor, 2, 2);//, 1, 1, Qt::AlignBottom);
    
    auto* gridLayoutKeySequence = new QGridLayout();
    gridLayoutKeySequence->setContentsMargins(0, 0, 0, 0);
    
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
    connect(m_selectCheckBox, &QCheckBox::toggled, this, &PlayerSelectorItem::selectionChanged);
    connect(m_selectCheckBox, &QCheckBox::toggled, this, &PlayerSelectorItem::settingsChanged);
    connect(m_playerName, &QLineEdit::textEdited, this, &PlayerSelectorItem::settingsChanged);
    connect(m_moveLeft, &KKeySequenceWidget::keySequenceChanged, this, &PlayerSelectorItem::settingsChanged);
    connect(m_moveUp, &KKeySequenceWidget::keySequenceChanged, this, &PlayerSelectorItem::settingsChanged);
    connect(m_moveRight, &KKeySequenceWidget::keySequenceChanged, this, &PlayerSelectorItem::settingsChanged);
    connect(m_moveDown, &KKeySequenceWidget::keySequenceChanged, this, &PlayerSelectorItem::settingsChanged);
    connect(m_dropBomb, &KKeySequenceWidget::keySequenceChanged, this, &PlayerSelectorItem::settingsChanged);
}

PlayerSelectorItem::~PlayerSelectorItem()
{
    delete m_selectCheckBox;
    
    delete m_playerName;
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
    m_playerPreviewPixmap = QPixmap(pixmap);

    QImage tempImage = m_playerPreviewPixmap.toImage();
    tempImage.setAlphaChannel(m_playerPreviewImageAlphaChannel);
    m_playerPreviewPixmapLabel->setPixmap(QPixmap::fromImage(tempImage));
}

void PlayerSelectorItem::setPlayerAuthor(const QString& name, const QString& mail)
{
    QString mailString;
    if(!mail.isEmpty())
    {
        mailString = QStringLiteral("<br />") + mail;
    }
    m_playerAuthor->setText(QStringLiteral("<i>%1</i>%2").arg(i18nc("Author attribution, e.g. \"by Jack\"", "by %1", name), mailString));
}

void PlayerSelectorItem::selectionChanged(bool selectionState)
{
    m_playerName->setEnabled(selectionState);
    m_playerAuthor->setEnabled(selectionState);
    
    if(selectionState == true)
    {
        m_playerPreviewImageAlphaChannel.fill(QColor(255, 255, 255, 255));
    }
    else
    {
        m_playerPreviewImageAlphaChannel.fill(QColor(64, 64, 64, 255));
    }

    QImage tempImage = m_playerPreviewPixmap.toImage();
    tempImage.setAlphaChannel(m_playerPreviewImageAlphaChannel);
    m_playerPreviewPixmapLabel->setPixmap(QPixmap::fromImage(tempImage));
    
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

#include "moc_playerselectoritem.cpp"
