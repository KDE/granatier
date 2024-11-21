/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mainwindow.h"

#include "granatier_version.h"

#include <KAboutData>
#include <KCrash>
#include <KLocalizedString>

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <KDBusService>
#define HAVE_KICONTHEME __has_include(<KIconTheme>)
#if HAVE_KICONTHEME
#include <KIconTheme>
#endif

#define HAVE_STYLE_MANAGER __has_include(<KStyleManager>)
#if HAVE_STYLE_MANAGER
#include <KStyleManager>
#endif
int main(int argc, char** argv)
{
#if HAVE_KICONTHEME
    KIconTheme::initTheme();
#endif
    QApplication app(argc, argv);
#if HAVE_STYLE_MANAGER
    KStyleManager::initStyle();
#else // !HAVE_STYLE_MANAGER
#if defined(Q_OS_MACOS) || defined(Q_OS_WIN)
    QApplication::setStyle(QStringLiteral("breeze"));
#endif // defined(Q_OS_MACOS) || defined(Q_OS_WIN)
#endif // HAVE_STYLE_MANAGER
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("granatier")));
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("granatier"));

    // About Granatier
    KAboutData about(QStringLiteral("granatier"),  i18n("Granatier"),
        QStringLiteral(GRANATIER_VERSION_STRING),
        i18n("Granatier is a Bomberman Clone!"),
        KAboutLicense::GPL, i18n("Copyright (c) 2009-2015 The Granatier Team!"),
        QString(),
        QStringLiteral("https://apps.kde.org/granatier")
    );
    about.addAuthor(i18n("Mathias Kraus"), i18n("Maintainer"), QStringLiteral("k.hias@gmx.de"));
    about.addCredit(i18n("Arturo Silva"), i18n("Most of the beautiful granatier graphics"));
    about.addCredit(i18n("Thomas Gallinari"), i18n("Developer of Kapman, which was the base of Granatier"), QStringLiteral("tg8187@yahoo.fr"));
    about.addCredit(i18n("Pierre-Benoit Besse"), i18n("Developer of Kapman, which was the base of Granatier"), QStringLiteral("besse.pb@gmail.com"));
    about.addCredit(i18n("Romain Castan"), i18n("Developer of Kapman, which was the base of Granatier"), QStringLiteral("romaincastan@gmail.com"));
    about.addCredit(i18n("Alexandre Galinier"), i18n("Developer of Kapman, which was the base of Granatier"), QStringLiteral("alex.galinier@gmail.com"));
    about.addCredit(i18n("Nathalie Liesse"), i18n("Developer of Kapman, which was the base of Granatier"), QStringLiteral("nathalie.liesse@gmail.com"));

    KAboutData::setApplicationData(about);

    KCrash::initialize();

    // Command line arguments
    QCommandLineParser parser;
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
