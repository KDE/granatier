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
#include "arena.h"
#include "arenaitem.h"
#include "mapparser.h"
#include "settings.h"

#include <QPointer>

#include <KgTheme>
#include <QIcon>
#include <KGameRenderer>
#include <KConfigSkeleton>
#include <KNS3/DownloadDialog>
#include <QFile>
#include <QDir>
#include <QStandardPaths>

#include "ui_arenaselector.h"
#include "arenasettings.h"

class ArenaSelector::Private
{
    public:
        Private(ArenaSelector* parent, Options options);
        ~Private();

        ArenaSelector* q;
        Options m_options;

        QMap<QString, ArenaSettings*> arenaMap;
        Ui::ArenaSelectorBase ui;
        QString lookupDirectory;
        QString groupName;

        Arena* m_arena;
        KGameRenderer* m_renderer;
        QGraphicsScene* m_graphicsScene;
        QList <KGameRenderedItem*> m_arenaItems;
        qreal m_svgScaleFactor;

        QStringList* m_randomArenaModeArenaList;
        QStringList m_tempRandomArenaModeArenaList;

        void setupData(KConfigSkeleton* aconfig);
        void findArenas(const QString &initialSelection);
        QSize calculateSvgSize();

        // private slots
        void _k_updatePreview(QListWidgetItem* currentItem = nullptr);
        void _k_updateArenaList(const QString& strArena);
        void _k_openKNewStuffDialog();
        void _k_setRandomArenaMode(bool randomModeEnabled);
        void _k_updateRandomArenaModeArenaList(QListWidgetItem* item);
};

ArenaSelector::ArenaSelector(QWidget* parent, KConfigSkeleton* aconfig, QStringList* randomArenaModeArenaList, ArenaSelector::Options options, const QString& groupName, const QString& directory)
    : QWidget(parent), d(new Private(this, options))
{
    d->m_randomArenaModeArenaList = randomArenaModeArenaList;
    d->lookupDirectory = directory;
    d->groupName = groupName;
    d->setupData(aconfig);
}

ArenaSelector::~ArenaSelector()
{
    delete d;
}

void ArenaSelector::resizeEvent(QResizeEvent*)
{
    d->_k_updatePreview();
}

void ArenaSelector::showEvent(QShowEvent*)
{
    d->_k_updatePreview();
}

ArenaSelector::Private::Private(ArenaSelector* parent, Options options) : q(parent), m_options(options), m_arena(nullptr), m_graphicsScene(nullptr), m_svgScaleFactor(1)
{
    KgTheme* theme = new KgTheme(QByteArray());
    theme->setGraphicsPath(QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("themes/granatier.svgz")));
    m_renderer = new KGameRenderer(theme);
}

ArenaSelector::Private::~Private()
{
    qDeleteAll(arenaMap);
    if(m_graphicsScene)
    {
        qDeleteAll(m_arenaItems);
        delete m_graphicsScene;
    }
    delete m_renderer;
    delete m_arena;
}

void ArenaSelector::Private::setupData(KConfigSkeleton * aconfig)
{
    ui.setupUi(q);

    //setup KNS button
    if (m_options & EnableNewStuffDownload)
    {
        ui.getNewButton->setIcon(QIcon::fromTheme( QLatin1String( "get-hot-new-stuff" )));
        connect(ui.getNewButton, SIGNAL(clicked()), q, SLOT(_k_openKNewStuffDialog()));
    }
    else
    {
        ui.getNewButton->hide();
    }


    //The lineEdit widget holds our arena path for automatic connection via KConfigXT.
    //But the user should not manipulate it directly, so we hide it.
    ui.kcfg_Arena->hide();
    connect(ui.kcfg_Arena, SIGNAL(textChanged(QString)), q, SLOT(_k_updateArenaList(QString)));

    //graphicsscene for new arena preview
    m_graphicsScene = new QGraphicsScene();
    ui.arenaPreview->setScene(m_graphicsScene);
    ui.arenaPreview->setBackgroundBrush(Qt::black);

    //Get the last used arena path from the KConfigSkeleton
    KConfigSkeletonItem * configItem = aconfig->findItem(QStringLiteral("Arena"));
    QString lastUsedArena = configItem->property().toString();

    configItem = aconfig->findItem(QStringLiteral("RandomArenaModeArenaList"));
    m_tempRandomArenaModeArenaList = configItem->property().toStringList();
    m_tempRandomArenaModeArenaList.removeDuplicates();

    //Now get our arenas into the list widget
    findArenas(lastUsedArena);

    connect(ui.kcfg_RandomArenaMode, SIGNAL(toggled(bool)), q, SLOT(_k_setRandomArenaMode(bool)));
}

