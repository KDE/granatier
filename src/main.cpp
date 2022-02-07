/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mainwindow.h"

#include "granatier_version.h"

#include <KAboutData>
#include <KCrash>

#include <QApplication>
#include <KLocalizedString>
#include <QCommandLineParser>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Kdelibs4ConfigMigrator>
#include <Kdelibs4Migration>
#endif
#include <QStandardPaths>
#include <QDir>
#include <KDBusService>
#include <KSharedConfig>

int main(int argc, char** argv)
{
    // Fixes blurry icons with fractional scaling
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QApplication app(argc, argv);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)

    Kdelibs4ConfigMigrator migrate(QStringLiteral("granatier"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("granatierrc"));
    migrate.setUiFiles(QStringList() << QStringLiteral("granatierui.rc"));
    if(migrate.migrate())
    {
        // migrate old data
        Kdelibs4Migration dataMigrator;
        const QString sourceBasePath = dataMigrator.saveLocation("data", QStringLiteral("granatier"));
        const QString targetBasePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1Char('/');
        QString targetFilePath;

        QStringList dataDirs;
        dataDirs.append(QStringLiteral("arenas"));
        dataDirs.append(QStringLiteral("players"));
        dataDirs.append(QStringLiteral("themes"));

        // migrate arenas, players and themes
        for(const auto& dataDir: dataDirs)
        {
            QDir sourceDir(sourceBasePath + dataDir);
            if(!sourceDir.exists()) continue;
            const QStringList fileNames = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
            QDir().mkpath(targetBasePath + dataDir);
            for(const auto& fileName: fileNames)
            {
                targetFilePath = targetBasePath + dataDir + QLatin1Char('/') + fileName;
                if(!QFile::exists(targetFilePath))
                {
                    QFile::copy(sourceBasePath + dataDir + QLatin1Char('/') + fileName, targetFilePath);
                }
            }
        }

        // update the configuration cache
        KSharedConfig::openConfig()->reparseConfiguration();
    }
#endif
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("granatier")));
    KLocalizedString::setApplicationDomain("granatier");

    // About Granatier
    KAboutData about(QStringLiteral("granatier"),  i18n("Granatier"),
        QStringLiteral(GRANATIER_VERSION_STRING),
        i18n("Granatier is a Bomberman Clone!"),
        KAboutLicense::GPL, i18n("Copyright (c) 2009-2015 The Granatier Team!"));
    about.addAuthor(i18n("Mathias Kraus"), i18n("Maintainer"), QStringLiteral("k.hias@gmx.de"));
    about.addCredit(i18n("Arturo Silva"), i18n("Most of the beautiful granatier graphics"));
    about.addCredit(i18n("Thomas Gallinari"), i18n("Developer of Kapman, which was the base of Granatier"), QStringLiteral("tg8187@yahoo.fr"));
    about.addCredit(i18n("Pierre-Benoit Besse"), i18n("Developer of Kapman, which was the base of Granatier"), QStringLiteral("besse.pb@gmail.com"));
    about.addCredit(i18n("Romain Castan"), i18n("Developer of Kapman, which was the base of Granatier"), QStringLiteral("romaincastan@gmail.com"));
    about.addCredit(i18n("Alexandre Galinier"), i18n("Developer of Kapman, which was the base of Granatier"), QStringLiteral("alex.galinier@gmail.com"));
    about.addCredit(i18n("Nathalie Liesse"), i18n("Developer of Kapman, which was the base of Granatier"), QStringLiteral("nathalie.liesse@gmail.com"));
    about.setHomepage(QStringLiteral("https://www.kde.org/applications/games/granatier/"));
    // Command line arguments
    QCommandLineParser parser;
    KAboutData::setApplicationData(about);
    KCrash::initialize();
    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);
    KDBusService service;
    // Create the application
    // Create the main window
    auto* window = new MainWindow();
    // Show the main window
    window->show();
    // Execute the application
    return app.exec();
}

