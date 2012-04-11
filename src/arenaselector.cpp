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

#include <QGraphicsView>
#include <KgTheme>
#include <KGameRenderer>
#include <KStandardDirs>
#include <KConfigSkeleton>
#include <knewstuff2/engine.h>
#include <KSaveFile>
#include <QDir>
#include <KComponentData>

#include "ui_arenaselector.h"
#include "arenasettings.h"

class ArenaSelector::ArenaSelectorPrivate
{
    public:
        ArenaSelectorPrivate(ArenaSelector* parent) : q(parent) {}
        ~ArenaSelectorPrivate()
        {
            qDeleteAll(arenaMap);
            if(m_graphicsScene)
            {
                qDeleteAll(m_arenaItems);
                delete m_graphicsScene;
            }
            delete m_renderer;
        }

        ArenaSelector* q;

        QMap<QString, ArenaSettings*> arenaMap;
        Ui::ArenaSelectorBase ui;
        QString lookupDirectory;
        QString groupName;
        
        KGameRenderer* m_renderer;
        QGraphicsScene* m_graphicsScene;
        QList <KGameRenderedItem*> m_arenaItems;

        void setupData(KConfigSkeleton* config, ArenaSelector::NewStuffState knsflags);
        void findArenas(const QString &initialSelection);

        // private slots
        void _k_updatePreview();
        void _k_updateArenaList(const QString& strArena);
        void _k_openKNewStuffDialog();
        void _k_importArenasDialog();
};

