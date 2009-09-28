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

#include "playerselector.h"
#include "playersettings.h"
#include <settings.h>

#include <KLocale>
#include <KStandardDirs>
#include <knewstuff2/engine.h>
#include <KComponentData>
#include <KSvgRenderer>
#include <QGraphicsSvgItem>
#include <QDebug>

#include "ui_playerselector.h"

class PlayerSelector::PlayerSelectorPrivate
{
public:
    PlayerSelectorPrivate(PlayerSelector* parent, PlayerSettings* playerSettings) : q(parent)
    {
        m_playerSettings = playerSettings;
    }
    ~PlayerSelectorPrivate()
    {
        if(m_playerPreview)
        {
            if(m_graphicsScene->items().contains(m_playerPreview))
            {
                m_graphicsScene->removeItem(m_playerPreview);
            }
            delete m_playerPreview;
        }
        if(m_renderer)
        {
            delete m_renderer;
        }
        if(m_graphicsScene)
        {
            delete m_graphicsScene;
        }
    }

    PlayerSelector* q;
    
    Ui::PlayerSelectorBase ui;
    KSvgRenderer* m_renderer;
    QGraphicsScene* m_graphicsScene;
    QGraphicsSvgItem* m_playerPreview;
    
    QStringList strPlayerIDs;
    PlayerSettings* m_playerSettings;
    
    QString m_testSetting;
    
    void setupData();
    
    //slot
    void slotUpdatePreview();
    void slotUpdateName(QListWidgetItem* item);
    void slotUpdateShortcut(const QKeySequence& seq);
};

PlayerSelector::PlayerSelector(QWidget* parent, PlayerSettings* playerSettings) : QWidget(parent), d(new PlayerSelectorPrivate(this, playerSettings))
{
    d->setupData();
}

PlayerSelector::~PlayerSelector()
{
    delete d;
}

void PlayerSelector::PlayerSelectorPrivate::setupData()
{
    ui.setupUi(q);
    
    //hide dummy kcfg
    ui.kcfg_Dummy->hide();
    
    ui.getNewButton->setIcon(KIcon("get-hot-new-stuff"));
    ui.getNewButton->hide();
    
    strPlayerIDs = m_playerSettings->playerIDs();
    Qt::CheckState checkState;
    for(int i = 0; i < strPlayerIDs.count(); i++)
    {
        checkState = Qt::Unchecked;
        QListWidgetItem * item = new QListWidgetItem(m_playerSettings->playerName(strPlayerIDs[i]), ui.playerList);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        if(m_playerSettings->enabled(strPlayerIDs[i]))
        {
            checkState = Qt::Checked;
        }
        item->setCheckState(checkState);
        item->setIcon(QIcon(KStandardDirs::locate("appdata", QString("players/%1").arg(m_playerSettings->playerPreviewFile(strPlayerIDs[i])))));
    }
    
    m_graphicsScene = new QGraphicsScene();
    ui.playerPreview->setScene(m_graphicsScene);
    m_renderer = 0;
    m_playerPreview = 0;
    ui.playerPreview->fitInView(ui.playerPreview->sceneRect(), Qt::KeepAspectRatio);
    ui.playerPreview->setBackgroundBrush(Qt::black);
    
    connect(ui.playerList, SIGNAL(itemSelectionChanged()), q, SLOT(slotUpdatePreview()));
    connect(ui.playerList, SIGNAL(itemChanged(QListWidgetItem*)), q, SLOT(slotUpdateName(QListWidgetItem*)));
    
    if(ui.playerList->count() > 0)
    {
        ui.playerList->setCurrentItem(ui.playerList->item(0));
        
        ui.keyUp->setObjectName(strPlayerIDs[0]);
        ui.keyUp->setKeySequence(m_playerSettings->keyUp(strPlayerIDs[0]));
        ui.keyUp->setModifierlessAllowed(true);
        ui.keyUp->setMultiKeyShortcutsAllowed(false);
        ui.keyRight->setObjectName(strPlayerIDs[0]);
        ui.keyRight->setKeySequence(m_playerSettings->keyRight(strPlayerIDs[0]));
        ui.keyRight->setModifierlessAllowed(true);
        ui.keyRight->setMultiKeyShortcutsAllowed(false);
        ui.keyDown->setObjectName(strPlayerIDs[0]);
        ui.keyDown->setKeySequence(m_playerSettings->keyDown(strPlayerIDs[0]));
        ui.keyDown->setModifierlessAllowed(true);
        ui.keyDown->setMultiKeyShortcutsAllowed(false);
        ui.keyLeft->setObjectName(strPlayerIDs[0]);
        ui.keyLeft->setKeySequence(m_playerSettings->keyLeft(strPlayerIDs[0]));
        ui.keyLeft->setModifierlessAllowed(true);
        ui.keyLeft->setMultiKeyShortcutsAllowed(false);
        ui.keyPutBomb->setObjectName(strPlayerIDs[0]);
        ui.keyPutBomb->setKeySequence(m_playerSettings->keyPutBomb(strPlayerIDs[0]));
        ui.keyPutBomb->setModifierlessAllowed(true);
        ui.keyPutBomb->setMultiKeyShortcutsAllowed(false);
    }
    
    connect(ui.keyUp, SIGNAL(keySequenceChanged(const QKeySequence&)), q, SLOT(slotUpdateShortcut(const QKeySequence&)));
    connect(ui.keyRight, SIGNAL(keySequenceChanged(const QKeySequence&)), q, SLOT(slotUpdateShortcut(const QKeySequence&)));
    connect(ui.keyDown, SIGNAL(keySequenceChanged(const QKeySequence&)), q, SLOT(slotUpdateShortcut(const QKeySequence&)));
    connect(ui.keyLeft, SIGNAL(keySequenceChanged(const QKeySequence&)), q, SLOT(slotUpdateShortcut(const QKeySequence&)));
    connect(ui.keyPutBomb, SIGNAL(keySequenceChanged(const QKeySequence&)), q, SLOT(slotUpdateShortcut(const QKeySequence&)));
}