void ArenaSelector::Private::findArenas(const QString &initialSelection)
{
    qDeleteAll(arenaMap);
    arenaMap.clear();

    //Disconnect the arenaList as we are going to clear it and do not want previews generated
    ui.arenaList->disconnect();
    ui.arenaList->clear();
    ui.arenaList->setSortingEnabled(true);

    QStringList arenasAvailable;
    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::AppDataLocation, QStringLiteral("arenas"), QStandardPaths::LocateDirectory);
    for(auto& dir: dirs) {
         const QStringList fileNames = QDir(dir).entryList(QStringList() << QStringLiteral("*.desktop"));
         for(auto& file: fileNames) {
                arenasAvailable.append(file);
         }
    }
    
    QStringList randomArenaModeArenaList;
    // store the random arenas if they are available
    for(const auto& randomArena: m_tempRandomArenaModeArenaList) {
        if(arenasAvailable.contains(randomArena)) {
            randomArenaModeArenaList.append(randomArena);
        }
    }

    m_tempRandomArenaModeArenaList = randomArenaModeArenaList.isEmpty() ? arenasAvailable : randomArenaModeArenaList;

    bool initialFound = false;

    for(const auto& file: arenasAvailable)
    {
      QString arenaPath = lookupDirectory + QLatin1Char('/') + file;
      ArenaSettings* arenaSettings = new ArenaSettings(groupName);

      if (arenaSettings->load(arenaPath)) {
        QString arenaName; // Start with an empty QString here so that the first += allocates a reserve for future +=.
        arenaName += arenaSettings->arenaProperty(QStringLiteral("Name"));
        //Add underscores to avoid duplicate names.
        while (arenaMap.contains(arenaName))
          arenaName += QLatin1Char('_');
        arenaMap.insert(arenaName, arenaSettings);
        QListWidgetItem * item = new QListWidgetItem(arenaName, ui.arenaList);
        if(ui.kcfg_RandomArenaMode->isChecked())
        {
            if(m_tempRandomArenaModeArenaList.contains(file))
            {
                item->setCheckState(Qt::Checked);
            }
            else
            {
                item->setCheckState(Qt::Unchecked);
            }
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        }
        else
        {
            item->setCheckState(Qt::PartiallyChecked);
            item->setFlags(item->flags() & ~Qt::ItemIsUserCheckable);
        }

        //Find if this is our currently configured arena
        if (arenaPath==initialSelection) {
          initialFound = true;
          ui.arenaList->setCurrentItem(item);
          _k_updatePreview(item);
        }
      } else {
        delete arenaSettings;
      }
    }

    if (!initialFound)
    {
      // TODO change this if we ever change ArenaSettings::loadDefault
      QLatin1String defaultPath("arenas/granatier.desktop");
      for(auto arenaSettings: arenaMap)
      {
        if (arenaSettings->path().endsWith(defaultPath))
        {
          const QList<QListWidgetItem *> itemList = ui.arenaList->findItems(arenaSettings->arenaProperty(QStringLiteral("Name")), Qt::MatchExactly);
          // never can be != 1 but better safe than sorry
          if (itemList.count() == 1)
          {
            ui.arenaList->setCurrentItem(itemList.first());
            _k_updatePreview(itemList.first());
          }
        }
      }
    }

    //Reconnect the arenaList
    connect(ui.arenaList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), q, SLOT(_k_updatePreview(QListWidgetItem*)));
    if(ui.kcfg_RandomArenaMode->isChecked())
    {
        connect(ui.arenaList, SIGNAL(itemChanged(QListWidgetItem*)), q, SLOT(_k_updateRandomArenaModeArenaList(QListWidgetItem*)));
    }
}

