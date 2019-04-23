/*
    Copyright (C) 2009 Mathias Kraus <k.hias@gmx.de>
    Copyright (C) 2007 Mauricio Piacentini   <mauricio@tabuleiro.com>
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

#ifndef ARENASETTINGS_H
#define ARENASETTINGS_H

#include <QString>

class ArenaSettingsPrivate;

/**
 * \class ArenaSettings arenasettings.h <ArenaSettings>
 * 
 * @short Class for loading arena files
 *
 * Essentially just a wrapper around a .desktop arena file. Load a file with
 * load() and then access its properties.
 *
 * For more advanced feaures like dynamic arenas or custom game rules, it
 * will likely be necessary to derive from this class
 *
 * @author Mauricio Piacentini
 **/
class ArenaSettings
{
    public:
        explicit ArenaSettings(const QString &arenaGroup = QStringLiteral("Arena"));
        virtual ~ArenaSettings();

        /**
         * Load the default arena file. Called "granatier.desktop"
         * @return true if the arena files and properties could be loaded
         */
        virtual bool loadDefault();
        /**
         * Load a specific arena file.
         * Note that although arena could be successfully loaded,
         * no check on the validity of arena's XML file contents is done.
         * Application writers will need to perform this check manually
         * @param file the name of the arena file relative to the share/apps/appname
         * directory. e.g. "arena/granatier.desktop"
         * @return true if the arena files and properties could be loaded
         */
        virtual bool load(const QString &file);
        /// @return the full path of the .desktop file
        QString path() const;
        /// @return just the "*.desktop" part
        QString fileName() const;
        /// @return the full path of the svg file which is specified in "FileName" key
        virtual QString graphics() const;
        /// @return a property directly from the .desktop file
        QString property(const QString &key) const;
        /**
         * Possible keys:
         * - Name
         * - Author
         * - Description
         * - AuthorEmail
         * @param key the key of the wanted property
         * @return the data related to 'key'
         */
        virtual QString arenaProperty(const QString &key) const;

    private:
        friend class ArenaSettingsPrivate;
        ArenaSettingsPrivate *const d;
};

#endif
