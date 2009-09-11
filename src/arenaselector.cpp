/*
    Copyright (C) 2009 Mathias Kraus <k.hias@gmx.de>
    Copyright (C) 2007 Mauricio Piacentini  <mauricio@tabuleiro.com>
    Copyright (C) 2007 Matt Williams   <matt@milliams.com>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "arenaselector.h"

#include <KLocale>
#include <KStandardDirs>
#include <KConfigSkeleton>
#include <knewstuff2/engine.h>
#include <KComponentData>

#include "ui_arenaselector.h"
#include "arenasettings.h"

class ArenaSelector::ArenaSelectorPrivate
{
    public:
        ArenaSelectorPrivate(ArenaSelector* parent) : q(parent) {}
        ~ArenaSelectorPrivate() { qDeleteAll(arenaMap); }

        ArenaSelector* q;

        QMap<QString, ArenaSettings*> arenaMap;
        Ui::ArenaSelectorBase ui;
        QString lookupDirectory;
        QString groupName;

        void setupData(KConfigSkeleton* config, ArenaSelector::NewStuffState knsflags);
        void findArenas(const QString &initialSelection);

        // private slots
        void _k_updatePreview();
        void _k_updateArenaList(const QString& strArena);
        void _k_openKNewStuffDialog();
};

ArenaSelector::ArenaSelector(QWidget* parent, KConfigSkeleton * aconfig, ArenaSelector::NewStuffState knsflags, const QString &groupName, const QString &directory)
    : QWidget(parent), d(new ArenaSelectorPrivate(this))
{
    d->lookupDirectory = directory;
    d->groupName = groupName;
    d->setupData(aconfig, knsflags);
}

ArenaSelector::~ArenaSelector()
{
    delete d;
}

void ArenaSelector::ArenaSelectorPrivate::setupData(KConfigSkeleton * aconfig, ArenaSelector::NewStuffState knsflags)
{
    ui.setupUi(q);
    ui.getNewButton->setIcon(KIcon("get-hot-new-stuff"));

    //The lineEdit widget holds our arena path for automatic connection via KConfigXT.
    //But the user should not manipulate it directly, so we hide it.
    ui.kcfg_Arena->hide();
    connect(ui.kcfg_Arena, SIGNAL(textChanged(const QString&)), q, SLOT(_k_updateArenaList(const QString&)));

    //Disable KNS button?
    if (knsflags==ArenaSelector::NewStuffDisableDownload) {
      ui.getNewButton->hide();
    }

    //Get the last used arena path from the KConfigSkeleton
    KConfigSkeletonItem * configItem = aconfig->findItem("Arena");
    QString lastUsedArena = configItem->property().toString();

    //Now get our arenas into the list widget
    KGlobal::dirs()->addResourceType("arenaselector", "data", KGlobal::mainComponent().componentName() + '/' + lookupDirectory + '/');
    findArenas(lastUsedArena);

    connect(ui.getNewButton, SIGNAL(clicked()), q, SLOT(_k_openKNewStuffDialog()));
}

void ArenaSelector::ArenaSelectorPrivate::findArenas(const QString &initialSelection)
{
    qDeleteAll(arenaMap.values());
    arenaMap.clear();

    //Disconnect the arenaList as we are going to clear it and do not want previews generated
    ui.arenaList->disconnect();
    ui.arenaList->clear();
    ui.arenaList->setSortingEnabled(true);

    QStringList arenasAvailable;
    KGlobal::dirs()->findAllResources("arenaselector", "*.desktop", KStandardDirs::Recursive, arenasAvailable);

    bool initialFound = false;
    foreach (const QString &file, arenasAvailable)
    {
      QString arenaPath = lookupDirectory + '/' + file;
      ArenaSettings* arenaSettings = new ArenaSettings(groupName);

      if (arenaSettings->load(arenaPath)) {
        QString arenaName = arenaSettings->arenaProperty("Name");
        //Add underscores to avoid duplicate names.
        while (arenaMap.contains(arenaName))
          arenaName += '_';
        arenaMap.insert(arenaName, arenaSettings);
        QListWidgetItem * item = new QListWidgetItem(arenaName, ui.arenaList);

        //Find if this is our currently configured arena
        if (arenaPath==initialSelection) {
          initialFound = true;
          ui.arenaList->setCurrentItem(item);
          _k_updatePreview();
        }
      } else {
        delete arenaSettings;
      }
    }

    if (!initialFound)
    {
      // TODO change this if we ever change ArenaSettings::loadDefault
      QString defaultPath = "arenas/granatier.desktop";
      foreach(ArenaSettings* arenaSettings, arenaMap)
      {
        if (arenaSettings->path().endsWith(defaultPath))
        {
          const QList<QListWidgetItem *> itemList = ui.arenaList->findItems(arenaSettings->arenaProperty("Name"), Qt::MatchExactly);
          // never can be != 1 but better safe than sorry
          if (itemList.count() == 1)
          {
            ui.arenaList->setCurrentItem(itemList.first());
            _k_updatePreview();
          }
        }
      }
    }

    //Reconnect the arenaList
    connect(ui.arenaList, SIGNAL(currentItemChanged ( QListWidgetItem * , QListWidgetItem * )), q, SLOT(_k_updatePreview()));
}

void ArenaSelector::ArenaSelectorPrivate::_k_updatePreview()
{
    ArenaSettings * selArena = arenaMap.value(ui.arenaList->currentItem()->text());
    //Sanity checkings. Should not happen.
    if (!selArena) return;
    if (selArena->path() == ui.kcfg_Arena->text()) {
        return;
    }
    ui.kcfg_Arena->setText(selArena->fileName());

    QString authstr("Author");
    QString contactstr("AuthorEmail");
    QString descstr("Description");
    QString emailstr;
    if (!selArena->arenaProperty(contactstr).isEmpty() ) {
        emailstr = QString("<a href=\"mailto:%1\">%1</a>").arg(selArena->arenaProperty(contactstr));
    }

    ui.arenaAuthor->setText(selArena->arenaProperty(authstr));
    ui.arenaContact->setText(emailstr);
    ui.arenaDescription->setText(selArena->arenaProperty(descstr));

    //Draw the preview
    QPixmap pix(selArena->preview());
    ui.arenaPreview->setPixmap(pix.scaled(ui.arenaPreview->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
}

void ArenaSelector::ArenaSelectorPrivate::_k_updateArenaList(const QString& strArena)
{
    //find arena and set selection to the current arena; happens when pressing "Default"
    if(arenaMap.value(ui.arenaList->currentItem()->text())->fileName() != strArena)
    {
        for(int i = 0; i < ui.arenaList->count(); i++)
        {
            if(arenaMap.value(ui.arenaList->item(i)->text())->fileName() == strArena)
            {
                ui.arenaList->setCurrentItem(ui.arenaList->item(i));
                break;
            }
        }
    }
}

void ArenaSelector::ArenaSelectorPrivate::_k_openKNewStuffDialog()
{
    KNS::Entry::List entries = KNS::Engine::download();
    //rescan arena directory
    QString currentArenaPath = ui.kcfg_Arena->text();
    if (entries.size()>0) findArenas(currentArenaPath);
    //Needed as the static KNS constructor made copies
    qDeleteAll(entries);
}

#include "arenaselector.moc"