ArenaSelector::ArenaSelector(QWidget* parent, KConfigSkeleton * aconfig, ArenaSelector::NewStuffState knsflags, const QString &groupName, const QString &directory)
    : QWidget(parent), d(new ArenaSelectorPrivate(this))
{
    d->lookupDirectory = directory;
    d->groupName = groupName;
    d->setupData(aconfig, knsflags);
    d->_k_updatePreview();
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

void ArenaSelector::ArenaSelectorPrivate::setupData(KConfigSkeleton * aconfig, ArenaSelector::NewStuffState knsflags)
{
    ui.setupUi(q);
    ui.getNewButton->setIcon(KIcon( QLatin1String( "get-hot-new-stuff" )));

    //The lineEdit widget holds our arena path for automatic connection via KConfigXT.
    //But the user should not manipulate it directly, so we hide it.
    ui.kcfg_Arena->hide();
    connect(ui.kcfg_Arena, SIGNAL(textChanged(QString)), q, SLOT(_k_updateArenaList(QString)));

    //Disable KNS button?
    if (knsflags==ArenaSelector::NewStuffDisableDownload) {
      ui.getNewButton->hide();
    }
    
    //graphicsscene for new arena preview
    m_graphicsScene = new QGraphicsScene();
    ui.arenaPreview->setScene(m_graphicsScene);
    ui.arenaPreview->setBackgroundBrush(Qt::black);
    m_renderer = 0;
    
    //Get the last used arena path from the KConfigSkeleton
    KConfigSkeletonItem * configItem = aconfig->findItem("Arena");
    QString lastUsedArena = configItem->property().toString();

    //Now get our arenas into the list widget
    KGlobal::dirs()->addResourceType("arenaselector", "data", KGlobal::mainComponent().componentName() + '/' + lookupDirectory + '/');
    findArenas(lastUsedArena);

    connect(ui.getNewButton, SIGNAL(clicked()), q, SLOT(_k_openKNewStuffDialog()));
    connect(ui.importArenas, SIGNAL(clicked()), q, SLOT(_k_importArenasDialog()));
}

void ArenaSelector::ArenaSelectorPrivate::findArenas(const QString &initialSelection)
{
    qDeleteAll(arenaMap);
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
    connect(ui.arenaList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), q, SLOT(_k_updatePreview()));
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
    
    //show the arena without a preview pixmap
    Arena* arena = new Arena;
    MapParser mapParser(arena);
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
    delete m_renderer;
    
    KgTheme* theme = new KgTheme(QByteArray());
    theme->setGraphicsPath(KStandardDirs::locate("appdata", QString("themes/granatier.svgz")));
    m_renderer = new KGameRenderer(theme);
    
    ui.arenaPreview->setSceneRect(0, 0, arena->getNbColumns()*Cell::SIZE, arena->getNbRows()*Cell::SIZE);
    ui.arenaPreview->fitInView(ui.arenaPreview->sceneRect(), Qt::KeepAspectRatio);
    
    qreal x = m_graphicsScene->views().at(0)->x();
    qreal y = m_graphicsScene->views().at(0)->y();
    qreal width = m_graphicsScene->views().at(0)->width();
    qreal height = m_graphicsScene->views().at(0)->height();
    QPointF topLeftView = m_graphicsScene->views().at(0)->mapToScene(0, 0);
    QPointF bottomRightView = m_graphicsScene->views().at(0)->mapToScene(width, height);
    QPixmap pixmap;
    
    x = topLeftView.x();
    y = topLeftView.y();
    width = bottomRightView.x() - x;
    height = bottomRightView.y() - y;
    
    //calculate the scale factor between graphicsscene and graphicsview
    //TODO: calcute with width and views().at(0)->width();
    QPoint topLeft(0, 0);
    QPoint bottomRight(100, 100);
    topLeft = m_graphicsScene->views().at(0)->mapFromScene(topLeft);
    bottomRight = m_graphicsScene->views().at(0)->mapFromScene(bottomRight);
    qreal svgScaleFactor = 100.0 / (bottomRight.x() - topLeft.x());
    
    //the svg size
    QSize svgSize;
    
    for (int i = 0; i < arena->getNbRows(); ++i)
    {
        for (int j = 0; j < arena->getNbColumns(); ++j)
        {
            // Create the ArenaItem and set the image
            ArenaItem* arenaItem = new ArenaItem(j * Cell::SIZE, i * Cell::SIZE, m_renderer, "");
            
            switch(arena->getCell(i,j).getType())
            {
                case Cell::WALL:
                    arenaItem->setSpriteKey("arena_wall");
                    arenaItem->setZValue(-2);
                    break;
                case Cell::BLOCK:
                    arenaItem->setSpriteKey("arena_block");
                    arenaItem->setZValue(0);
                    break;
                case Cell::HOLE:
                    delete arenaItem;
                    arenaItem = NULL;
                    break;
                case Cell::ICE:
                    arenaItem->setSpriteKey("arena_ice");
                    arenaItem->setZValue(0);
                    break;
                case Cell::BOMBMORTAR:
                    arenaItem->setSpriteKey("arena_bomb_mortar");
                    arenaItem->setZValue(0);
                    break;
                case Cell::ARROWUP:
                    arenaItem->setSpriteKey("arena_arrow_up");
                    arenaItem->setZValue(0);
                    break;
                case Cell::ARROWRIGHT:
                    arenaItem->setSpriteKey("arena_arrow_right");
                    arenaItem->setZValue(0);
                    break;
                case Cell::ARROWDOWN:
                    arenaItem->setSpriteKey("arena_arrow_down");
                    arenaItem->setZValue(0);
                    break;
                case Cell::ARROWLEFT:
                    arenaItem->setSpriteKey("arena_arrow_left");
                    arenaItem->setZValue(0);
                    break;
                case Cell::GROUND:
                default:
                    arenaItem->setSpriteKey("arena_ground");
                    arenaItem->setZValue(-1);
            }
            if(arenaItem)
            {
                svgSize = m_renderer->boundsOnSprite(arenaItem->spriteKey()).size().toSize();
                
                QPoint topLeft(0, 0); 
                topLeft = m_graphicsScene->views().at(0)->mapFromScene(topLeft);
                
                QPoint bottomRight(svgSize.width(), svgSize.height()); 
                bottomRight = m_graphicsScene->views().at(0)->mapFromScene(bottomRight);
                
                svgSize.setHeight(bottomRight.y() - topLeft.y());
                svgSize.setWidth(bottomRight.x() - topLeft.x());
                
                arenaItem->setRenderSize(svgSize);
                arenaItem->setScale(svgScaleFactor);
                
                m_arenaItems.append(arenaItem);
                m_graphicsScene->addItem(arenaItem);
            }
        }
    }
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

void ArenaSelector::ArenaSelectorPrivate::_k_importArenasDialog()
{
    //find the clanbomber files
    QStringList listClanbomberPaths;
    listClanbomberPaths.append(QDir::homePath() + "/.clanbomber/maps/");
    for(int i = 0; i < listClanbomberPaths.count(); i++)
    {
        QDir clanbomberDir(listClanbomberPaths[i]);
        if(!clanbomberDir.exists())
        {
            continue;
        }
        
        QStringList listMaps;
        listMaps = clanbomberDir.entryList(QStringList("*.map"));
        for(int j = 0; j < listMaps.count(); j++)
        {
            QFile mapFile(listClanbomberPaths[i] + listMaps[j]);
            mapFile.open(QIODevice::ReadOnly | QIODevice::Text);
            QTextStream readStream(&mapFile);
            
            QString strAuthor = readStream.readLine();
            int nNumberOfPlayers = readStream.readLine().toInt();
            
            KSaveFile desktopFile;
            QString strName = listMaps[j].left(listMaps[j].count()-4);
            desktopFile.setFileName(QString("%1clanbomber_%2.desktop").arg(KStandardDirs::locateLocal("appdata", "arenas/")).arg(strName));
            desktopFile.open();
            QTextStream streamDesktopFile(&desktopFile);
            
            streamDesktopFile << "[Arena]\n";
            streamDesktopFile << "Name=" << strName << "\n";
            streamDesktopFile << "Description=Clanbomber Import\n";
            streamDesktopFile << "Type=XML\n";
            streamDesktopFile << "FileName=clanbomber_" << strName << ".xml\n";
            streamDesktopFile << "Author=" << strAuthor << "\n";
            streamDesktopFile << "AuthorEmail=-\n";
            
            streamDesktopFile.flush();
            desktopFile.finalize();
            desktopFile.close();
            
            QStringList arena;
            do
            {
                arena.append(readStream.readLine());
            }
            while(!readStream.atEnd());
            arena.replaceInStrings("*", "=");
            arena.replaceInStrings(" ", "_");
            arena.replaceInStrings("-", " ");
            arena.replaceInStrings("S", "-");
            arena.replaceInStrings("R", "x");
            arena.replaceInStrings("^", "u");
            arena.replaceInStrings(">", "r");
            arena.replaceInStrings("v", "d");
            arena.replaceInStrings("<", "l");
            arena.replaceInStrings("0", "p");
            arena.replaceInStrings("1", "p");
            arena.replaceInStrings("2", "p");
            arena.replaceInStrings("3", "p");
            arena.replaceInStrings("4", "p");
            arena.replaceInStrings("5", "p");
            arena.replaceInStrings("6", "p");
            arena.replaceInStrings("7", "p");
            arena.replaceInStrings("8", "p");
            arena.replaceInStrings("9", "p");
            
            KSaveFile arenaFile;
            arenaFile.setFileName(QString("%1clanbomber_%2.xml").arg(KStandardDirs::locateLocal("appdata", "arenas/")).arg(strName));
            arenaFile.open();
            
            QTextStream streamArenaFile(&arenaFile);
            
            streamArenaFile << "<?xml version=\"1.0\"?>\n";
            streamArenaFile << "<Arena arenaFileVersion=\"1\" rowCount=\"" << arena.count() << "\" colCount=\"" << arena[0].count() << "\">\n";
            for(int j = 0; j < arena.count(); j++)
            {
                streamArenaFile << "  <Row>" << arena[j] << "</Row>\n";
            }
            streamArenaFile << "</Arena>\n";
        }
    }
    
    ArenaSettings* selArena = arenaMap.value(ui.arenaList->currentItem()->text());
    findArenas(selArena->fileName());
}

#include "arenaselector.moc"
