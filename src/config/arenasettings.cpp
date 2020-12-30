/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2007 Mauricio Piacentini <mauricio@tabuleiro.com>
    SPDX-FileCopyrightText: 2007 Matt Williams <matt@milliams.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "arenasettings.h"


#include <KConfig>
#include <KConfigGroup>
#include "granatier_debug.h"
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

class ArenaSettingsPrivate
{
    public:
        ArenaSettingsPrivate() : loaded(false) {}

        QMap<QString, QString> arenaProperties;
        QString fullPath; ///< Full path e.g. "/opt/kde/share/apps/appname/default.desktop"
        QString fileName; ///< just e.g. "default.desktop"
        QString graphics; ///< The full path of the svg file
        QString prefix; ///< Filepath of the .desktop file without the filename e.g. "/opt/kde/share/apps/appname/"
        QString arenaGroup;

        bool loaded;
};

ArenaSettings::ArenaSettings(const QString &arenaGroup)
    : d(new ArenaSettingsPrivate)
{
    d->arenaGroup = arenaGroup;
    //KGlobal::dirs()->addResourceType("gametheme", KStandardDirs::kde_default("data") + KGlobal::mainComponent().componentName());
}

ArenaSettings::~ArenaSettings() {
    delete d;
}

bool ArenaSettings::loadDefault()
{
    return load(QStringLiteral("arenas/granatier.desktop")); //TODO make this editable to match custom directories.
                                           // If this ever changes change findArenas in ArenaSelectorPrivate too
}

#define kArenaVersionFormat 1

bool ArenaSettings::load(const QString &fileName) {
    if( fileName.isEmpty() )
    {
        qCDebug(GRANATIER_LOG) << "Refusing to load arena with no name";
        return false;
    }
    QString filePath = QStandardPaths::locate(QStandardPaths::AppDataLocation, fileName);
    if (filePath.isEmpty()) {
        qCDebug(GRANATIER_LOG) << "Could not find arena with name" << fileName;
        return false;
    }

    // verify if it is a valid file first and if we can open it
    QFile arenaFile(filePath);
    if (!arenaFile.open(QIODevice::ReadOnly)) {
        qCDebug(GRANATIER_LOG) << "Could not open .desktop arena file" << filePath;
        return false;
    }
    d->prefix = QFileInfo(arenaFile).absolutePath() + QLatin1Char('/');
    arenaFile.close();

    KConfig arenaConfig(filePath, KConfig::SimpleConfig);
    if (!arenaConfig.hasGroup(d->arenaGroup))
    {
        qCDebug(GRANATIER_LOG) << "Config group" << d->arenaGroup << "does not exist in" << filePath;
        return false;
    }
    KConfigGroup group = arenaConfig.group(d->arenaGroup);

    //Copy the whole entryMap, so we can inherit generic properties as well, reducing the need to subclass for simple implementations
    d->arenaProperties = group.entryMap();

    //Version control
    int arenaVersion = group.readEntry("VersionFormat",0);
    //Format is increased when we have incompatible changes, meaning that older clients are not able to use the remaining information safely
    if (arenaVersion > kArenaVersionFormat) {
        return false;
    }

    QString arenaName = group.readEntry("FileName");
    //d->graphics = QStandardPaths::locate(QStandardPaths::AppDataLocation, graphName);
    d->graphics = d->prefix + arenaName;
    if (d->graphics.isEmpty()) return false;

    // let's see if svg file exists and can be opened
    QFile svgFile( d->graphics );
    if ( !svgFile.open( QIODevice::ReadOnly ) ) {
        qCDebug(GRANATIER_LOG) << "Could not open file" << d->graphics;
        return false;
    }

    QString previewName = group.readEntry("Preview");
    //QString graphicsPath = QStandardPaths::locate(QStandardPaths::AppDataLocation, previewName);
    QString graphicsPath = d->prefix + previewName;

    d->fileName = fileName;
    d->fullPath = filePath;
    d->loaded = true;
    return true;
}

QString ArenaSettings::property(const QString &key) const
{
    if(!d->loaded)
    {
        qCDebug(GRANATIER_LOG) << "No arena file has been loaded. ArenaSettings::load() or ArenaSettings::loadDefault() must be called.";
        return QString();
    }
    KConfig arenaConfig(path(), KConfig::SimpleConfig);
    KConfigGroup group = arenaConfig.group(d->arenaGroup);
    return group.readEntry(key, QString());
}

QString ArenaSettings::path() const {
    if(!d->loaded)
    {
        qCDebug(GRANATIER_LOG) << "No arena file has been loaded. ArenaSettings::load() or ArenaSettings::loadDefault() must be called.";
        return QString();
    }
    return d->fullPath;
}

QString ArenaSettings::fileName() const {
    if(!d->loaded)
    {
        qCDebug(GRANATIER_LOG) << "No arena file has been loaded. ArenaSettings::load() or ArenaSettings::loadDefault() must be called.";
        return QString();
    }
    return d->fileName;
}

QString ArenaSettings::graphics() const {
    if(!d->loaded)
    {
        qCDebug(GRANATIER_LOG) << "No arena file has been loaded. ArenaSettings::load() or ArenaSettings::loadDefault() must be called.";
        return QString();
    }
    return d->graphics;
}

QString ArenaSettings::arenaProperty(const QString &key) const {
    if(!d->loaded)
    {
        qCDebug(GRANATIER_LOG) << "No arena file has been loaded. ArenaSettings::load() or ArenaSettings::loadDefault() must be called.";
        return QString();
    }
    return d->arenaProperties[key];
}
