/*
    SPDX-FileCopyrightText: 2012 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2009-2012 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

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
    void fillList();

    Private(PlayerSettings* playerSettings, Options options, PlayerSelector* q) : q(q), m_playerSettings(playerSettings), m_options(options) {}
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
    auto* delegate = new PlayerSelectorDelegate(d->m_list);
    const QSize itemSizeHint = delegate->sizeHint(QStyleOptionViewItem(), QModelIndex());
    const QSize scrollBarSizeHint = d->m_list->verticalScrollBar()->sizeHint();
    d->m_list->setMinimumSize(static_cast<int>(itemSizeHint.width() + 2 * scrollBarSizeHint.width()), static_cast<int>(3.3 * itemSizeHint.height()));
    //setup main layout
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(d->m_list);
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
        auto* item = new QListWidgetItem(playerIDs[i], m_list);

        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);

        playerSelectorItem = new PlayerSelectorItem(playerIDs[i], m_playerSettings, m_list);

        renderer.load(QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("players/%1").arg(m_playerSettings->playerGraphicsFile(playerIDs[i]))));
        renderer.render(&pixPainter, QStringLiteral("player_0"));
        playerSelectorItem->setPlayerPreviewPixmap(pixmap);

        KConfig desktopFile(QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("players/%1").arg(playerIDs[i])), KConfig::SimpleConfig);
        auto author = desktopFile.group(QStringLiteral("KGameTheme")).readEntry<QString>("Author", QStringLiteral(""));
        QString authorEmail = QStringLiteral("<a href=\"mailto:%1\">%1</a>").arg(desktopFile.group(QStringLiteral("KGameTheme")).readEntry<QString>("AuthorEmail", QStringLiteral("")));
        //TODO: QString description = desktopFile.group(QStringLiteral("KGameTheme")).readEntry<QString>("Description", "");
        playerSelectorItem->setPlayerAuthor(author, authorEmail);

        modelIndex = m_list->model()->index(i, 0, m_list->rootIndex());
        m_list->setIndexWidget(modelIndex, playerSelectorItem);
    }
}

//END PlayerSelector
//BEGIN PlayerSelectorDelegate

PlayerSelectorDelegate::PlayerSelectorDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
    auto* view = qobject_cast<QAbstractItemView*>(parent);
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
    return {600, 64 /*player preview height*/ + 2 * 6 /*padding*/ + 40 /* some space for the player name */};
}

//END PlayerSelectorDelegate

#include "moc_playerselector.cpp"