void ArenaSelector::Private::_k_updatePreview(QListWidgetItem* currentItem)
{
    if(currentItem != nullptr)
    {
        ArenaSettings * selArena = arenaMap.value(ui.arenaList->currentItem()->text());
        //Sanity checkings. Should not happen.
        if (!selArena) return;
        if (selArena->path() == ui.kcfg_Arena->text()) {
            return;
        }
        ui.kcfg_Arena->setText(selArena->fileName());

        QString authstr(QStringLiteral("Author"));
        QString contactstr(QStringLiteral("AuthorEmail"));
        QString descstr(QStringLiteral("Description"));
        QString emailstr = selArena->arenaProperty(contactstr);
        if(emailstr.compare(QLatin1String("-")) == 0) // the imported clanbomber arenas have a "-" if no email address was defined in the clanbomber arena file
        {
            emailstr.clear();
        }
        if (!emailstr.isEmpty())
        {
            emailstr = QStringLiteral("<a href=\"mailto:%1\">%1</a>").arg(selArena->arenaProperty(contactstr));
        }

        ui.arenaAuthor->setText(i18nc("Author attribution, e.g. \"by Jack\"", "by %1", selArena->arenaProperty(authstr)));
        ui.arenaContact->setText(emailstr);
        ui.arenaDescription->setText(selArena->arenaProperty(descstr));

        //show the arena without a preview pixmap
        delete m_arena;
        m_arena = new Arena;
        MapParser mapParser(m_arena);
        QFile arenaXmlFile(selArena->graphics());
        QXmlInputSource source(&arenaXmlFile);
        // Create the XML file reader
        QXmlSimpleReader reader;
        reader.setContentHandler(&mapParser);
        // Parse the XML file
        reader.parse(source);

        while(!m_arenaItems.isEmpty())
        {
            if(m_graphicsScene->items().contains(m_arenaItems.last()))
            {
                m_graphicsScene->removeItem(m_arenaItems.last());
            }
            delete m_arenaItems.takeLast();
        }

        ui.arenaPreview->setSceneRect(0, 0, m_arena->getNbColumns()*Granatier::CellSize, m_arena->getNbRows()*Granatier::CellSize);
        ui.arenaPreview->fitInView(ui.arenaPreview->sceneRect(), Qt::KeepAspectRatio);
    }

    qreal svgScaleFactor;
    QRectF minSize = ui.arenaPreview->sceneRect();

    if(minSize.width() == 0)
    {
        minSize.setWidth(1);
    }
    if(minSize.height() == 0)
    {
        minSize.setHeight(1);
    }

    //calculate the scaling factor for the SVGs
    int horizontalPixelsPerCell = static_cast<int>((ui.arenaPreview->size().width() - 4) / (minSize.width()/Granatier::CellSize));
    int verticalPixelsPerCell = static_cast<int>((ui.arenaPreview->size().height() - 4) / (minSize.height()/Granatier::CellSize));
    if(horizontalPixelsPerCell < verticalPixelsPerCell)
    {
        svgScaleFactor = Granatier::CellSize / horizontalPixelsPerCell;
    }
    else
    {
        svgScaleFactor = Granatier::CellSize / verticalPixelsPerCell;
    }

    QTransform transform;
    transform.scale(1/svgScaleFactor, 1/svgScaleFactor);
    m_graphicsScene->views().first()->setTransform(transform);
    m_graphicsScene->views().first()->centerOn( ui.arenaPreview->sceneRect().center());

    if(currentItem == nullptr)
    {
        if(m_svgScaleFactor != svgScaleFactor)
        {
            m_svgScaleFactor = svgScaleFactor;
            for(auto arenaItem: m_arenaItems)
            {
                arenaItem->setRenderSize(calculateSvgSize());
                arenaItem->setScale(m_svgScaleFactor);
            }
        }
    }
    else
    {
        m_svgScaleFactor = svgScaleFactor;
        for (int i = 0; i < m_arena->getNbRows(); ++i)
        {
            for (int j = 0; j < m_arena->getNbColumns(); ++j)
            {
                // Create the ArenaItem and set the image
                ArenaItem* arenaItem = new ArenaItem(j * Granatier::CellSize, i * Granatier::CellSize, m_renderer, QStringLiteral(""));

                switch(m_arena->getCell(i,j).getType())
                {
                    case Granatier::Cell::WALL:
                        arenaItem->setSpriteKey(QStringLiteral("arena_wall"));
                        arenaItem->setZValue(-2);
                        break;
                    case Granatier::Cell::BLOCK:
                        arenaItem->setSpriteKey(QStringLiteral("arena_block"));
                        arenaItem->setZValue(0);
                        break;
                    case Granatier::Cell::HOLE:
                        delete arenaItem;
                        arenaItem = nullptr;
                        break;
                    case Granatier::Cell::ICE:
                        arenaItem->setSpriteKey(QStringLiteral("arena_ice"));
                        arenaItem->setZValue(0);
                        break;
                    case Granatier::Cell::BOMBMORTAR:
                        arenaItem->setSpriteKey(QStringLiteral("arena_bomb_mortar"));
                        arenaItem->setZValue(0);
                        break;
                    case Granatier::Cell::ARROWUP:
                        arenaItem->setSpriteKey(QStringLiteral("arena_arrow_up"));
                        arenaItem->setZValue(0);
                        break;
                    case Granatier::Cell::ARROWRIGHT:
                        arenaItem->setSpriteKey(QStringLiteral("arena_arrow_right"));
                        arenaItem->setZValue(0);
                        break;
                    case Granatier::Cell::ARROWDOWN:
                        arenaItem->setSpriteKey(QStringLiteral("arena_arrow_down"));
                        arenaItem->setZValue(0);
                        break;
                    case Granatier::Cell::ARROWLEFT:
                        arenaItem->setSpriteKey(QStringLiteral("arena_arrow_left"));
                        arenaItem->setZValue(0);
                        break;
                    case Granatier::Cell::GROUND:
                    default:
                        arenaItem->setSpriteKey(QStringLiteral("arena_ground"));
                        arenaItem->setZValue(-1);
                }
                if(arenaItem)
                {
                    arenaItem->setRenderSize(calculateSvgSize());
                    arenaItem->setScale(m_svgScaleFactor);

                    m_arenaItems.append(arenaItem);
                    m_graphicsScene->addItem(arenaItem);
                }
            }
        }
    }
}

