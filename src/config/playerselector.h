/*
    SPDX-FileCopyrightText: 2012 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2012 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef PLAYERSELECTOR_H
#define PLAYERSELECTOR_H

#include <QWidget>

class PlayerSettings;

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

    explicit PlayerSelector(PlayerSettings* playerSettings, Options options = DefaultBehavior, QWidget* parent = nullptr);
    virtual ~PlayerSelector();
private:
    struct Private;
    Private* const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PlayerSelector::Options)

#endif // PLAYERSELECTOR_H
