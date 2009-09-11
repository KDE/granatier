/*
    Copyright (C) 2009 Mathias Kraus <k.hias@gmx.de>
    Copyright (C) 2006 Mauricio Piacentini  <mauricio@tabuleiro.com>
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

#ifndef ARENASELECTOR_H
#define ARENASELECTOR_H

#include <QtGui/QWidget>

class ArenaSettings;
class KConfigSkeleton; 
class ArenaSelectorPrivate;

/**
 * \class ArenaSelector arenaselector.h <ArenaSelector>
 * 
 * @short A widget used to select the game's arena
 *
 * The most common way to use the arena selector is to add it as page to a KConfigDialog
 * \code
 * KConfigDialog *dialog = new KConfigDialog(this, "settings", Settings::self());
 * dialog->addPage(new ArenaSelector(dialog, Settings::self()), i18n("Arena"), "game_arena");
 * dialog->show();
 * \endcode
 * This will create a page in your KConfigDialog with the title "Arena" and using the 
 * "game_arena" icon. By default, the widget will search in the share/apps/appname/arenas 
 * directory for .desktop files with a group called "Arena".
 *
 * @author Mauricio Piacentini
 **/
class ArenaSelector : public QWidget
{
    Q_OBJECT
    public:
      enum NewStuffState {
          NewStuffDisableDownload,
          NewStuffEnableDownload
        };
        /**
         * Load a specific arena file.
         * @param groupName the title of the config group in the arena .desktop file
         * @param directory subdirectory (of share/apps/appname) to search in
         * @return true if the arena files and properties could be loaded
         */
        ArenaSelector(QWidget* parent, KConfigSkeleton* config, ArenaSelector::NewStuffState knsflags = ArenaSelector::NewStuffEnableDownload, const QString &groupName = QLatin1String("Arena"), const QString &directory = QLatin1String("arenas"));
        virtual ~ArenaSelector();
    
    private:
        class ArenaSelectorPrivate;
        ArenaSelectorPrivate* const d;

        Q_DISABLE_COPY(ArenaSelector)

        Q_PRIVATE_SLOT(d, void _k_updatePreview())
        Q_PRIVATE_SLOT(d, void _k_updateArenaList(const QString&))
        Q_PRIVATE_SLOT(d, void _k_openKNewStuffDialog())
};

#endif