QSize ArenaSelector::Private::calculateSvgSize()
{
    if(m_graphicsScene->views().isEmpty())
    {
        return QSize(1, 1);
    }

    QPoint topLeft(0, 0);
    topLeft = m_graphicsScene->views().first()->mapFromScene(topLeft);

    QPoint bottomRight(static_cast<int>(Granatier::CellSize), static_cast<int>(Granatier::CellSize));
    bottomRight = m_graphicsScene->views().first()->mapFromScene(bottomRight);

    QSize svgSize;
    svgSize.setHeight(bottomRight.y() - topLeft.y());
    svgSize.setWidth(bottomRight.x() - topLeft.x());

    return svgSize;
}

void ArenaSelector::Private::_k_updateArenaList(const QString& strArena)
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

void ArenaSelector::Private::_k_openKNewStuffDialog()
{
    QPointer<KNS3::DownloadDialog> dialog = new KNS3::DownloadDialog (q);
    if(dialog->exec() == QDialog::Accepted)
    {
        if(!(dialog->changedEntries().isEmpty()))
        {
            //TODO: discover new arenas and add them to the list
        }
    }
    delete dialog;
}

void ArenaSelector::Private::_k_setRandomArenaMode(bool randomModeEnabled)
{
    if(!randomModeEnabled)
    {
        disconnect(ui.arenaList, SIGNAL(itemChanged(QListWidgetItem*)), q, SLOT(_k_updateRandomArenaModeArenaList(QListWidgetItem*)));
    }

    m_randomArenaModeArenaList->clear();

    int numberOfItems = ui.arenaList->count();
    for(int i = 0; i < numberOfItems; i++)
    {
        QListWidgetItem* item = ui.arenaList->item(i);
        if(randomModeEnabled)
        {
            QString arenaName = arenaMap.value(item->text())->fileName();
            arenaName.remove(0, 7); //length of "arenas/"
            if(m_tempRandomArenaModeArenaList.contains(arenaName))
            {
                item->setCheckState(Qt::Checked);
            }
            else
            {
                item->setCheckState(Qt::Unchecked);
            }
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        }
        else
        {
            item->setCheckState(Qt::PartiallyChecked);
            item->setFlags(item->flags() & ~Qt::ItemIsUserCheckable);
        }
    }

    if(randomModeEnabled)
    {
        *m_randomArenaModeArenaList = m_tempRandomArenaModeArenaList;
        connect(ui.arenaList, SIGNAL(itemChanged(QListWidgetItem*)), q, SLOT(_k_updateRandomArenaModeArenaList(QListWidgetItem*)));
    }
}

void ArenaSelector::Private::_k_updateRandomArenaModeArenaList(QListWidgetItem* item)
{
    QString arenaName = arenaMap.value(item->text())->fileName();
    arenaName.remove(0, 7); //length of "arenas/"
    if(item->checkState() == Qt::Checked)
    {
        m_tempRandomArenaModeArenaList.append(arenaName);
    }
    else
    {
        int index = m_tempRandomArenaModeArenaList.indexOf(arenaName);
        if(index >= 0)
        {
            m_tempRandomArenaModeArenaList.removeAt(index);
        }
    }
    m_tempRandomArenaModeArenaList.removeDuplicates();
    *m_randomArenaModeArenaList = m_tempRandomArenaModeArenaList;
    Settings::self()->setDummy(Settings::self()->dummy() + 3);
}
#include "moc_arenaselector.cpp"

