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
    
    //connect(ui.playerList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), q ,SLOT(slotUpdatePreview()));
    connect(ui.playerList, SIGNAL(itemSelectionChanged()), q, SLOT(slotUpdatePreview()));
    connect(ui.playerList, SIGNAL(itemChanged(QListWidgetItem*)), q, SLOT(slotUpdateName(QListWidgetItem*)));
    
    if(ui.playerList->count() > 0)
    {
        ui.playerList->setCurrentItem(ui.playerList->item(0));
    }
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
    //TODO: check if renderer is needed
    m_renderer = new KSvgRenderer;
    m_renderer->load(KStandardDirs::locate("appdata", QString("players/%1").arg(m_playerSettings->playerPreviewFile(strPlayerIDs[ui.playerList->currentIndex().row()]))));
    int nDummy = ui.kcfg_Dummy->value() + 1;
    ui.kcfg_Dummy->setValue(nDummy);
    m_playerPreview = new QGraphicsSvgItem();
    m_playerPreview->setSharedRenderer(m_renderer);
    m_playerPreview->setElementId("player");
    m_graphicsScene->addItem(m_playerPreview);
    ui.playerPreview->fitInView(ui.playerPreview->sceneRect(), Qt::KeepAspectRatio);
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

#include "playerselector.moc"
