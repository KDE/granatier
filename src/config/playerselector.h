/***************************************************************************
 *   Copyright 2012 Mathias Kraus <k.hias@gmx.de>                          *
 *   Copyright 2012 Stefan Majewsky <majewsky@gmx.net>                     *
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

#ifndef PLAYERSELECTOR_H
#define PLAYERSELECTOR_H

#include <QWidget>

class PlayerSettings;
class PlayerProvider;

class PlayerSelector : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(PlayerSelector)
public:
    ///Flags which control the behavior of PlayerSelector.
    enum Option {
        DefaultBehavior = 0,
        ///Enable downloading of additional themes with KNewStuff3.
        ///This requires a KNS3 config file to be installed for this app.
        EnableNewStuffDownload = 1 << 0
    };
    Q_DECLARE_FLAGS(Options, Option)

    explicit PlayerSelector(PlayerSettings* playerSettings, Options options = DefaultBehavior, QWidget* parent = 0);
    virtual ~PlayerSelector();
private:
    struct Private;
    Private* const d;

    Q_PRIVATE_SLOT(d, void _k_showNewStuffDialog());
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PlayerSelector::Options)

#endif // PLAYERSELECTOR_H
