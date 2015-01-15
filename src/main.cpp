/*
 * Copyright 2009 Mathias Kraus <k.hias@gmx.de>
 * Copyright 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of 
 * the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mainwindow.h"

#include <KAboutData>

#include <QApplication>
#include <KLocalizedString>
#include <QCommandLineParser>
#include <kdelibs4configmigrator.h>
#include <KDBusService>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    Kdelibs4ConfigMigrator migrate(QStringLiteral("granatier"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("granatierrc"));
    migrate.setUiFiles(QStringList() << QStringLiteral("granatierui.rc"));
    migrate.migrate();

    // About Granatier
    KAboutData about("granatier",  i18n("Granatier"), "1.2.1",
        i18n("Granatier is a Bomberman Clone!"),
        KAboutLicense::GPL, i18n("Copyright (c) 2009 The Granatier Team!"));
    about.addAuthor(i18n("Mathias Kraus"), i18n("Maintainer"), "k.hias@gmx.de", 0);
    about.addCredit(i18n("Thomas Gallinari"), i18n("Developer of Kapman, which was the base of Granatier"), "tg8187@yahoo.fr", 0);
    about.addCredit(i18n("Pierre-Benoit Besse"), i18n("Developer of Kapman, which was the base of Granatier"), "besse.pb@gmail.com", 0);
    about.addCredit(i18n("Romain Castan"), i18n("Developer of Kapman, which was the base of Granatier"), "romaincastan@gmail.com", 0);
    about.addCredit(i18n("Alexandre Galinier"), i18n("Developer of Kapman, which was the base of Granatier"), "alex.galinier@gmail.com", 0);
    about.addCredit(i18n("Nathalie Liesse"), i18n("Developer of Kapman, which was the base of Granatier"), "nathalie.liesse@gmail.com", 0);
    // Command line arguments
    QCommandLineParser parser;
    KAboutData::setApplicationData(about);
    parser.addVersionOption();
    parser.addHelpOption();
    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);
    KDBusService service;
    // Create the application
    // Create the main window
    MainWindow* window = new MainWindow();
    // Show the main window
    window->show();
    // Execute the application
    return app.exec();
}

