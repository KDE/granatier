/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
    SPDX-FileCopyrightText: 2007-2008 Pierre-Benoît Besse <besse.pb@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ARENAITEM_H
#define ARENAITEM_H

#include <KGameRenderedItem>

class KGameRenderer;

/**
 * @brief This class is the graphical view of the Arena.
 */
class ArenaItem : public QObject, public KGameRenderedItem
{

    Q_OBJECT

public:

    /**
      * Creates a new ArenaItem instances.
      * @param p_x the x-coordinate
      * @param p_y the y-coordinate
      * @param renderer the KGameRenderer
      * @param spriteKey the spriteKey for the ArenaItem
      */
    ArenaItem(qreal p_x, qreal p_y, KGameRenderer* renderer, const QString& spriteKey);

    /**
      * Deletes the ArenaItem instances.
      */
    ~ArenaItem() override;

public Q_SLOTS:

    /**
     * Updates the graphics after a resize
     * @param svgScaleFactor the scaling factor between svg and rendered pixmap
     */
    virtual void updateGraphics(qreal svgScaleFactor);
};

#endif

