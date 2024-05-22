/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2006 Mauricio Piacentini <mauricio@tabuleiro.com>
    SPDX-FileCopyrightText: 2007 Matt Williams <matt@milliams.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ARENASELECTOR_H
#define ARENASELECTOR_H

#include <QWidget>

class KConfigSkeleton; 
class KGameThemeProvider;
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
    ///Flags which control the behavior of ArenaSelector.
    enum Option {
        DefaultBehavior = 0,
        ///Enable downloading of additional themes with KNewStuff3.
        ///This requires a KNS3 config file to be installed for this app.
        EnableNewStuffDownload = 1 << 0
    };
    Q_DECLARE_FLAGS(Options, Option)
    
    /**
    * Load a specific arena file.
    * @param themeProvider the theme provider to get the default theme from for preview
    * @param parent the parent widget
    * @param aconfig the KConfigSceleton
    * @param randomArenaModeArenaList the arena mode list
    * @param options the options
    * @param groupName the title of the config group in the arena .desktop file
    * @param directory subdirectory (of share/apps/appname) to search in
    * @return true if the arena files and properties could be loaded
    */
    ArenaSelector(KGameThemeProvider *themeProvider, QWidget* parent, KConfigSkeleton* aconfig, QStringList* randomArenaModeArenaList, Options options = DefaultBehavior, const QString& groupName = QStringLiteral("Arena"), const QString& directory = QStringLiteral("arenas"));
    ~ArenaSelector() override;
    
protected:
    /**
    * Resizes the items when the view is resized.
    * @param p_event the resize event
    */
    void resizeEvent(QResizeEvent* p_event) override;
    
    /**
    * Resizes the items when the view is showed.
    * @param p_event the resize event
    */
    void showEvent(QShowEvent* p_event) override;

private:
    class Private;
    Private* const d;

    Q_DISABLE_COPY(ArenaSelector)

    Q_PRIVATE_SLOT(d, void _k_updatePreview(QListWidgetItem* currentItem = NULL))
    Q_PRIVATE_SLOT(d, void _k_updateArenaList(const QString&))
    Q_PRIVATE_SLOT(d, void _k_setRandomArenaMode(bool randomModeEnabled))
    Q_PRIVATE_SLOT(d, void _k_updateRandomArenaModeArenaList(QListWidgetItem* item))
};

#endif