void PlayerSelector::PlayerSelectorPrivate::slotUpdatePreview()
{
    if(m_playerPreview)
    {
        if(m_graphicsScene->items().contains(m_playerPreview))
        {
            m_graphicsScene->removeItem(m_playerPreview);
        }
        delete m_playerPreview;
    }
    if(m_renderer)
    {
        delete m_renderer;
    }
    int nIndex = ui.playerList->currentIndex().row();
    //TODO: check if renderer is needed
    m_renderer = new KSvgRenderer;
    m_renderer->load(KStandardDirs::locate("appdata", QString("players/%1").arg(m_playerSettings->playerPreviewFile(strPlayerIDs[nIndex]))));
    int nDummy = ui.kcfg_Dummy->value() + 1;
    ui.kcfg_Dummy->setValue(nDummy);
    m_playerPreview = new QGraphicsSvgItem();
    m_playerPreview->setSharedRenderer(m_renderer);
    if(m_renderer->elementExists("player"))
    {
        m_playerPreview->setElementId("player");
    }
    m_graphicsScene->addItem(m_playerPreview);
    ui.playerPreview->fitInView(ui.playerPreview->sceneRect(), Qt::KeepAspectRatio);
    
    KConfig desktopFile(KStandardDirs::locate("appdata", "players/" + strPlayerIDs[nIndex]), KConfig::SimpleConfig);
    ui.playerAuthor->setText(desktopFile.group("Player").readEntry<QString>("Author", ""));
    ui.playerContact->setText(QString("<a href=\"mailto:%1\">%1</a>").arg(desktopFile.group("Player").readEntry<QString>("AuthorEmail", "")));
    ui.playerDescription->setText(desktopFile.group("Player").readEntry<QString>("Description", ""));
    
    ui.keyUp->setObjectName(strPlayerIDs[nIndex]);
    ui.keyUp->setKeySequence(m_playerSettings->keyUp(strPlayerIDs[nIndex]));
    ui.keyRight->setObjectName(strPlayerIDs[nIndex]);
    ui.keyRight->setKeySequence(m_playerSettings->keyRight(strPlayerIDs[nIndex]));
    ui.keyDown->setObjectName(strPlayerIDs[nIndex]);
    ui.keyDown->setKeySequence(m_playerSettings->keyDown(strPlayerIDs[nIndex]));
    ui.keyLeft->setObjectName(strPlayerIDs[nIndex]);
    ui.keyLeft->setKeySequence(m_playerSettings->keyLeft(strPlayerIDs[nIndex]));
    ui.keyPutBomb->setObjectName(strPlayerIDs[nIndex]);
    ui.keyPutBomb->setKeySequence(m_playerSettings->keyPutBomb(strPlayerIDs[nIndex]));
}

void PlayerSelector::PlayerSelectorPrivate::slotUpdateName(QListWidgetItem* item)
{
    int nIndex = ui.playerList->currentIndex().row();
    if(m_playerSettings->playerName(strPlayerIDs[nIndex]) != item->text())
    {
        m_playerSettings->setPlayerName(strPlayerIDs[nIndex], item->text());
        int nDummy = ui.kcfg_Dummy->value() + 1;
        ui.kcfg_Dummy->setValue(nDummy);
    }
    bool checkState = false;
    if(item->checkState() == Qt::Checked)
    {
        checkState = true;
    }
    if(m_playerSettings->enabled(strPlayerIDs[nIndex]) != checkState)
    {
        m_playerSettings->setEnabled(strPlayerIDs[nIndex], checkState);
        int nDummy = ui.kcfg_Dummy->value() + 1;
        ui.kcfg_Dummy->setValue(nDummy);
    }
}

void PlayerSelector::PlayerSelectorPrivate::slotUpdateShortcut(const QKeySequence& seq)
{
    int nIndex = ui.playerList->currentIndex().row();
    m_playerSettings->setKeyUp(strPlayerIDs[nIndex], ui.keyUp->keySequence());
    m_playerSettings->setKeyRight(strPlayerIDs[nIndex], ui.keyRight->keySequence());
    m_playerSettings->setKeyDown(strPlayerIDs[nIndex], ui.keyDown->keySequence());
    m_playerSettings->setKeyLeft(strPlayerIDs[nIndex], ui.keyLeft->keySequence());
    m_playerSettings->setKeyPutBomb(strPlayerIDs[nIndex], ui.keyPutBomb->keySequence());
    
    int nDummy = ui.kcfg_Dummy->value() + 1;
    ui.kcfg_Dummy->setValue(nDummy);
}

#include "playerselector.moc"
