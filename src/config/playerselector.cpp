/***************************************************************************
 *   Copyright 2012 Mathias Kraus <k.hias@gmx.de>                          *
 *   Copyright 2009-2012 Stefan Majewsky <majewsky@gmx.net>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License          *
 *   version 2 as published by the Free Software Foundation                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "playerselector.h"
#include "playerselectoritem.h"
#include "playersettings.h"

#include <QPointer>
#include <QAbstractItemView>
#include <QApplication>
#include <QFont>
#include <QListWidget>
#include <QPainter>
#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QSvgRenderer>
#include <QStandardPaths>
#include <KConfig>
#include <KLocalizedString>
#include <KNS3/DownloadDialog>
#include <QIcon>
#include <KConfigGroup>
//PlayerSelectorDelegate declaration
#include <QStyledItemDelegate>
class PlayerSelectorDelegate : public QStyledItemDelegate
{
public:
    explicit PlayerSelectorDelegate(QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    ///@note The implementation is independent of @a option and @a index.
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

//BEGIN PlayerSelector

struct PlayerSelector::Private
{
    PlayerSelector* q;
    PlayerSettings* m_playerSettings;
    Options m_options;
    QListWidget* m_list;
    QPushButton* m_knsButton;

    void fillList();

    Private(PlayerSettings* playerSettings, Options options, PlayerSelector* q) : q(q), m_playerSettings(playerSettings), m_options(options), m_knsButton(nullptr) {}

    void _k_showNewStuffDialog();
};

PlayerSelector::PlayerSelector(PlayerSettings* playerSettings, Options options, QWidget* parent)
    : QWidget(parent)
    , d(new Private(playerSettings, options, this))
{
    d->m_list = new QListWidget(this);
    d->m_list->setSelectionMode(QAbstractItemView::NoSelection);
    d->m_list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    //load themes from provider
    d->fillList();
    //setup appearance of the theme list (min. size = 4 items)
    PlayerSelectorDelegate* delegate = new PlayerSelectorDelegate(d->m_list);
    const QSize itemSizeHint = delegate->sizeHint(QStyleOptionViewItem(), QModelIndex());
    const QSize scrollBarSizeHint = d->m_list->verticalScrollBar()->sizeHint();
    d->m_list->setMinimumSize(static_cast<int>(itemSizeHint.width() + 2 * scrollBarSizeHint.width()), static_cast<int>(3.3 * itemSizeHint.height()));
    //setup main layout
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(d->m_list);
    //setup KNS button
    if (options & EnableNewStuffDownload)
    {
        d->m_knsButton = new QPushButton(QIcon::fromTheme(QStringLiteral("get-hot-new-stuff")),
            i18n("Get New Players..."), this);
        layout->addWidget(d->m_knsButton);
        connect(d->m_knsButton, SIGNAL(clicked()), SLOT(_k_showNewStuffDialog()));
    }
}

PlayerSelector::~PlayerSelector()
{
    delete d;
}

void PlayerSelector::Private::fillList()
{
    m_list->clear();

    QModelIndex modelIndex;
    PlayerSelectorItem* playerSelectorItem;

    QSvgRenderer renderer;
    QPixmap pixmap(QSize(64, 64));
    pixmap.fill(QColor(0, 0, 0, 0));
    QPainter pixPainter(&pixmap);

    QStringList playerIDs = m_playerSettings->playerIDs();

    for(int i = 0; i < playerIDs.count(); i++)
    {
        QListWidgetItem* item = new QListWidgetItem(playerIDs[i], m_list);

        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);

        playerSelectorItem = new PlayerSelectorItem(playerIDs[i], m_playerSettings, m_list);

        renderer.load(QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("players/%1").arg(m_playerSettings->playerGraphicsFile(playerIDs[i]))));
        renderer.render(&pixPainter, QStringLiteral("player_0"));
        playerSelectorItem->setPlayerPreviewPixmap(pixmap);

        KConfig desktopFile(QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("players/%1").arg(playerIDs[i])), KConfig::SimpleConfig);
        QString author = desktopFile.group("KGameTheme").readEntry<QString>("Author", QStringLiteral(""));
        QString authorEmail = QStringLiteral("<a href=\"mailto:%1\">%1</a>").arg(desktopFile.group("KGameTheme").readEntry<QString>("AuthorEmail", QStringLiteral("")));
        //TODO: QString description = desktopFile.group("KGameTheme").readEntry<QString>("Description", "");
        playerSelectorItem->setPlayerAuthor(author, authorEmail);

        modelIndex = m_list->model()->index(i, 0, m_list->rootIndex());
        m_list->setIndexWidget(modelIndex, playerSelectorItem);
    }
}

void PlayerSelector::Private::_k_showNewStuffDialog()
{
    QPointer<KNS3::DownloadDialog> dialog = new KNS3::DownloadDialog (q);
    if(dialog->exec() == QDialog::Accepted)
    {
        if(!(dialog->changedEntries().isEmpty()))
        {
            //TODO: discover new arenas and add them to the list
            fillList();
        }
    }
    delete dialog;
}

//END PlayerSelector
//BEGIN PlayerSelectorDelegate

PlayerSelectorDelegate::PlayerSelectorDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
    QAbstractItemView* view = qobject_cast<QAbstractItemView*>(parent);
    if (view)
        view->setItemDelegate(this);
}

void PlayerSelectorDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const
{
    //draw background
    QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, nullptr);
}

QSize PlayerSelectorDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option) Q_UNUSED(index)
    //TODO: take text size into account
    return QSize(600, 64 /*player preview height*/ + 2 * 6 /*padding*/ + 40 /* some space for the player name */);
}

//END PlayerSelectorDelegate

#include "moc_playerselector.cpp"
