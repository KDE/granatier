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

#include <QtCore/QPointer>
#include <QtGui/QAbstractItemView>
#include <QtGui/QApplication>
#include <QtGui/QCloseEvent>
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <QtGui/QListWidget>
#include <QtGui/QLineEdit>
#include <QtGui/QPainter>
#include <QtGui/QPushButton>
#include <QtGui/QScrollBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QSpacerItem>
#include <QtSvg/QSvgRenderer>
#include <KDE/KStandardDirs>
#include <KDE/KConfig>
#include <KDE/KLocalizedString>
#include <KNS3/DownloadDialog>

//PlayerSelectorDelegate declaration
#include <QtGui/QStyledItemDelegate>
class PlayerSelectorDelegate : public QStyledItemDelegate
{
public:
    PlayerSelectorDelegate(QObject* parent = 0);
    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    ///@note The implementation is independent of @a option and @a index.
    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
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

    Private(PlayerSettings* playerSettings, Options options, PlayerSelector* q) : q(q), m_playerSettings(playerSettings), m_options(options), m_knsButton(0) {}

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
    d->m_list->setMinimumSize(itemSizeHint.width() + 2 * scrollBarSizeHint.width(), 3.3 * itemSizeHint.height());
    //setup main layout
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->addWidget(d->m_list);
    //setup KNS button
    if (options & EnableNewStuffDownload)
    {
        d->m_knsButton = new QPushButton(KIcon("get-hot-new-stuff"),
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
        
        renderer.load(KStandardDirs::locate("appdata", QString("players/%1").arg(m_playerSettings->playerPreviewFile(playerIDs[i]))));
        renderer.render(&pixPainter, "player");
        playerSelectorItem->setPlayerPreviewPixmap(pixmap);
        
        KConfig desktopFile(KStandardDirs::locate("appdata", "players/" + playerIDs[i]), KConfig::SimpleConfig);
        QString author = desktopFile.group("KGameTheme").readEntry<QString>("Author", "");
        QString authorEmail = QString("<a href=\"mailto:%1\">%1</a>").arg(desktopFile.group("KGameTheme").readEntry<QString>("AuthorEmail", ""));
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

class PlayerSelector::Dialog : public KDialog
{
    public:
        Dialog(PlayerSelector* sel, const QString& caption)
        {
            setMainWidget(sel);
            //replace
            QPushButton* btn = sel->d->m_knsButton;
            if (btn)
            {
                btn->hide();
                setButtons(Close | User1);
                setButtonText(User1, btn->text());
                //cannot use btn->icon() because setButtonIcon() wants KIcon
                setButtonIcon(User1, KIcon("get-hot-new-stuff"));
                connect(this, SIGNAL(user1Clicked()), btn, SIGNAL(clicked()));
            }
            else
            {
                setButtons(Close);
            }
            //window caption
            if (caption.isEmpty())
            {
                setCaption(i18nc("@title:window config dialog", "Select player"));
            }
            else
            {
                setCaption(caption);
            }
            show();
        }
    protected:
        virtual void closeEvent(QCloseEvent* event)
        {
            event->accept();
            PlayerSelector* sel = qobject_cast<PlayerSelector*>(mainWidget());
            //delete myself, but *not* the PlayerSelector
            sel->setParent(0);
            deleteLater();
            //restore the KNS button
            if (sel->d->m_knsButton)
            {
                sel->d->m_knsButton->show();
            }
        }
};

void PlayerSelector::showAsDialog(const QString& caption)
{
    if (!isVisible())
    {
        new PlayerSelector::Dialog(this, caption);
    }
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

void PlayerSelectorDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    //draw background
    QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, 0);
}

QSize PlayerSelectorDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option) Q_UNUSED(index)
    //TODO: take text size into account
    return QSize(600, 64 /*player preview height*/ + 2 * 6 /*padding*/ + 40 /* some space for the player name */);
}

//END PlayerSelectorDelegate

#include "playerselector.moc"
